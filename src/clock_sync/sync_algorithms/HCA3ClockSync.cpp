/*
 * HCA3ClockSync.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: sascha
 */


#include <math.h>
#include <sstream>
#include <vector>

#include "HCA3ClockSync.hpp"
#include "LinearModelFitterStandard.hpp"
#include "LinearModelFitterDebug.hpp"
#include "clock_sync/clock_offset_algs/PingpongClockOffsetAlg.hpp"
#include "clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.hpp"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


HCA3ClockSync::HCA3ClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints, bool recompute_intercept) :
    HCAAbstractClockSync(offsetAlg, n_fitpoints), recompute_intercept(recompute_intercept) {
}

HCA3ClockSync* HCA3ClockSync::from_string(const std::string& str) {
  bool recompute         = true;
  int n_fitpoints        = 500;
  ClockOffsetAlg* offset_alg = new SKaMPIClockOffsetAlg(5, 20);

  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, '@')) {
    tokens.push_back(token);
  }

  if (tokens.size() >= 5) {
    recompute   = std::stoi(tokens[0]) == 1;
    n_fitpoints = std::stoi(tokens[1]);
    std::string alg = tokens[2];
    int p1      = std::stoi(tokens[3]);
    int p2      = std::stoi(tokens[4]);

    ClockOffsetAlg* parsed_alg = nullptr;
    if (alg == "pingpong_offset") {
      parsed_alg = new PingpongClockOffsetAlg(p1, p2);
    } else if (alg == "skampi_offset") {
      parsed_alg = new SKaMPIClockOffsetAlg(p1, p2);
    }
    if (parsed_alg != nullptr) {
      delete offset_alg;
      offset_alg = parsed_alg;
    }
  } else {
    ZF_LOGW("using default HCA3ClockSync parameters: recompute=%d, fitpoints=%d, offset_alg=skampi_offset@5@20", recompute, n_fitpoints);
  }

  if (n_fitpoints < 2) {
    ZF_LOGW("fitpoints=%d is too small, using 2", n_fitpoints);
    n_fitpoints = 2;
  }

  return new HCA3ClockSync(offset_alg, n_fitpoints, recompute);
}

HCA3ClockSync::~HCA3ClockSync() {

}


void HCA3ClockSync::remeasure_intercept_call_back(MPI_Comm comm, Clock &c,LinModel* lm, int client, int p_ref) {
  ZF_LOGV("remeasure_intercept_call_back in HCA3");
  if (true == this->recompute_intercept) {
    remeasure_intercept(comm, c, lm, client, p_ref);
  }
}

void HCA3ClockSync::remeasure_all_intercepts_call_back(MPI_Comm comm, Clock &c, LinModel* lm, const int ref_rank) {
  ZF_LOGV("compute remeasure_all_intercepts_call_back in HCA3");
  // do nothing here
}


GlobalClock* HCA3ClockSync::synchronize_all_clocks(MPI_Comm comm, Clock& c) {
  int my_rank, nprocs;
  int i;
  int max_power_two, nrounds_step1;
  int running_power, next_power;
  int other_rank;
  LinModel lm;
  GlobalClock* my_clock;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &nprocs);

  nrounds_step1 = floor(log2((double) nprocs));
  max_power_two = (int) pow(2.0, (double) nrounds_step1);

  ZF_LOGV("%d (%d): nrounds_step1:%d max_power_two:%d", my_rank, nprocs, nrounds_step1, max_power_two);

  //MPI_Type_create_struct(2, blocklen, disp, dtype, &mpi_lm_t);
  //MPI_Type_commit(&mpi_lm_t);

  my_clock = new GlobalClockLM(c, 0, 0);
  for (i = nrounds_step1; i > 0; i--) {
     running_power = (int) pow(2.0, (double) i);
     next_power = (int) pow(2.0, (double)(i-1));

     ZF_LOGV("%d: running_power:%d next_power:%d", my_rank, running_power, next_power);

     if (my_rank >= max_power_two) {
       // nothing to do here
     } else {
       if (my_rank % running_power == 0) { // parent
         other_rank = my_rank + next_power;
         ZF_LOGV("[rank=%d] parent=%d child=%d", my_rank, my_rank, other_rank);

         // compute model through linear regression
         lm = learn_model(comm, *(my_clock), my_rank, other_rank);

       } else if (my_rank % running_power == next_power) { // child
         other_rank = my_rank - next_power;
         ZF_LOGV("[rank=%d] parent=%d child=%d", my_rank, other_rank, my_rank);

         // compute model through linear regression
         lm = learn_model(comm, *(my_clock), other_rank, my_rank);

         // compute current clock as a global clock based on my drift model with the parent
         my_clock = new GlobalClockLM(c, lm.slope, lm.intercept);
       }
     }
     //MPI_Barrier(this->comm);
   }

  // compute the clock models for ranks >= max_power_two (which learn models from ranks 0..(nprocs - max_power_two) in one round)
  if (my_rank >= max_power_two) { // child
    other_rank = my_rank - max_power_two;
    // compute model through linear regression
    lm = learn_model(comm, *(my_clock), other_rank, my_rank);

    // compute current clock as a global clock based on my drift model with the parent
    my_clock = new GlobalClockLM(c, lm.slope, lm.intercept);
  } else if (my_rank < (nprocs - max_power_two)) { // parent
    other_rank = my_rank + max_power_two;

    // compute model through linear regression
    lm = learn_model(comm, *(my_clock), my_rank, other_rank);
  }

   return my_clock;
}



