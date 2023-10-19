
#include <cstdio>
#include <string>
#include <cstdlib>
#include <mpi.h>

#include "clock_sync/mpi_clock_sync_internal.h"
#include "time_provider/clocks/GlobalClock.hpp"
#include "clock_sync/clock_sync_loader.hpp"

#include "clock_sync/clock_offset_algs/PingpongClockOffsetAlg.hpp"
#include "clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.hpp"

#include "clock_sync/sync_algorithms/HCAClockSync.hpp"
#include "clock_sync/sync_algorithms/HCA2ClockSync.hpp"
#include "clock_sync/sync_algorithms/HCA3ClockSync.hpp"
#include "clock_sync/sync_algorithms/offset/HCA3OffsetClockSync.hpp"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

static ClockSync* clock_sync;
static Clock* local_clock;
static GlobalClock* global_clock;

static void synchronize_clocks(MPI_Comm comm) {
    if( global_clock != NULL ) {
        delete global_clock;
    }
    global_clock = clock_sync->synchronize_all_clocks(comm, *(local_clock));
}

static double get_normalized_time(double local_time) {
  return default_get_normalized_time(local_time, global_clock);
}

static void hca_cleanup_module(void) {
  delete local_clock;
  delete clock_sync;

  if (global_clock != NULL) {
    delete global_clock;
  }
}

static void hca_common_print(FILE* f)
{
//  fprintf(f, "#@fitpoints=%d\n", parameters.n_fitpoints);
//  fprintf(f, "#@exchanges=%d\n", parameters.n_exchanges);
}

/******************************
 *
 * HCA
 *
 */

static void hca_print_sync_parameters(FILE* f)
{
  hca_common_print(f);
  fprintf (f, "#@clocksync=HCA\n");
  fprintf(f, "#@hcasynctype=linear\n");
}


static void hca_init_module(MPI_Comm comm, int argc, char** argv) {
  ClockSyncLoader loader;

  global_clock = NULL;
  local_clock = initialize_local_clock();

  clock_sync = loader.instantiate_clock_sync(comm, "alg");
  if( clock_sync != NULL ) {
    // now we make sure it's really an HCA instance
    if( dynamic_cast<HCAClockSync*>(clock_sync) == NULL ) {
      ZF_LOGE("instantiated clock sync is not of type HCA. aborting..");
      exit(1);
    }
  } else {
    ZF_LOGV("using default hca clock sync");
    clock_sync = new HCAClockSync(new PingpongClockOffsetAlg(100,100), 1000);
  }
}


/******************************
 *
 * HCA2
 *
 */

static void hca2_print_sync_parameters(FILE* f)
{
  hca_common_print(f);
  fprintf(f, "#@clocksync=HCA2\n");
  fprintf(f, "#@hcasynctype=logp\n");
}


static void hca2_init_module(MPI_Comm comm, int argc, char** argv) {
  ClockSyncLoader loader;

  global_clock = NULL;
  local_clock = initialize_local_clock();

  clock_sync = loader.instantiate_clock_sync(comm, "alg");
  if( clock_sync != NULL ) {
    // now we make sure it's really an HCA2 instance
    if( dynamic_cast<HCA2ClockSync*>(clock_sync) == NULL ) {
      ZF_LOGE("instantiated clock sync is not of type HCA2. aborting..");
      exit(1);
    }
  } else {
    ZF_LOGV("using default HCA2 clock sync");
    clock_sync = new HCA2ClockSync(new PingpongClockOffsetAlg(100,100), 1000, false);
  }
}


/******************************
 *
 * HCA3
 *
 */

static void hca3_init_module(MPI_Comm comm, int argc, char** argv) {
  ClockSyncLoader loader;

  global_clock = NULL;
  local_clock = initialize_local_clock();

  clock_sync = loader.instantiate_clock_sync(comm, "alg");
  if( clock_sync != NULL ) {
    // now we make sure it's really an HCA3 instance
    if( dynamic_cast<HCA3ClockSync*>(clock_sync) == NULL ) {
      ZF_LOGE("instantiated clock sync is not of type HCA3. aborting..");
      exit(1);
    }
  } else {
    ZF_LOGV("using default hca3 clock sync");
    clock_sync = new HCA3ClockSync(new PingpongClockOffsetAlg(100,100), 1000, false);
  }


}

static void hca3_print_sync_parameters(FILE* f)
{
  fprintf (f, "#@clocksync=HCA3\n");
}

/******************************
 *
 * HCA3_Offset
 *
 */

static void hca3_offset_init_module(MPI_Comm comm, int argc, char** argv) {
    ClockSyncLoader loader;

    global_clock = NULL;
    local_clock = initialize_local_clock();

    clock_sync = loader.instantiate_clock_sync(comm, "alg");
    if( clock_sync != NULL ) {
        // now we make sure it's really an HCA3Offset instance
        if( dynamic_cast<HCA3OffsetClockSync*>(clock_sync) == NULL ) {
            ZF_LOGE("instantiated clock sync is not of type HCA3OffsetClockSync. aborting..");
            exit(1);
        }
    } else {
        ZF_LOGV("using default HCA3O clock sync");
        clock_sync = new HCA3OffsetClockSync(new SKaMPIClockOffsetAlg(5,20));
    }

}

static void hca3_offset_print_sync_parameters(FILE* f)
{
    fprintf (f, "#@clocksync=HCA3O\n");
}


extern "C"
void register_hca_module(mpits_clocksync_t *sync_mod) {
  sync_mod->name = (char*)std::string("HCA").c_str();
  sync_mod->clocksync = MPITS_CLOCKSYNC_HCA;
  sync_mod->init_module = hca_init_module;
  sync_mod->cleanup_module = hca_cleanup_module;
  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = hca_print_sync_parameters;
}

extern "C"
void register_hca2_module(mpits_clocksync_t *sync_mod) {
  sync_mod->name = (char*)std::string("HCA2").c_str();
  sync_mod->clocksync = MPITS_CLOCKSYNC_HCA2;
  sync_mod->init_module = hca2_init_module;
  sync_mod->cleanup_module = hca_cleanup_module;
  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = hca2_print_sync_parameters;
}


extern "C"
void register_hca3_module(mpits_clocksync_t *sync_mod) {
  sync_mod->name = (char*)std::string("HCA3").c_str();
  sync_mod->clocksync = MPITS_CLOCKSYNC_HCA3;
  sync_mod->init_module = hca3_init_module;
  sync_mod->cleanup_module = hca_cleanup_module;
  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = hca3_print_sync_parameters;
}


extern "C"
void register_hca3_offset_module(mpits_clocksync_t *sync_mod) {
    sync_mod->name = (char*)std::string("HCA3O").c_str();
    sync_mod->clocksync = MPITS_CLOCKSYNC_HCA3_OFFSET;
    sync_mod->init_module = hca3_offset_init_module;
    sync_mod->cleanup_module = hca_cleanup_module;
    sync_mod->sync_clocks = synchronize_clocks;

    sync_mod->init_sync = default_init_synchronization;
    sync_mod->finalize_sync = default_finalize_synchronization;

    sync_mod->get_global_time = get_normalized_time;
    sync_mod->print_sync_info = hca3_offset_print_sync_parameters;
}


