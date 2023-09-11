
#ifndef MPITS_CLOCKSYNC_CLASS_H_
#define MPITS_CLOCKSYNC_CLASS_H_

#include <mpi.h>

#include "time_provider/clocks/Clock.hpp"
#include "time_provider/clocks/GlobalClock.hpp"

class ClockSync {

//protected:
//	MPI_Comm comm;
//	Clock* local_clock;

public:
  virtual GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c) = 0;
  virtual ~ClockSync() {};

};

class BaseClockSync : public ClockSync {
public:
  virtual GlobalClock* create_global_dummy_clock(MPI_Comm comm, Clock& c) = 0;
  virtual ~BaseClockSync() {};
};

class ComposedClockSync : public ClockSync {
public:
  virtual ~ComposedClockSync() {};
};

class LinModel {
public:
  double slope;
  double intercept;
};


#endif /*  MPITS_CLOCKSYNC_CLASS_H_  */
