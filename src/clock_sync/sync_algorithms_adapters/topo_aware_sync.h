//
// Created by Sascha on 9/10/23.
//

#ifndef MPI_TIME_SYNC_TOPO_AWARE_SYNC_H
#define MPI_TIME_SYNC_TOPO_AWARE_SYNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mpits.h"

void register_topo_aware_sync1_module(mpits_clocksync_t *sync_mod);

#ifdef __cplusplus
}
#endif

#endif //MPI_TIME_SYNC_TOPO_AWARE_SYNC_H
