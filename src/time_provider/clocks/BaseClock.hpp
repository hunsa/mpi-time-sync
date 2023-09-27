//
// Created by Sascha on 9/27/23.
//

#ifndef MPI_TIME_SYNC_BASECLOCK_H
#define MPI_TIME_SYNC_BASECLOCK_H

#include <mpi.h>
#include "Clock.hpp"

class BaseClock : public Clock {

public:
  BaseClock();
  ~BaseClock();

  double get_time(void);
  bool is_base_clock();

};


#endif //MPI_TIME_SYNC_BASECLOCK_H
