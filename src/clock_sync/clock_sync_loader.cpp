
#include <string>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "clock_sync_loader.hpp"

#include "time_provider/clocks/GlobalClockOffset.hpp"
#include "clock_sync/clock_offset_algs/PingpongClockOffsetAlg.hpp"
#include "clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.hpp"
#include "clock_sync/sync_algorithms/HierarchicalClockSync.hpp"
#include "clock_sync/sync_algorithms/offset/SKaMPIClockSync.hpp"
#include "clock_sync/sync_algorithms/JKClockSync.hpp"
#include "clock_sync/sync_algorithms/HCAClockSync.hpp"
#include "clock_sync/sync_algorithms/HCA2ClockSync.hpp"
#include "clock_sync/sync_algorithms/HCA3ClockSync.hpp"
#include "clock_sync/sync_algorithms/offset/HCA3OffsetClockSync.hpp"
#include "clock_sync/sync_algorithms/ClockPropagationSync.hpp"
#include "helpers/dict/mpits_dict.h"
#include "helpers/dict/cli_param_parser.h"
//#include "clock_sync/clock_sync_common.h"
//#include "clock_sync/clock_sync_lib.h"
//#include "common/sync_module_helpers.h"


//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

static std::vector<std::string> str_split(const char *str, char c);
static ClockOffsetAlg* instantiate_clock_offset_alg(std::vector<std::string> &tokens);


static std::vector<std::string> str_split(const char *str, char c = ' ') {
  std::vector<std::string> result;

  do {
    const char *begin = str;

    while (*str != c && *str) {
      str++;
    }

    result.push_back(std::string(begin, str));
  } while (0 != *str++);

  return result;
}


static ClockOffsetAlg* instantiate_clock_offset_alg(std::vector<std::string> &tokens) {
  ClockOffsetAlg *offset_alg = NULL;

  if( tokens[0] == "skampi_offset" ) {
    // skampi_offset@min_nb_ping_pongs@nb_ping_pongs
    if( tokens.size() != 3 ) {
      ZF_LOGE("number of parameters to ClockOffsetAlg wrong (!=3)\nskampi_offset@min_nb_ping_pongs@nb_ping_pongs");
    } else {
      int min_nb_ping_pongs = stoi(tokens[1]);
      int nb_ping_pongs     = stoi(tokens[2]);
      ZF_LOGV("skampi offset with %d,%d ping-pongs", min_nb_ping_pongs, nb_ping_pongs);
      offset_alg = new SKaMPIClockOffsetAlg(min_nb_ping_pongs, nb_ping_pongs);
    }
  } else if( tokens[0] == "pingpong_offset" ) {
    // pingpong_offset,nexchanges_rtt,nexchanges
    if( tokens.size() != 3 ) {
      ZF_LOGE("number of parameters to ClockOffsetAlg wrong (!=3)");
    } else {
      int nexchanges_rtt = stoi(tokens[1]);
      int nexchanges     = stoi(tokens[2]);
      ZF_LOGV("ping-pong offset with %d,%d exchanges", nexchanges_rtt, nexchanges);
      offset_alg = new PingpongClockOffsetAlg(nexchanges_rtt, nexchanges);
    }
  } else {
    ZF_LOGE("unknown offset algorithm '%s'", tokens[0].c_str());
  }

  return offset_alg;
}

