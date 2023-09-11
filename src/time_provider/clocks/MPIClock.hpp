
#ifndef MPITS_MPI_CLOCK_CLASS_H_
#define MPITS_MPI_CLOCK_CLASS_H_

#include <mpi.h>
#include "Clock.hpp"

class MPIClock : public Clock {

public:
  MPIClock();
  ~MPIClock();

  double get_time(void);
  bool is_base_clock();

};




#endif /*  MPITS_MPI_CLOCK_CLASS_H_  */
