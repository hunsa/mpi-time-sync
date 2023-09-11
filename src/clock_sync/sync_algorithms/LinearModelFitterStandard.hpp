/*
 * LinearModelFitterStandard.hpp
 *
 *  Created on: Apr 24, 2018
 *      Author: sascha
 */

#ifndef MPITS_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_LINEARMODELFITTERSTANDARD_HPP_
#define MPITS_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_LINEARMODELFITTERSTANDARD_HPP_

#include "LinearModelFitter.hpp"

class LinearModelFitterStandard : public LinearModelFitter {

public:
  LinearModelFitterStandard();
  ~LinearModelFitterStandard();

  int fit_linear_model(const double *xvals, const double *yvals, const int nb_vals, double *slope, double *intercept);

};



#endif /* MPITS_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_LINEARMODELFITTERSTANDARD_HPP_ */
