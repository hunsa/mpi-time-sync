#ifndef MPITS_HCA3OFFSETCLOCKSYNC_H
#define MPITS_HCA3OFFSETCLOCKSYNC_H

#include "clock_sync/sync_algorithms/ClockSync.hpp"
#include "clock_sync/clock_offset_algs/ClockOffsetAlg.hpp"

/**
 *
 * essentially the same algorithm as HCA3, except that we only learn the clock offset
 */

class HCA3OffsetClockSync : public BaseClockSync {

public:
  HCA3OffsetClockSync(ClockOffsetAlg *offsetAlg);
  ~HCA3OffsetClockSync();
  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
  GlobalClock* create_global_dummy_clock(MPI_Comm comm, Clock& c);

private:
  ClockOffsetAlg *offset_alg;
};


#endif //MPITS_HCA3OFFSETCLOCKSYNC_H
