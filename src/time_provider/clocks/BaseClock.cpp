//
// Created by Sascha on 9/27/23.
//
#include "mpits.h"
#include "BaseClock.hpp"

BaseClock::BaseClock() {

}

BaseClock::~BaseClock() {

}

double BaseClock::get_time(void) {
  return MPITS_get_time();
}

bool BaseClock::is_base_clock() {
  return true;
}
