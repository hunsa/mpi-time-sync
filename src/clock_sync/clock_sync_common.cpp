/*
 * clock_sync_common.cpp
 *
 *  Created on: Mar 14, 2018
 *      Author: sascha
 */

#include <iostream>
#include "time_provider/clocks/GlobalClock.hpp"
#include "time_provider/clocks/BaseClock.hpp"
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
  return new BaseClock();
}

