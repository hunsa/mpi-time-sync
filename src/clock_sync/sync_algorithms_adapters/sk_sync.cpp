/*  from ReproMPI Benchmark
 *
 * Copyright 2003-2008 Werner Augustin, Lars Diesselberg - SKaMPI   MPI-Benchmark
   Lehrstuhl Informatik fuer Naturwissenschaftler und Ingenieure
   Fakultaet fuer Informatik
   University of Karlsruhe
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria
*/

#include <cstdio>
#include <string>
#include <cstdlib>
#include <mpi.h>

#include "mpits.h"

#include "time_provider/clocks/Clock.hpp"
#include "time_provider/clocks/GlobalClockOffset.hpp"

#include "clock_sync/mpi_clock_sync_internal.h"
#include "clock_sync/clock_sync_common.h"

#include "clock_sync/sync_algorithms/ClockSync.hpp"
#include "clock_sync/sync_algorithms/offset/SKaMPIClockSync.hpp"

#include "helpers/dict/mpits_dict.h"
#include "helpers/dict/cli_param_parser.h"


//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


typedef struct {
    long n_rep; /* --repetitions */
} mpits_sk_options_t;


static ClockSync* clock_sync;
static Clock* local_clock;
static GlobalClock* global_clock;


static void sk_print_sync_parameters(FILE* f) {
    fprintf(f, "#@clocksync=SKaMPI\n");
}

static void synchronize_clocks(MPI_Comm comm) {
  global_clock = clock_sync->synchronize_all_clocks(comm, *(local_clock));
}

static double get_normalized_time(double local_time) {
  return default_get_normalized_time(local_time, global_clock);
}

void sk_init_module(MPI_Comm comm, int argc, char** argv) {
  global_clock = NULL;
  local_clock = initialize_local_clock();

  char *options;
  mpits_get_value_from_dict(mpits_get_global_param_store(), "options", &options);
  if (options == NULL) {
    options = (char*)"";
  }
  clock_sync = SKaMPIClockSync::from_string(std::string(options));
}

void sk_cleanup_module(void) {
  delete local_clock;
  delete clock_sync;

  if (global_clock != NULL) {
    delete global_clock;
  }
}

extern "C"
void register_skampi_module(mpits_clocksync_t *sync_mod) {
  //sync_mod->name = (char*) std::string("SKaMPI").c_str();
  sync_mod->clocksync = MPITS_CLOCKSYNC_SKAMPI;

  sync_mod->init_module = sk_init_module;
  sync_mod->cleanup_module = sk_cleanup_module;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = sk_print_sync_parameters;
}
