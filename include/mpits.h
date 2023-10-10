#ifndef MPI_TIME_SYNC_MPITS_H
#define MPI_TIME_SYNC_MPITS_H

#include <stdio.h>
#include "mpi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef double (*sync_normtime_t)(double local_time);
typedef void (*print_sync_info_t)(FILE *f);

typedef enum {
  MPITS_CLOCKSYNC_HCA = 0,
  MPITS_CLOCKSYNC_HCA2,
  MPITS_CLOCKSYNC_HCA3,
  MPITS_CLOCKSYNC_HCA3_OFFSET,
  MPITS_CLOCKSYNC_JK,
  MPITS_CLOCKSYNC_SKAMPI,
#ifdef HAVE_HWLOC
  MPITS_CLOCKSYNC_TOPO1,
#endif
  MPITS_CLOCKSYNC_TOPO2,
  MPITS_CLOCKSYNC_NONE
} mpits_clocksync_alg_t;

typedef struct mpits_sync_module {
  void (*init_module)(MPI_Comm, int argc, char **argv);

  void (*cleanup_module)(void);

  void (*init_sync)(MPI_Comm);

  void (*finalize_sync)(MPI_Comm);

  void (*sync_clocks)(MPI_Comm);

  sync_normtime_t get_global_time;
  print_sync_info_t print_sync_info;

  char *name;
  // a module is uniquely identified by the clock sync. method and the process synchronization method
  mpits_clocksync_alg_t clocksync;

  MPI_Comm comm;
} mpits_clocksync_t;


/**
 * Initialize the time synchronization library.
 *
 * It must be called after MPI_Init.
 *
 * @param comm
 * @param clocksync
 * @return
 */
int MPITS_Init(MPI_Comm comm, mpits_clocksync_t *clocksync);

/**
 * Finalize call to time synchronization library.
 *
 * Must be called before MPI_Finalize.
 *
 * @return
 */
int MPITS_Finalize();


/**
 * Initialize a clock synchronization algorithm.
 * @param clocksync
 * @return
 */

int MPITS_Clocksync_init(mpits_clocksync_t *clocksync);

/**
 * Perform a clock synchronization.
 * @param clocksync
 * @return
 */

int MPITS_Clocksync_sync(mpits_clocksync_t *clocksync);

/**
 * Perform a resynchronizatin of the clock.
 *
 * Must be called after one call to MPITS_Clocksync_sync.
 *
 * @param clocksync
 * @return
 */

int MPITS_Clocksync_resync(mpits_clocksync_t *clocksync);

/**
 * Finalize a clock synchronization algorithm.
 * @param clocksync
 * @return
 */

int MPITS_Clocksync_finalize(mpits_clocksync_t *clocksync);


/**
 * This function returns the current global time of a process.
 *
 * Internally, it will call MPITS_get_time() and convert the local timestamp
 * into a global timestamp depending on the clock synchronization method selected.
 *
 * @param clocksync
 * @return global time stamp
 */
double MPITS_Clocksync_get_time(mpits_clocksync_t *clocksync);


/**
 * This is a time wrapper for the actual time on each process.
 * It abstracts from time sources such as MPI_Wtime or clock_gettime.
 *
 * @return current time
 */
double MPITS_get_time(void);

#ifdef __cplusplus
}
#endif

#endif //MPI_TIME_SYNC_MPITS_H
