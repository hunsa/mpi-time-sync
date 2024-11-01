//
// Created by Sascha on 9/13/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <mpi.h>

#include "cli_param_parser.h"


static mpits_map params_dict;
static int params_dict_initialized = 0;

static const int STRING_SIZE = 256;

enum {
  MPITS_ARGS_PARAMS = 700
};


static const struct option reprompi_params_long_options[] = {
        {"params", optional_argument, 0, MPITS_ARGS_PARAMS},
        { 0, 0, 0, 0 }
};
static const char reprompi_params_opts_str[] = "";

static void exit_after_keyvalue_error(const char* error_str) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0) {
    fprintf(stderr, "\nERROR: %s\n\n", error_str);
  }
  MPI_Finalize();
  exit(1);
}



static void parse_keyvalue_list(char* args, mpits_map dict) {
  char* params_tok;
  char *save_str, *s, *keyvalue_list;
  char *kv_str, *kv_s;
  char* key;
  char* val;
  int ok;

  save_str = (char*) malloc(STRING_SIZE * sizeof(char));
  kv_str   = (char*) malloc(STRING_SIZE * sizeof(char));
  s    = save_str;
  kv_s = kv_str;

  /* Parse the list of message sizes */
  if (args != NULL) {

    keyvalue_list = strdup(args);
    params_tok = strtok_r(keyvalue_list, ",", &save_str);
    while (params_tok != NULL) {
      key = strtok_r(params_tok, ":", &kv_str);
      val = strtok_r(NULL, ":", &kv_str);

      if (key!=NULL && val!= NULL) {
        if (!mpits_dict_has_key(dict, key)) {
          ok = mpits_add_element_to_dict(dict, key, val);
          //printf("add '%s' -> '%s'\n", key, val);
          if (ok != 0) {
            exit_after_keyvalue_error("Cannot add parameter to dictionary");
          }
        }
        else {
          exit_after_keyvalue_error("Parameter already exists");
        }
      }
      else {
        exit_after_keyvalue_error("Key-value parameters invalid");
      }
      params_tok = strtok_r(NULL, ",", &save_str);
    }

    free(keyvalue_list);
  }

  free(s);
  free(kv_s);
}


void mpits_parse_extra_key_value_options(mpits_map dict, int argc, char **argv) {
  int c;
//  int nprocs, my_rank;
//  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
//  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  optind = 1;	// reset optind to enable option re-parsing, especially if we are not the first ones
  opterr = 0;
  while (1) {
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, reprompi_params_opts_str, reprompi_params_long_options,
                    &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;
    switch (c) {
      case MPITS_ARGS_PARAMS: /* list of key-value parameters */
        parse_keyvalue_list(optarg, dict);
        break;
      case '?':
        break;
    }
  }
  optind = 1;	// reset optind to enable option re-parsing
  opterr = 1;	// reset opterr to catch invalid options
}


mpits_map mpits_get_global_param_store() {
  if( params_dict_initialized == 0 ) {
    params_dict = mpits_init_dictionary();
    params_dict_initialized = 1;
  }
  return params_dict;
}

