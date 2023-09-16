
#ifndef CLOCK_SYNC_CLOCK_SYNC_LOADER_HPP_
#define CLOCK_SYNC_CLOCK_SYNC_LOADER_HPP_

#include "clock_sync/sync_algorithms/ClockSync.hpp"

class ClockSyncLoader {

public:
  BaseClockSync* instantiate_clock_sync(const char *param_name);
};


#endif
