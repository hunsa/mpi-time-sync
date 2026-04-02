
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <mpi.h>

#include "HierarchicalClockSync.hpp"

#include "time_provider/clocks/GlobalClockLM.hpp"
#include "time_provider/clocks/GlobalClockOffset.hpp"
#include "clock_sync/clock_offset_algs/PingpongClockOffsetAlg.hpp"
#include "clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.hpp"
#include "clock_sync/sync_algorithms/HCAClockSync.hpp"
#include "clock_sync/sync_algorithms/HCA2ClockSync.hpp"
#include "clock_sync/sync_algorithms/HCA3ClockSync.hpp"
#include "clock_sync/sync_algorithms/JKClockSync.hpp"
#include "clock_sync/sync_algorithms/ClockPropagationSync.hpp"
#include "clock_sync/sync_algorithms/offset/HCA3OffsetClockSync.hpp"
#include "clock_sync/sync_algorithms/offset/SKaMPIClockSync.hpp"
#include "clock_sync/sync_algorithms/utils/hwloc_helpers.h"

#include "log/zf_log.h"

static BaseClockSync* instantiate_topo_alg_from_string(const std::string& spec) {
  std::vector<std::string> tokens;
  std::stringstream ss(spec);
  std::string token;
  while (std::getline(ss, token, '@')) {
    tokens.push_back(token);
  }

  if (tokens.empty()) {
    return nullptr;
  }

  const std::string sync_alg = tokens[0];
  const std::string rest = spec.size() > sync_alg.size() ? spec.substr(sync_alg.size() + 1) : "";

  if (sync_alg == "HCA") {
    return HCAClockSync::from_string(rest);
  }
  if (sync_alg == "HCA2") {
    return HCA2ClockSync::from_string(rest);
  }
  if (sync_alg == "HCA3") {
    return HCA3ClockSync::from_string(rest);
  }
  if (sync_alg == "HCA3O") {
    return HCA3OffsetClockSync::from_string(rest);
  }
  if (sync_alg == "SKaMPI") {
    if (tokens.size() >= 4) {
      ClockOffsetAlg* offset_alg = nullptr;
      if (tokens[1] == "pingpong_offset") {
        offset_alg = new PingpongClockOffsetAlg(std::stoi(tokens[2]), std::stoi(tokens[3]));
      } else if (tokens[1] == "skampi_offset") {
        offset_alg = new SKaMPIClockOffsetAlg(std::stoi(tokens[2]), std::stoi(tokens[3]));
      }
      if (offset_alg != nullptr) {
        return new SKaMPIClockSync(offset_alg);
      }
    }
    return SKaMPIClockSync::from_string("");
  }
  if (sync_alg == "JK") {
    return JKClockSync::from_string(rest);
  }
  if (sync_alg == "prop") {
    if (tokens.size() >= 2 && tokens[1] == "0") {
      return new ClockPropagationSync(ClockPropagationSync::ClockType::CLOCK_OFFSET);
    }
    if (tokens.size() >= 2 && tokens[1] == "1") {
      return new ClockPropagationSync(ClockPropagationSync::ClockType::CLOCK_LM);
    }
    ZF_LOGW("problem with format of prop clock sync '%s', using prop@1", spec.c_str());
    return new ClockPropagationSync(ClockPropagationSync::ClockType::CLOCK_LM);
  }

  ZF_LOGW("unknown Topo1 sub-algorithm '%s'", spec.c_str());
  return nullptr;
}

HierarchicalClockSync::HierarchicalClockSync(BaseClockSync *syncInterNode,
                                             BaseClockSync *syncSocket,
                                             BaseClockSync *syncOnSocket) :
    syncInterNode(syncInterNode), syncSocket(syncSocket), syncOnSocket(syncOnSocket) {

  this->comm_internode = MPI_COMM_NULL;
  this->comm_intranode = MPI_COMM_NULL;
  this->comm_intersocket = MPI_COMM_NULL;
  this->comm_intrasocket = MPI_COMM_NULL;

  this->comm_initialized = false;
}

HierarchicalClockSync::~HierarchicalClockSync() {

  if( comm_internode != MPI_COMM_NULL ) {
    MPI_Comm_free(&comm_internode);
  }

  if( comm_intranode != MPI_COMM_NULL ) {
    MPI_Comm_free(&comm_intranode);
  }

  if( comm_intersocket != MPI_COMM_NULL ) {
    MPI_Comm_free(&comm_intersocket);
  }

  if( comm_intrasocket != MPI_COMM_NULL ) {
    MPI_Comm_free(&comm_intrasocket);
  }

}

HierarchicalClockSync* HierarchicalClockSync::from_string(const std::string& str) {
  const char* default_spec = "HCA3@0@500@skampi_offset@10@100;HCA3@0@500@skampi_offset@10@100;prop@1";
  std::vector<std::string> levels;
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, ';')) {
    levels.push_back(token);
  }

  if (levels.size() != 3) {
    ZF_LOGW("using default Topo1 parameters: options:%s", default_spec);
    return new HierarchicalClockSync(
      new HCA3ClockSync(new SKaMPIClockOffsetAlg(10, 100), 500, false),
      new HCA3ClockSync(new SKaMPIClockOffsetAlg(10, 100), 500, false),
      new ClockPropagationSync(ClockPropagationSync::ClockType::CLOCK_LM));
  }

  BaseClockSync* alg1 = instantiate_topo_alg_from_string(levels[0]);
  BaseClockSync* alg2 = instantiate_topo_alg_from_string(levels[1]);
  BaseClockSync* alg3 = instantiate_topo_alg_from_string(levels[2]);

  if (alg1 == nullptr || alg2 == nullptr || alg3 == nullptr) {
    delete alg1;
    delete alg2;
    delete alg3;
    ZF_LOGW("using default Topo1 parameters because parsing failed: options:%s", default_spec);
    return new HierarchicalClockSync(
      new HCA3ClockSync(new SKaMPIClockOffsetAlg(10, 100), 500, false),
      new HCA3ClockSync(new SKaMPIClockOffsetAlg(10, 100), 500, false),
      new ClockPropagationSync(ClockPropagationSync::ClockType::CLOCK_LM));
  }

  return new HierarchicalClockSync(alg1, alg2, alg3);
}

