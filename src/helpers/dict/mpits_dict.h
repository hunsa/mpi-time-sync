//
// Created by Sascha on 9/5/23.
//

#ifndef MPI_TIME_SYNC_MPITS_DICT_H
#define MPI_TIME_SYNC_MPITS_DICT_H

#include<stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* mpits_map;

mpits_map mpits_init_dictionary();
void mpits_cleanup_dictionary(mpits_map map);
int mpits_add_element_to_dict(mpits_map map, const char* key, const char* val);
int mpits_get_value_from_dict(const mpits_map map, const char* key, char** value);
int mpits_remove_element_from_dict(mpits_map map, const char* key);
int mpits_get_keys_from_dict(const mpits_map map, char ***keys, int *nkeys);
int mpits_dict_is_empty(const mpits_map map);
int mpits_dict_get_nkeys(const mpits_map map);
int mpits_dict_has_key(const mpits_map map, const char *key);
void mpits_print_dictionary(const mpits_map map, FILE* f);

#ifdef __cplusplus
}
#endif

#endif //MPI_TIME_SYNC_MPITS_DICT_H
