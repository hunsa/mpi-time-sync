/*
 * HCA2ClockSync.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: sascha
 */

#include <math.h>
#include <gsl/gsl_fit.h>
#include <sstream>
#include <vector>

#include "HCA2ClockSync.hpp"
#include "LinearModelFitterStandard.hpp"
#include "clock_sync/clock_offset_algs/PingpongClockOffsetAlg.hpp"
#include "clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.hpp"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

HCA2ClockSync::HCA2ClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints, bool recompute_intercept) :
  HCAClockSync(offsetAlg, n_fitpoints), recompute_intercept(recompute_intercept) {
}

HCA2ClockSync::~HCA2ClockSync() {
}

HCA2ClockSync* HCA2ClockSync::from_string(const std::string& str) {
  bool recompute        = true;
  int n_fitpoints       = 500;
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
    // print that we use default values
    ZF_LOGW("using default HCA2ClockSync parameters: recompute=%d, fitpoints=%d, offset_alg=skampi_offset@5@20", recompute, n_fitpoints); 
  }

  if (n_fitpoints < 2) {
    ZF_LOGW("fitpoints=%d is too small, using 2", n_fitpoints);
    n_fitpoints = 2;
  }

  return new HCA2ClockSync(offset_alg, n_fitpoints, recompute);
}

void HCA2ClockSync::remeasure_intercept_call_back(MPI_Comm comm, Clock &c, LinModel* lm, int client, int p_ref) {
  ZF_LOGV("compute intercept call back in HCA2");

  if (true == this->recompute_intercept) {
    remeasure_intercept(comm, c, lm, client, p_ref);
  }

}

void HCA2ClockSync::remeasure_all_intercepts_call_back(MPI_Comm comm, Clock &c, LinModel* lm, const int ref_rank) {
  ZF_LOGV("compute all intercepts call back in HCA2");
  // empty call back here
  // nothing to be done for HCA2
}
