//
// Created by Sascha on 5/11/23.
//

#ifndef MPITS_CLOCK_DRIFT_UTILS_H
#define MPITS_CLOCK_DRIFT_UTILS_H

#include <mpi.h>
#include "mpi_clock_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

double SKaMPIClockOffset_measure_offset(MPI_Comm comm, int ref_rank, int client_rank, mpits_clocksync_t *clock_sync);

void generate_test_process_list(double process_ratio, int **testprocs_list_p, int *ntestprocs);

extern int Minimum_ping_pongs;
extern int Number_ping_pongs;
extern const int OUTPUT_ROOT_PROC;

#ifdef __cplusplus
}
#endif


#endif // MPITS_CLOCK_DRIFT_UTILS_H
