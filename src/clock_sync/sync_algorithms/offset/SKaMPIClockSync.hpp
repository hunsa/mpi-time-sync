
#ifndef MPITS_SKAMPICLOCKSYNC_CLASS_H_
#define MPITS_SKAMPICLOCKSYNC_CLASS_H_

#include "time_provider/clocks/Clock.hpp"
#include "clock_sync/clock_offset_algs/ClockOffsetAlg.hpp"
#include "clock_sync/sync_algorithms/ClockSync.hpp"


class SKaMPIClockSync : public BaseClockSync {

private:
   double *tds; /* tds[i] is the time difference between the
   current node and global node i */
   ClockOffsetAlg* offset_alg;

public:
	SKaMPIClockSync(ClockOffsetAlg *offsetAlg);
  ~SKaMPIClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
  GlobalClock* create_global_dummy_clock(MPI_Comm comm, Clock& c);
};




#endif /*  MPITS_SKAMPICLOCKSYNC_CLASS_H_  */
