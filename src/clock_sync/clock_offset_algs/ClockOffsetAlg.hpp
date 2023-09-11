
#ifndef MPITS_CLOCKOFFSETALG_CLASS_H_
#define MPITS_CLOCKOFFSETALG_CLASS_H_

#include <mpi.h>
#include "time_provider/clocks/Clock.hpp"
#include "ClockOffset.hpp"

class ClockOffsetAlg {

public:
  ClockOffsetAlg();

  virtual ClockOffset* measure_offset(MPI_Comm comm, int ref_rank, int other_rank, Clock& clock) = 0;
  virtual ~ClockOffsetAlg() = 0;

};

#endif /*  MPITS_CLOCKOFFSETALG_CLASS_H_  */
