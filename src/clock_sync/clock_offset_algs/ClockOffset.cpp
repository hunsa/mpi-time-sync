

#include "ClockOffset.hpp"


ClockOffset::ClockOffset(double timestamp, double offset) :
  timestamp(timestamp), offset(offset)
{}

ClockOffset::~ClockOffset()
{}

double ClockOffset::get_timestamp(void) {
  return timestamp;
}

double ClockOffset::get_offset(void) {
  return offset;
}

