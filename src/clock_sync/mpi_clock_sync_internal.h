
#ifndef CLOCK_SYNCHRONIZATION_LIB_H_
#define CLOCK_SYNCHRONIZATION_LIB_H_

#include "mpi_clock_sync.h"
#include "clock_sync_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sync_module_info {
  char* name;
} sync_module_info_t;


typedef struct sync_type {
  const char* name;
  int type;
} sync_type_t;

void MPITS_register_sync_modules(void);
void MPITS_deregister_sync_modules(void);

#ifdef __cplusplus
}
#endif

#endif

