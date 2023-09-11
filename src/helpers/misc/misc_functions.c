//
// Created by Sascha on 9/10/23.
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#include "mpits_helpers.h"

double mpits_min(double a, double b) {
  return (a < b) ? a : b;
}

double mpits_max(double a, double b) {
  return (a > b) ? a : b;
}


void mpits_shuffle(int *array, size_t n) {
  srand(time(NULL));

  if (n > 1) {
    size_t i;
    for (i = 0; i < n - 1; i++) {
      size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}


int mpits_str_to_long(const char *str, long* result) {
  char *endptr;
  int error = 0;
  long res;

  errno = 0;
  res = strtol(str, &endptr, 10);

  /* Check for various possible errors */
  if ((errno == ERANGE && (res == LONG_MAX || res == LONG_MIN)) || (errno != 0 && res == 0)) {
    error = 1;
  }
  if (endptr == str) {  // no digits parsed
    error = 1;
  }
  if (!error) {
    *result = res;
  }
  else {
    *result = 0;
  }

  return error;
}
