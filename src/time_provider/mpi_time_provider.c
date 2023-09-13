//
// Created by Sascha on 8/8/23.
//

#include <mpi.h>
#include "mpits.h"

#if defined(ENABLE_GETTIME_REALTIME) || defined(ENABLE_GETTIME_MONOTONIC)
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

double wtime;
struct timespec ts;
#endif


double MPITS_get_time(void) {
#ifdef ENABLE_GETTIME_REALTIME
  if( clock_gettime( CLOCK_REALTIME, &ts) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }
    wtime = (double)(ts.tv_nsec) / 1.0e+9 + ts.tv_sec;
    return wtime;
#elif ENABLE_GETTIME_MONOTONIC
  if( clock_gettime( CLOCK_MONOTONIC, &ts) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }
    wtime = (double)(ts.tv_nsec) / 1.0e+9 + ts.tv_sec;
    return wtime;
#else
  return PMPI_Wtime();
#endif
}