void HierarchicalClockSync::initialized_communicators(MPI_Comm comm) {
  int socket_id;
  int my_rank, np;


  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &np);

  // create node-level communicators for each node
  create_intranode_communicator(comm, &comm_intranode);

  print_comm_debug_info("intranode", comm, comm_intranode);

  // create an internode-communicator with processes having rank = 0 on the local communicator
  create_interlevel_communicator(comm, comm_intranode, 0, &comm_internode);

  print_comm_debug_info("internode", comm, comm_internode);

  // within the node-level communicator, create intra-socket communicators
  socket_id = get_socket_id();
  if (socket_id < 0) {
    // could not get socket information - assign socket 0 to all processes
    if (my_rank == 0) {
      std::cerr << "WARNING: Could not get socket information - assuming all processes run on the same socket\n" << std::endl;
    }
    socket_id = 0;
  }
  create_intrasocket_communicator(comm_intranode, socket_id, &comm_intrasocket);

  print_comm_debug_info("intrasocket", comm, comm_intrasocket);

  // within the node-level communicator, create an inter-socket communicator for each
  // process with rank = 0 on the intra-socket communicator
  create_interlevel_communicator(comm_intranode, comm_intrasocket, 0, &comm_intersocket);

  print_comm_debug_info("intersocket", comm, comm_intersocket);

}


GlobalClock* HierarchicalClockSync::synchronize_all_clocks(MPI_Comm comm, Clock& c) {

  GlobalClock* global_clock1 = NULL;
  GlobalClock* global_clock2 = NULL;
  GlobalClock* global_clock3 = NULL;

  int my_rank, np;
  //int socket_id;
  int subcomm_size;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &np);

  if (this->comm_initialized == false) {
    this->initialized_communicators(comm);
    this->comm_initialized = true;
  }

  // Step 1: synchronization between nodes
  if (comm_internode != MPI_COMM_NULL) {
    MPI_Comm_size(comm_internode, &subcomm_size);
    ZF_LOGV("%d: subcomm size:%d", my_rank, subcomm_size);
    if( subcomm_size > 1 ) {
      ZF_LOGV("%d: sync1 real", my_rank);
      global_clock1 = syncInterNode->synchronize_all_clocks(comm_internode, c);
    } else {
      ZF_LOGV("%d: sync1 dummy", my_rank);
      global_clock1 = new GlobalClockLM(c, 0.0, 0.0);
    }
  } else {
    // dummy clock
    ZF_LOGV("%d: sync1 dummy", my_rank);
    global_clock1 = new GlobalClockLM(c, 0.0, 0.0);
  }

#if ZF_LOG_LEVEL == ZF_LOG_VERBOSE
  global_clock1->print_clock_info();
#endif

  // Step 2: synchronization between sockets
  if (comm_intersocket != MPI_COMM_NULL) {
    MPI_Comm_size(comm_intersocket, &subcomm_size);
    ZF_LOGV("%d: subcomm size:%d", my_rank, subcomm_size);
    if( subcomm_size > 1 ) {
      ZF_LOGV("%d: sync2 real", my_rank);
      global_clock2 = syncSocket->synchronize_all_clocks(comm_intersocket, *(global_clock1));
//    GlobalClockLM *gc1 = dynamic_cast<GlobalClockLM*>(global_clock1);
//    GlobalClockLM *gc2 = dynamic_cast<GlobalClockLM*>(global_clock2);
//    global_clock2 = new GlobalClockLM(c,
//        gc1->get_slope() - gc2->get_slope() - gc1->get_slope() *gc2->get_slope(),
//        gc1->get_intercept() + gc2->get_intercept() + gc1->get_slope() * gc2->get_intercept());
    } else {
      ZF_LOGV("%d: sync2 dummy", my_rank);
      global_clock2 = global_clock1;
    }
  } else {
    ZF_LOGV("%d: sync2 dummy", my_rank);
    global_clock2 = global_clock1;
  }

#if ZF_LOG_LEVEL == ZF_LOG_VERBOSE
  global_clock2->print_clock_info();
#endif

  ZF_LOGV("%d: sync 3", my_rank);

  // Step 3: synchronization within the socket
  // all processes have an intra-socket comm
  /*
   * same here
   * if communicator has only one process, then do nothing
   */
  MPI_Comm_size(comm_intrasocket, &subcomm_size);
  ZF_LOGV("%d: subcomm size:%d", my_rank, subcomm_size);
  if( subcomm_size > 1 ) {
    global_clock3 = syncOnSocket->synchronize_all_clocks(comm_intrasocket, *(global_clock2));
  } else {
    global_clock3 = global_clock2;
  }

  ZF_LOGV("%d: sync 3 done", my_rank);

  ZF_LOGV("%d: test clock3, %g", my_rank, global_clock3->get_local_time());

//  if (comm_internode != MPI_COMM_NULL) {
//    return global_clock1;
//  }
//
//  if (comm_intersocket != MPI_COMM_NULL) {
//    return global_clock2;
//  }

  return global_clock3;
}
