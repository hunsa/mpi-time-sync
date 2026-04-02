/*
 * JKClockSync.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: sascha
 */

#include <iostream>
#include <sstream>
#include <vector>

#include <mpi.h>
#include <gsl/gsl_fit.h>

#include "JKClockSync.hpp"
#include "clock_sync/clock_offset_algs/PingpongClockOffsetAlg.hpp"
#include "clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.hpp"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


JKClockSync::JKClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints) {
  this->offset_alg = offsetAlg;
  this->n_fitpoints = n_fitpoints;
//  this->n_exchanges = n_exchanges;
}

JKClockSync::~JKClockSync() {
}

JKClockSync* JKClockSync::from_string(const std::string& str) {
  int n_fitpoints = 1000;
  ClockOffsetAlg* offset_alg = new PingpongClockOffsetAlg(100, 100);

  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, '@')) {
    tokens.push_back(token);
  }

  if (tokens.size() >= 4) {
    n_fitpoints = std::stoi(tokens[0]);
    std::string alg = tokens[1];
    int p1 = std::stoi(tokens[2]);
    int p2 = std::stoi(tokens[3]);

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
    ZF_LOGW("using default JKClockSync parameters: fitpoints=%d, offset_alg=pingpong_offset@100@100", n_fitpoints);
  }

  if (n_fitpoints < 2) {
    ZF_LOGW("fitpoints=%d is too small, using 2", n_fitpoints);
    n_fitpoints = 2;
  }

  return new JKClockSync(offset_alg, n_fitpoints);
}


GlobalClock* JKClockSync::synchronize_all_clocks(MPI_Comm comm, Clock& c) {
  int j, p;
  int my_rank, np;
  int root_rank = 0;
  double slope, intercept;

   slope = 0;
   intercept = 0;

   MPI_Comm_rank(comm, &my_rank);
   MPI_Comm_size(comm, &np);

   if (my_rank == root_rank) {

       ZF_LOGV("jk:root m offset with %d procs", np);
       for (j = 0; j < this->n_fitpoints; j++) {
           for (p = 0; p < np; p++) {
               if (p != root_rank) {
                 ZF_LOGV("jk:root=%d m offset with %d", my_rank, p);
                 ClockOffset* offset = offset_alg->measure_offset(comm, root_rank, p, c);
                 delete offset;
                 ZF_LOGV("jk:root=%d m offset with %d DONE", my_rank, p);
               }
           }
       }
       ZF_LOGV("jk:root m offset DONE---");
   } else {
     double *xfit, *yfit;
     double cov00, cov01, cov11, sumsq;

     xfit = new double[this->n_fitpoints];
     yfit = new double[this->n_fitpoints];


     for (j = 0; j < this->n_fitpoints; j++) {
       ZF_LOGV("jk:%d m offset with root=%d", my_rank, root_rank);
       ClockOffset* offset = offset_alg->measure_offset(comm, root_rank, my_rank, c);
       xfit[j] = offset->get_timestamp();
       yfit[j] = offset->get_offset();
       delete offset;
       ZF_LOGV("jk:%d m offset with root=%d DONE", my_rank, root_rank);
     }


     gsl_fit_linear(xfit, 1, yfit, 1, this->n_fitpoints, &intercept, &slope, &cov00, &cov01, &cov11, &sumsq);

     delete[] xfit;
     delete[] yfit;
   }
   return new GlobalClockLM(c, slope, intercept);
}

GlobalClock* JKClockSync::create_global_dummy_clock(MPI_Comm comm, Clock& c) {
  return new GlobalClockLM(c, 0.0, 0.0);
}
