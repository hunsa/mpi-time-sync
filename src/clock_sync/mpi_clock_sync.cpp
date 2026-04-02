
#include <getopt.h>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cctype>
#include <string>
#include <vector>

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
static const char DEFAULT_MPITS_PARAMS[] = "--clock-sync=HCA3O --params=options:skampi_offset@5@20";

typedef struct {
  int argc;
  char **argv;
} mpits_owned_argv_t;

static mpits_owned_argv_t g_mpits_override_args = {0, nullptr};

static void cleanup_override_lib_env_params() {
  if (g_mpits_override_args.argv == nullptr) {
    return;
  }

  for (int i = 0; i < g_mpits_override_args.argc; ++i) {
    free(g_mpits_override_args.argv[i]);
  }
  free(g_mpits_override_args.argv);
  g_mpits_override_args.argc = 0;
  g_mpits_override_args.argv = nullptr;
}

static std::vector<std::string> split_args(const char *str) {
  std::vector<std::string> tokens;
  if (str == nullptr) {
    return tokens;
  }

  const char *cursor = str;
  while (*cursor != '\0') {
    while (*cursor != '\0' && std::isspace(static_cast<unsigned char>(*cursor)) != 0) {
      ++cursor;
    }
    if (*cursor == '\0') {
      break;
    }

    const char *start = cursor;
    while (*cursor != '\0' && std::isspace(static_cast<unsigned char>(*cursor)) == 0) {
      ++cursor;
    }
    tokens.emplace_back(start, cursor - start);
  }

  return tokens;
}

static void fail_on_invalid_mpits_params(const char *params) {
  fprintf(stderr, "ERROR: failed to parse MPITS_PARAMS=\"%s\"\n", params != nullptr ? params : "(null)");
  std::abort();
}

void cleanup_sync_options(sync_module_info_t* opts_p) {
  if (opts_p != nullptr && opts_p->name != nullptr) {
    free(opts_p->name);
    opts_p->name = nullptr;
  }
}

void mpits_check_and_override_lib_env_params(int *argc, char ***argv) {
  const char *env = getenv("MPITS_PARAMS");
  if (env == nullptr) {
    env = DEFAULT_MPITS_PARAMS;
  }

  cleanup_override_lib_env_params();

  std::vector<std::string> tokens = split_args(env);
  g_mpits_override_args.argc = static_cast<int>(tokens.size()) + 1;
  g_mpits_override_args.argv = static_cast<char**>(calloc(g_mpits_override_args.argc + 1, sizeof(char*)));
  if (g_mpits_override_args.argv == nullptr) {
    fail_on_invalid_mpits_params(env);
  }

  g_mpits_override_args.argv[0] = strdup("mpits");
  if (g_mpits_override_args.argv[0] == nullptr) {
    cleanup_override_lib_env_params();
    fail_on_invalid_mpits_params(env);
  }

  for (size_t i = 0; i < tokens.size(); ++i) {
    g_mpits_override_args.argv[i + 1] = strdup(tokens[i].c_str());
    if (g_mpits_override_args.argv[i + 1] == nullptr) {
      cleanup_override_lib_env_params();
      fail_on_invalid_mpits_params(env);
    }
  }

  if (tokens.empty()) {
    fprintf(stderr, "WARNING: MPITS_PARAMS is empty; using library defaults only\n");
  }

  *argc = g_mpits_override_args.argc;
  *argv = g_mpits_override_args.argv;
}

char* get_name_from_sync_type(const int n_types, const sync_type_t* type_list, int type) {
  for (int i = 0; i < n_types; ++i) {
    if (type_list[i].type == type) {
      return strdup(type_list[i].name);
    }
  }
  return nullptr;
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
  mpits_parse_extra_key_value_options(mpits_get_global_param_store(), argc, argv);

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

  cleanup_sync_options(&sync_module_info);

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
  cleanup_override_lib_env_params();
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
