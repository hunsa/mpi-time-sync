//
// Created by Sascha on 9/10/23.
//

#ifndef MPI_TIME_SYNC_SK_SYNC_H
#define MPI_TIME_SYNC_SK_SYNC_H

#include "mpits.h"

#ifdef __cplusplus
extern "C" {
#endif

void register_skampi_module(mpits_clocksync_t *sync_mod);

#ifdef __cplusplus
}
#endif

#endif //MPI_TIME_SYNC_SK_SYNC_H
