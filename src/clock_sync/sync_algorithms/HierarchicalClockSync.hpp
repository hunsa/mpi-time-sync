
#ifndef MPITS_HIERARCHICALCLOCKSYNC_CLASS_H_
#define MPITS_HIERARCHICALCLOCKSYNC_CLASS_H_

#include "time_provider/clocks/Clock.hpp"
#include "time_provider/clocks/GlobalClock.hpp"
#include "ClockSync.hpp"
#include "clock_sync/sync_algorithms/utils/communicator_utils.h"

#include "clock_sync/sync_algorithms/utils/hwloc_helpers.h"

class HierarchicalClockSync: public ClockSync {

private:
  BaseClockSync *syncInterNode;
  BaseClockSync *syncSocket;
  BaseClockSync *syncOnSocket;

  MPI_Comm comm_internode;
  MPI_Comm comm_intranode;
  MPI_Comm comm_intersocket;
  MPI_Comm comm_intrasocket;

  bool comm_initialized;

  void initialized_communicators(MPI_Comm comm);

public:
  HierarchicalClockSync(BaseClockSync *syncInterNode, BaseClockSync *syncSocket, BaseClockSync *syncOnSocket); //, SyncConfiguration& conf);
  ~HierarchicalClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
};

#endif /*  MPITS_HIERARCHICALCLOCKSYNC_CLASS_H_  */
