//
// Created by Sascha on 5/11/23.
//

#ifndef MPITS_CLOCK_DRIFT_UTILS_H
#define MPITS_CLOCK_DRIFT_UTILS_H

#include <mpi.h>
#include "mpits.h"

#ifdef __cplusplus
extern "C" {
#endif

double SKaMPIClockOffset_measure_offset(MPI_Comm comm, int ref_rank, int client_rank, mpits_clocksync_t *clock_sync);

void generate_test_process_list(double process_ratio, int **testprocs_list_p, int *ntestprocs);

double mpits_min(double a, double b);
double mpits_max(double a, double b);
void mpits_shuffle(int *array, size_t n);

int mpits_str_to_long(const char *str, long *result);

extern int Minimum_ping_pongs;
extern int Number_ping_pongs;
extern const int OUTPUT_ROOT_PROC;

#ifdef __cplusplus
}
#endif


#endif // MPITS_CLOCK_DRIFT_UTILS_H
