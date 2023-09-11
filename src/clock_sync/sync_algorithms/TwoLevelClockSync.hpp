
#ifndef MPITS_TWO_LEVEL_CLOCKSYNC_CLASS_H_
#define MPITS_TWO_LEVEL_CLOCKSYNC_CLASS_H_

#include "time_provider/clocks/Clock.hpp"
#include "time_provider/clocks/GlobalClock.hpp"
#include "clock_sync/sync_algorithms/utils/communicator_utils.h"
#include "ClockSync.hpp"

class TwoLevelClockSync: public ClockSync {

private:
  BaseClockSync *syncInterNode;
  BaseClockSync *syncIntraNode;

  MPI_Comm comm_internode;
  MPI_Comm comm_intranode;

  bool comm_initialized;

  void initialized_communicators(MPI_Comm comm);

public:
  TwoLevelClockSync(BaseClockSync *syncInterNode, BaseClockSync *syncIntraNode);
  ~TwoLevelClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
};

#endif /*  MPITS_TWO_LEVEL_CLOCKSYNC_CLASS_H_  */
