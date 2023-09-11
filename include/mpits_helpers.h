//
// Created by Sascha on 9/10/23.
//

#ifndef MPI_TIME_SYNC_MPITS_HELPERS_H
#define MPI_TIME_SYNC_MPITS_HELPERS_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

double mpits_min(double a, double b);
double mpits_max(double a, double b);
void mpits_shuffle(int *array, size_t n);

int mpits_str_to_long(const char *str, long *result);

#ifdef __cplusplus
}
#endif

#endif //MPI_TIME_SYNC_MPITS_HELPERS_H
