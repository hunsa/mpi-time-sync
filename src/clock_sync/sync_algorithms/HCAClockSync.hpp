#ifndef MPITS_HCACLOCKSYNC_CLASS_H_
#define MPITS_HCACLOCKSYNC_CLASS_H_

#include <mpi.h>

#include "HCAAbstractClockSync.hpp"


/**
 *
 * O(log p) rounds to learn linear regression models
 * O(p) rounds to fix intercept (only works for smaller p)
 *
 */

class HCAClockSync: public HCAAbstractClockSync {

protected:

  virtual void remeasure_intercept_call_back(MPI_Comm comm, Clock &c, LinModel* lm, int client, int p_ref);
  virtual void remeasure_all_intercepts_call_back(MPI_Comm comm, Clock &c, LinModel* lm, const int ref_rank);

public:
  HCAClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints);
  ~HCAClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
};

#endif /*  MPITS_HCACLOCKSYNC_CLASS_H_  */
