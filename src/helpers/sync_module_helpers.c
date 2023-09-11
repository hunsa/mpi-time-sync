
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <mpi.h>

#include "sync_module_helpers.h"

static mpits_map params_dict;
static const int HASHTABLE_SIZE=100;
static int params_dict_initialized = 0;

enum{
  MPITS_ARGS_SYNC_TYPE = 1
};

mpits_map* get_global_param_store() {
  if( params_dict_initialized == 0 ) {
    mpits_init_dictionary(&params_dict, HASHTABLE_SIZE);
    params_dict_initialized = 1;
  }
  return &params_dict;
}

void cleanup_sync_options(sync_module_info_t* opts_p) {
  if (opts_p != NULL && opts_p->name != NULL) {
    free(opts_p->name);
  }
}

char* get_name_from_sync_type(const int n_types, const sync_type_t* type_list, int type) {
  int i;
  char *name = NULL;

  for (i=0; i<n_types; i++) {
    if (type_list[i].type == type) {
      name = strdup(type_list[i].name);
    }
  }
  return name;
}
