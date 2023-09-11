#ifndef MPITS_JKCLOCKSYNC_CLASS_H_
#define MPITS_JKCLOCKSYNC_CLASS_H_


#include "time_provider/clocks/Clock.hpp"
#include "time_provider/clocks/GlobalClockLM.hpp"

#include "clock_sync/sync_algorithms/ClockSync.hpp"
#include "clock_sync/clock_offset_algs/ClockOffsetAlg.hpp"


class JKClockSync: public BaseClockSync {

private:
  int n_fitpoints; /* --fitpoints */
//  int n_exchanges; /* --exchanges */
  ClockOffsetAlg* offset_alg;

public:
  JKClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints);
  ~JKClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
  GlobalClock* create_global_dummy_clock(MPI_Comm comm, Clock& c);
};

#endif /*  MPITS_JKCLOCKSYNC_CLASS_H_  */
