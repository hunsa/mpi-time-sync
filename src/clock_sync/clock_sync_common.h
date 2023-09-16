
#ifndef MPITS_CLOCK_SYNCHRONIZATION_COMMON_H_
#define MPITS_CLOCK_SYNCHRONIZATION_COMMON_H_

#include "time_provider/clocks/GlobalClock.hpp"


void default_init_synchronization();

void default_finalize_synchronization();

double default_get_normalized_time(double local_time, GlobalClock *global_clock);

Clock *initialize_local_clock();


#endif /* MPITS_CLOCK_SYNCHRONIZATION_COMMON_H_ */
