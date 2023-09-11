#ifndef MPITS_CLOCK_SYNC_UTILS_H_
#define MPITS_CLOCK_SYNC_UTILS_H_

#include "time_provider/clocks/Clock.hpp"

void compute_rtt(int master_rank, int other_rank, MPI_Comm comm, int nwarmups, int n_pingpongs, Clock& c,  double *rtt);

#endif /* MPITS_CLOCK_SYNC_UTILS_H_ */
