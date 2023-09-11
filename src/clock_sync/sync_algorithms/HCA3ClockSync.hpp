#ifndef MPITS_HCA3CLOCKSYNC_CLASS_H_
#define MPITS_HCA3CLOCKSYNC_CLASS_H_

#include <mpi.h>

#include "time_provider/clocks/Clock.hpp"
#include "time_provider/clocks/GlobalClockLM.hpp"
#include "clock_sync/clock_offset_algs/ClockOffsetAlg.hpp"
#include "clock_sync/sync_algorithms/HCAAbstractClockSync.hpp"

class HCA3ClockSync: public HCAAbstractClockSync {

private:
  bool recompute_intercept;

protected:
  void remeasure_intercept_call_back(MPI_Comm comm, Clock &c,LinModel* lm, int client, int p_ref);
  void remeasure_all_intercepts_call_back(MPI_Comm comm, Clock &c, LinModel* lm, const int ref_rank);

public:
  HCA3ClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints, bool recompute_intercept);
  ~HCA3ClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
};

#endif /*   MPITS_HCA3CLOCKSYNC_CLASS_H_  */
