
#ifndef MPI_TIME_SYNC_JK_SYNC_H
#define MPI_TIME_SYNC_JK_SYNC_H

#include "mpi_clock_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

void register_jk_module(mpits_clocksync_t *sync_mod);

#ifdef __cplusplus
}
#endif

#endif //MPI_TIME_SYNC_JK_SYNC_H
