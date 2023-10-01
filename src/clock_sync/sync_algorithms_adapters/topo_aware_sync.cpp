
#include <cstdio>
#include <string>
#include <cstdlib>
#include <vector>
#include <mpi.h>

#include "topo_aware_sync.h"

#include "time_provider/clocks/Clock.hpp"
#include "time_provider/clocks/GlobalClockOffset.hpp"

#include "time_provider/clocks/Clock.hpp"
#include "time_provider/clocks/GlobalClockOffset.hpp"

#include "clock_sync/clock_sync_loader.hpp"
#include "clock_sync/clock_sync_common.h"

#include "clock_sync/clock_offset_algs/PingpongClockOffsetAlg.hpp"
#include "clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.hpp"

#include "clock_sync/sync_algorithms/HierarchicalClockSync.hpp"
#include "clock_sync/sync_algorithms/offset/SKaMPIClockSync.hpp"
#include "clock_sync/sync_algorithms/JKClockSync.hpp"
#include "clock_sync/sync_algorithms/HCA2ClockSync.hpp"
#include "clock_sync/sync_algorithms/HCA3ClockSync.hpp"
#include "clock_sync/sync_algorithms/ClockPropagationSync.hpp"


//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

static ClockSync* clock_sync;
static Clock* local_clock;
static GlobalClock* global_clock;


static void topo_synchronize_clocks(MPI_Comm comm) {
  global_clock = clock_sync->synchronize_all_clocks(comm, *(local_clock));
}

static double topo_normalized_time(double local_time) {
  return default_get_normalized_time(local_time, global_clock);
}


static void topo_cleanup_module(void) {
  delete local_clock;
  delete clock_sync;

  if (global_clock != NULL) {
    delete global_clock;
  }
}



static void topo_print_sync_parameters(FILE* f)
{
  fprintf (f, "#@clocksync=Topo1\n");
}


static void topo_init_module(MPI_Comm comm, int argc, char** argv) {
  int use_default = 0;
  BaseClockSync *alg1;
  BaseClockSync *alg2;
  BaseClockSync *alg3;
  ClockSyncLoader loader;


  alg1 = loader.instantiate_clock_sync(comm, "topoalg1");
  if( alg1 != nullptr ) {
    alg2 = loader.instantiate_clock_sync(comm, "topoalg2");
    if( alg2 != nullptr ) {
      alg3 = loader.instantiate_clock_sync(comm, "topoalg3");
      if( alg3 != nullptr ) {
        // now instantiate new hierarchical clock sync
        clock_sync = new HierarchicalClockSync(alg1, alg2, alg3);
      } else {
        use_default = 1;
      }
    } else {
      use_default = 1;
    }
  } else {
    use_default = 1;
  }


  global_clock = nullptr;
  local_clock  = initialize_local_clock();

  if( use_default == 1 ) {
    ZF_LOGW("!!! using default topo1 clock sync options");

    clock_sync = new HierarchicalClockSync(
      new HCA3ClockSync(new SKaMPIClockOffsetAlg(10,100), 500, false),
      new HCA3ClockSync(new SKaMPIClockOffsetAlg(10,100), 500, false),
      new ClockPropagationSync(ClockPropagationSync::ClockType::CLOCK_LM));

    //new HCA3ClockSync(new PingpongClockOffsetAlg(), 1000, 100),
    //new ClockPropagationSync(),
    //new HCA3ClockSync(new PingpongClockOffsetAlg(), 1000, 100),
    //new HCA3ClockSync(new PingpongClockOffsetAlg(), 1000, 100));
  }

}

extern "C"
void register_topo_aware_sync1_module(mpits_clocksync_t *sync_mod) {
  sync_mod->name = (char*)std::string("Topo1").c_str();
  sync_mod->clocksync = MPITS_CLOCKSYNC_TOPO1;
  sync_mod->init_module = topo_init_module;
  sync_mod->cleanup_module = topo_cleanup_module;
  sync_mod->sync_clocks = topo_synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = topo_normalized_time;
  sync_mod->print_sync_info = topo_print_sync_parameters;
}
