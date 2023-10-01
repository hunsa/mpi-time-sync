
#include <getopt.h>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "mpits.h"
#include "mpi_clock_sync_internal.h"
#include "helpers/dict/mpits_dict.h"
#include "helpers/dict/cli_param_parser.h"

#include "sync_algorithms_adapters/hca_sync.h"
#include "sync_algorithms_adapters/jk_sync.h"
#include "sync_algorithms_adapters/sk_sync.h"
#include "sync_algorithms_adapters/no_sync.h"
#include "sync_algorithms_adapters/topo_aware_2l_sync.h"

#ifdef HAVE_HWLOC
#include "sync_algorithms_adapters/topo_aware_sync.h"
#endif

// Implemented synchronization modules
static mpits_clocksync_t* sync_modules;

void parse_sync_options(int argc, char **argv, const char* argument_name, sync_module_info_t* opts_p);
void cleanup_sync_options(sync_module_info_t* opts_p) ;
int get_sync_type(const int n_types, const sync_type_t* type_list, const char* name);
char* get_name_from_sync_type(const int n_types, const sync_type_t* type_list, int type);

static const sync_type_t clock_sync_options[] = {
        { "HCA", MPITS_CLOCKSYNC_HCA},
        { "HCA2", MPITS_CLOCKSYNC_HCA2},
        { "HCA3", MPITS_CLOCKSYNC_HCA3},
        { "HCA3O", MPITS_CLOCKSYNC_HCA3_OFFSET},
        { "JK", MPITS_CLOCKSYNC_JK },
        { "SKaMPI", MPITS_CLOCKSYNC_SKAMPI },
#ifdef HAVE_HWLOC
        { "Topo1", MPITS_CLOCKSYNC_TOPO1 },
#endif
        { "Topo2", MPITS_CLOCKSYNC_TOPO2 },
        { "None", MPITS_CLOCKSYNC_NONE }
};
static const int N_CLOCK_SYNC_TYPES = sizeof(clock_sync_options)/sizeof(sync_type_t);
static const char CLOCK_SYNC_ARG[] = "clock-sync";

static int compute_argc(char *str) {
  int i;
  int cnt = 0;
  int white = 0;
  int seenword = 0;

  for (i = 0; i < strlen(str); i++) {
    if (str[i] == ' ') {
      white = 1;
    } else {
      if( i == strlen(str) -1 &&  white == 0 ) {
        cnt++;
      } else if (white == 1) {
        if( seenword == 1 ) {
          cnt++;
        }
      }
      white = 0;
      seenword = 1;
    }
  }
  return cnt;
}

void mpits_check_and_override_lib_env_params(int *argc, char ***argv) {
  char *env = getenv("MPITS_PARAMS");
  char **argvnew;

  if( env == NULL ) {
    env = strdup("--clock-sync=HCA3O --params=alg:hca3offset@skampi_offset@5@20");
  }

  if( env != NULL ) {
    char *token;
    //printf("env:%s\n", env);
    *argc = compute_argc(env) + 1;  // + 1 is for argv[0], which we'll copy
    //printf("argc: %d\n", *argc);

//    printf("(*argv)[0]=%s\n", (*argv)[0]);

    //  TODO: we should probably free the old argv
    argvnew = (char**)malloc(*argc * sizeof(char**));
    // copy old argv[0]
    argvnew[0] = (char*)"dummy";

//    printf("argvnew[0]=%s\n", argvnew[0]);

    token = strtok(env, " ");
    if( token != NULL ) {
//      printf("token: %s\n", token);
      argvnew[1] = token;
//      printf("argvnew[1]=%s\n", argvnew[1]);
      for(int i=2; i<*argc; i++) {
        token = strtok(NULL, " ");
        if( token != NULL ) {
//          printf("token: %s\n", token);
          argvnew[i] = token;
        }
      }
    }

    *argv = argvnew;
  }

}

int get_sync_type(const int n_types, const sync_type_t* type_list, const char* name) {
  int i;
  int type = -1;

  if (name != NULL) {
    for (i=0; i<n_types; i++) {
      if (strcmp(name, type_list[i].name) == 0) {
        type = type_list[i].type;
      }
    }
  }

  return type;
}

