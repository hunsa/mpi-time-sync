//
// Created by Sascha on 9/8/23.
//

#ifndef MPI_TIME_SYNC_NO_SYNC_H
#define MPI_TIME_SYNC_NO_SYNC_H

#include "mpi_clock_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

void register_no_clock_sync_module(mpits_clocksync_t *sync_mod);

#ifdef __cplusplus
}
#endif


#endif //MPI_TIME_SYNC_NO_SYNC_H
