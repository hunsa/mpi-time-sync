/*
 * clock_sync_common.cpp
 *
 *  Created on: Mar 14, 2018
 *      Author: sascha
 */

#include <iostream>
#include "time_provider/clocks/GlobalClock.hpp"

#if defined(ENABLE_GETTIME_REALTIME) || defined(ENABLE_GETTIME_MONOTONIC)
#include "time_provider/clocks/GettimeClock.h"
#else
#include "time_provider/clocks/MPIClock.hpp"
#endif


#include "mpi_clock_sync_internal.h"
#include "clock_sync_common.h"

void default_init_synchronization() {}
void default_finalize_synchronization() {}

double default_get_normalized_time(double local_time, GlobalClock *global_clock) {
  double normtime = 0;

  if (global_clock != nullptr) {
    normtime = global_clock->convert_to_global_time(local_time);
  } else {
    std::cerr << "ERROR: No global time defined for this clock sync. method\n" << std::endl;
  }
  return normtime;
}

Clock *initialize_local_clock() {
  Clock *local_clock = nullptr;
#if ENABLE_GETTIME_REALTIME
  local_clock = new GettimeClock(GettimeClock::LocalClockType::LOCAL_CLOCK_REALTIME);
#elif ENABLE_GETTIME_MONOTONIC
  local_clock = new GettimeClock(GettimeClock::LocalClockType::LOCAL_CLOCK_MONOTONIC);
#else
  local_clock = new MPIClock();
#endif

  return local_clock;
}

