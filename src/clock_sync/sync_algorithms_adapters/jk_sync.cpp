
#include <cstdio>
#include <string>
#include <cstdlib>
#include <mpi.h>

#include "mpits.h"
//#include "mpits_helpers.h"

#include "time_provider/clocks/Clock.hpp"
#include "time_provider/clocks/GlobalClockOffset.hpp"

#include "clock_sync/clock_sync_loader.hpp"
#include "clock_sync/clock_sync_common.h"

#include "clock_sync/clock_offset_algs/PingpongClockOffsetAlg.hpp"
//#include "clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.hpp"

#include "clock_sync/sync_algorithms/ClockSync.hpp"
#include "clock_sync/sync_algorithms/JKClockSync.hpp"


//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


static ClockSync* clock_sync;
static Clock* local_clock;
static GlobalClock* global_clock;


static void jk_print_sync_parameters(FILE* f) {
    fprintf(f, "#@clocksync=JK\n");
}

static void synchronize_clocks(MPI_Comm comm) {
  global_clock = clock_sync->synchronize_all_clocks(comm, *(local_clock));
}

static double get_normalized_time(double local_time) {
  return default_get_normalized_time(local_time, global_clock);
}


void jk_init_module(MPI_Comm comm, int argc, char** argv) {
  ClockSyncLoader loader;

  global_clock = NULL;
  local_clock = initialize_local_clock();

  clock_sync = loader.instantiate_clock_sync(comm,"alg");
  if( clock_sync != NULL ) {
    // now we make sure it's really a JK instance
    if( dynamic_cast<JKClockSync*>(clock_sync) == NULL ) {
      ZF_LOGE("instantiated clock sync is not of type JK. aborting..");
      exit(1);
    }
  } else {
    ZF_LOGV("using default JK clock sync");
    clock_sync = new JKClockSync(new PingpongClockOffsetAlg(100,100), 1000);
  }

}


void jk_cleanup_module(void) {
  delete local_clock;
  delete clock_sync;

  if (global_clock != NULL) {
    delete global_clock;
  }
}

extern "C"
void register_jk_module(mpits_clocksync_t *sync_mod) {
  sync_mod->name = (char*)std::string("JK").c_str();
  sync_mod->clocksync = MPITS_CLOCKSYNC_JK;

  sync_mod->init_module = jk_init_module;
  sync_mod->cleanup_module = jk_cleanup_module;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = jk_print_sync_parameters;
}






