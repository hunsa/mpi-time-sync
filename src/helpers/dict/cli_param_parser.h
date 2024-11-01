//
// Created by Sascha on 9/13/23.
//

#ifndef MPI_TIME_SYNC_CLI_PARAM_PARSER_H
#define MPI_TIME_SYNC_CLI_PARAM_PARSER_H

#include "mpits_dict.h"

#ifdef __cplusplus
extern "C" {
#endif

void mpits_parse_extra_key_value_options(mpits_map dict, int argc, char **argv);

mpits_map mpits_get_global_param_store();

#ifdef __cplusplus
}
#endif

#endif //MPI_TIME_SYNC_CLI_PARAM_PARSER_H
