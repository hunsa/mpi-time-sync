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
  void (*init_module)(int argc, char **argv);

  void (*cleanup_module)(void);

  void (*init_sync)(void);

  void (*finalize_sync)(void);

  void (*sync_clocks)(void);

  sync_normtime_t get_global_time;
  print_sync_info_t print_sync_info;

  char *name;
  // a module is uniquely identified by the clock sync. method and the process synchronization method
  mpits_clocksync_alg_t clocksync;
} mpits_clocksync_t;


int MPITS_Init(MPI_Comm comm, mpits_clocksync_t *clocksync);

int MPITS_Clocksync_init(mpits_clocksync_t *clocksync);

int MPITS_Clocksync_sync(mpits_clocksync_t *clocksync);

int MPITS_Clocksync_resync(mpits_clocksync_t *clocksync);

int MPITS_Clocksync_finalize(mpits_clocksync_t *clocksync);

double MPITS_Clocksync_get_time(mpits_clocksync_t *clocksync);

int MPITS_Finalize();

double MPITS_get_time(void);

#ifdef __cplusplus
}
#endif

#endif //MPI_TIME_SYNC_MPITS_H
