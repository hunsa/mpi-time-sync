
#include <mpi.h>
#include "MPIClock.hpp"

MPIClock::MPIClock() {

}

MPIClock::~MPIClock() {

}

double MPIClock::get_time(void) {
  return PMPI_Wtime();
}

bool MPIClock::is_base_clock() {
  return true;
}