void parse_sync_options(int argc, char **argv, const char* argument_name,
                        sync_module_info_t* opts_p) {
  int c;
  struct option mpits_sync_module_long_options[] = {
          { argument_name, required_argument, 0, 1 },
          { 0, 0, 0, 0 }
  };
  const char mpits_sync_module_opts_str[] = "";


  opts_p->name = NULL;

  optind = 1;
  optopt = 0;
  opterr = 0; // ignore invalid options
  while (1) {

    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, mpits_sync_module_opts_str, mpits_sync_module_long_options,
                    &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
      case 1: /* synchronization module */
        opts_p->name = strdup(optarg);
        break;
      case '?':
        break;
    }
  }
  optind = 1; // reset optind to enable option re-parsing
  opterr = 1; // reset opterr
}

static int get_sync_module_index(const char* name) {
  int i;
  int sync_type;

  if (name == NULL) {
    return -1;
  }
  sync_type = get_sync_type(N_CLOCK_SYNC_TYPES, clock_sync_options, name);

  if (sync_type < 0) {
    return -1;
  }
  assert(sync_modules != NULL);

  for (i=0; i<N_CLOCK_SYNC_TYPES; i++) {
    if (sync_modules[i].clocksync == sync_type) {
      return i;
    }
  }
  return -1;
}

int MPITS_Init(MPI_Comm comm, mpits_clocksync_t *clocksync) {

  sync_module_info_t sync_module_info;
  int index;
  int argc;
  char **argv;

  MPITS_register_sync_modules();

  mpits_check_and_override_lib_env_params(&argc, &argv);
  mpits_parse_extra_key_value_options(get_global_param_store(), argc, argv);

  parse_sync_options(argc, argv, CLOCK_SYNC_ARG, &sync_module_info);
  if (sync_module_info.name == NULL) {
    sync_module_info.name = strdup("None");
  }
  index = get_sync_module_index(sync_module_info.name);

  if (index < 0) {
    char err_msg[160];
    snprintf(err_msg, sizeof(err_msg), "Unknown synchronization module \"--clock-sync=%s\"", sync_module_info.name);
    fprintf(stderr, "%s\n", err_msg);
    return -1;
  }

  *clocksync = sync_modules[index];
  clocksync->init_module(comm, argc, argv);
  clocksync->comm = comm;

  //cleanup_sync_options(&sync_module_info);

  return 0;
}

int MPITS_Clocksync_init(mpits_clocksync_t *clocksync) {
  clocksync->init_sync(clocksync->comm);
  return 0;
}

int MPITS_Clocksync_sync(mpits_clocksync_t *clocksync) {
  clocksync->sync_clocks(clocksync->comm);
  return 0;
}

int MPITS_Clocksync_resync(mpits_clocksync_t *clocksync) {
  return MPITS_Clocksync_sync(clocksync);
}

int MPITS_Clocksync_finalize(mpits_clocksync_t *clocksync) {
  clocksync->finalize_sync(clocksync->comm);
  return 0;
}

double MPITS_Clocksync_get_time(mpits_clocksync_t *clocksync) {
  return clocksync->get_global_time(MPITS_get_time());
}

int MPITS_Finalize() {
  MPITS_deregister_sync_modules();
  return 0;
}


void MPITS_register_sync_modules(void) {
  int sync_module_idx;
  sync_modules = (mpits_clocksync_t*)calloc(N_CLOCK_SYNC_TYPES, sizeof(mpits_clocksync_t));

  sync_module_idx = 0;

  register_no_clock_sync_module(&(sync_modules[sync_module_idx++]));
  register_skampi_module(&(sync_modules[sync_module_idx++]));
  register_jk_module(&(sync_modules[sync_module_idx++]));

  register_hca_module(&(sync_modules[sync_module_idx++]));
  register_hca2_module(&(sync_modules[sync_module_idx++]));
  register_hca3_module(&(sync_modules[sync_module_idx++]));
  register_hca3_offset_module(&(sync_modules[sync_module_idx++]));
#ifdef HAVE_HWLOC
  register_topo_aware_sync1_module(&(sync_modules[sync_module_idx++]));
#endif
  register_topo_aware_sync2_module(&(sync_modules[sync_module_idx++]));
}


void MPITS_deregister_sync_modules(void) {
  free(sync_modules);
}

