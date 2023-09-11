//
// Created by Sascha on 9/8/23.
//

#ifndef MPI_TIME_SYNC_HCA_SYNC_H
#define MPI_TIME_SYNC_HCA_SYNC_H

#include "mpi_clock_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

void register_hca_module(mpits_clocksync_t *sync_mod);
void register_hca2_module(mpits_clocksync_t *sync_mod);
void register_hca3_module(mpits_clocksync_t *sync_mod);
void register_hca3_offset_module(mpits_clocksync_t *sync_mod);

#ifdef __cplusplus
}
#endif


#endif //MPI_TIME_SYNC_HCA_SYNC_H
