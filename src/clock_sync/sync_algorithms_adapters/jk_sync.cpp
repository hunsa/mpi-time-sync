
#include <cstdio>
#include <string>
#include <cstdlib>
#include <mpi.h>

#include "mpits.h"
//#include "mpits_helpers.h"

#include "time_provider/clocks/Clock.hpp"
#include "time_provider/clocks/GlobalClockOffset.hpp"

#include "clock_sync/mpi_clock_sync_internal.h"
#include "clock_sync/clock_sync_common.h"

#include "clock_sync/sync_algorithms/ClockSync.hpp"
#include "clock_sync/sync_algorithms/JKClockSync.hpp"

#include "helpers/dict/mpits_dict.h"
#include "helpers/dict/cli_param_parser.h"


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
  global_clock = NULL;
  local_clock = initialize_local_clock();

  char *options;
  mpits_get_value_from_dict(mpits_get_global_param_store(), "options", &options);
  if (options == NULL) {
    options = (char*)"";
  }
  clock_sync = JKClockSync::from_string(std::string(options));
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
  //sync_mod->name = (char*)std::string("JK").c_str();
  sync_mod->clocksync = MPITS_CLOCKSYNC_JK;

  sync_mod->init_module = jk_init_module;
  sync_mod->cleanup_module = jk_cleanup_module;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = jk_print_sync_parameters;
}


