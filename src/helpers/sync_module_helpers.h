
#ifndef MPITS_SYNC_MODULE_HELPERS_H_
#define MPITS_SYNC_MODULE_HELPERS_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "helpers/dict//mpits_dict.h"

typedef struct sync_module_info {
  char* name;
} sync_module_info_t;


typedef struct sync_type {
  char* name;
  int type;
} sync_type_t;

//void parse_sync_options(int argc, char **argv, const char* argument_name, sync_module_info_t* opts_p);
void cleanup_sync_options(sync_module_info_t* opts_p) ;
//int get_sync_type(const int n_types, const sync_type_t* type_list, const char* name);
//char* get_name_from_sync_type(const int n_types, const sync_type_t* type_list, int type);

mpits_map* get_global_param_store();

#ifdef __cplusplus
}
#endif

#endif /* MPITS_SYNC_MODULE_HELPERS_H_ */