BaseClockSync* ClockSyncLoader::instantiate_clock_sync(MPI_Comm comm, const char *param_name) {
  BaseClockSync* ret_sync = nullptr;
  char *alg_str;
  int rank;

  MPI_Comm_rank(comm, &rank);

  ZF_LOGV("instantiate clock for '%s'", param_name);

  //mpits_print_dictionary(get_global_param_store(), stdout);

  if (mpits_dict_has_key(mpits_get_global_param_store(), param_name) == 1) {
      mpits_get_value_from_dict(mpits_get_global_param_store(), param_name, &alg_str);

      std::vector<std::string> tokens = str_split(alg_str, '@');

      if (tokens.size() <= 0) {
          ZF_LOGE("value of %s incompatible", param_name);
      } else {
          std::string sync_alg = tokens[0];

          tokens.erase(tokens.begin());

          if (sync_alg == "HCA2" || sync_alg == "HCA3") {

              // hca2@recompute_offsets@fitpoints@offsetalg_format
              // hca3@recompute_offsets@fitpoints@offsetalg_format
              if (tokens.size() >= 2) {

                  bool recompute_offset = stoi(tokens[0]) == 1;
                  tokens.erase(tokens.begin());

                  // get: number of fitpoints
                  int n_fitpoints = stoi(tokens[0]);
                  tokens.erase(tokens.begin());

                  ClockOffsetAlg *offset_alg = instantiate_clock_offset_alg(tokens);
                  if (offset_alg == nullptr) {
                      ZF_LOGE("cannot instantiate clock offset algorithm");
                  } else {
                      if (sync_alg == "HCA2") {
                          ZF_LOGV("HCA2 clock sync with %d fitpoints, recompute %d", n_fitpoints, recompute_offset);
                          ret_sync = new HCA2ClockSync(offset_alg, n_fitpoints, recompute_offset);
                      } else if (sync_alg == "HCA3") {
                          ZF_LOGV("HCA3 clock sync with %d fitpoints, recompute %d", n_fitpoints, recompute_offset);
                          ret_sync = new HCA3ClockSync(offset_alg, n_fitpoints, recompute_offset);
                      }
                  }
              } else {
                  ZF_LOGE("format error sync alg '%s'", sync_alg.c_str());
              }

          } else if (sync_alg == "HCA" || sync_alg == "JK") {

              // hca@fitpoints@offsetalg_format
              //  jk@fitpoints@offsetalg_format
              if (!tokens.empty()) {
                  // get: number of fitpoints
                  int n_fitpoints = stoi(tokens[0]);
                  tokens.erase(tokens.begin());

                  ClockOffsetAlg *offset_alg = instantiate_clock_offset_alg(tokens);
                  if (offset_alg == nullptr) {
                      ZF_LOGE("cannot instantiate clock offset algorithm");
                  } else {
                      if (sync_alg == "HCA") {
                          ZF_LOGV("HCA clock sync with %d fitpoints", n_fitpoints);
                          ret_sync = new HCAClockSync(offset_alg, n_fitpoints);
                      } else if (sync_alg == "JK") {
                          ZF_LOGV("JK clock sync with %d fitpoints", n_fitpoints);
                          ret_sync = new JKClockSync(offset_alg, n_fitpoints);
                      }
                  }
              } else {
                  ZF_LOGE("format error sync alg '%s'", sync_alg.c_str());
              }

          } else if (sync_alg == "SKaMPI") {
              if (!tokens.empty()) {
                  ClockOffsetAlg *offset_alg = instantiate_clock_offset_alg(tokens);
                  if (offset_alg != nullptr) {
                      ret_sync = new SKaMPIClockSync(offset_alg);
                  } else {
                      ZF_LOGE("problem with format of SKaMPI clock offset alg");
                  }
              } else {
                  ZF_LOGE("format error sync alg '%s'", sync_alg.c_str());
              }
          } else if (sync_alg == "HCA3O") {
            if (!tokens.empty()) {
              ClockOffsetAlg *offset_alg = instantiate_clock_offset_alg(tokens);
              if (offset_alg != nullptr) {
                ret_sync = new HCA3OffsetClockSync(offset_alg);
              } else {
                ZF_LOGE("problem with format of HCA3O clock offset alg\nuse HCA3O@offsetalg_format");
              }
            } else {
              ZF_LOGE("problem with format of HCA3O clock offset alg: sync alg '%s'\nuse HCA3O@offsetalg_format", sync_alg.c_str());
            }
          } else if (sync_alg == "prop") {
              // topoalg2:prop@0   offset only
              // topoalg2:prop@1   linear model
              if (!tokens.empty()) {
                int prop_type = stoi(tokens[0]);
                if( prop_type == 0 ) {
                  ret_sync = new ClockPropagationSync(ClockPropagationSync::ClockType::CLOCK_OFFSET);
                } else if( prop_type == 1) {
                  ret_sync = new ClockPropagationSync(ClockPropagationSync::ClockType::CLOCK_LM);
                } else {
                  ZF_LOGE("problem with format of prop clock sync: prop@0 or prop@1");
                  exit(1);
                }
              } else {
                ZF_LOGE("problem with format of prop clock sync: prop@0 or prop@1");
              }

          } else {
              ZF_LOGE("unknown clock sync alg '%s'", sync_alg.c_str());
          }
      }

  } else {
      if (rank == 0) {
          ZF_LOGE("parameter '%s' not found, using default", param_name);
      }
  }

  return ret_sync;
}


