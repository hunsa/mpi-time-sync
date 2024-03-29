//
// Created by Sascha on 9/5/23.
//

#include "mpits_dict.h"

#include <map>
#include <iostream>
#include <cstdlib>

struct MapWrapper {
  std::map<std::string, std::string> data;
};

mpits_map mpits_init_dictionary() {
  return new MapWrapper();
}

void mpits_cleanup_dictionary(mpits_map map) {
  MapWrapper* mw = (MapWrapper*)map;
  delete mw;
}

int mpits_add_element_to_dict(mpits_map map, const char* key, const char* val) {
  MapWrapper* mw = (MapWrapper*)map;
  std::string key_s(key);
  std::string val_s(val);
  mw->data[key_s] = val_s;
  return 0;
}

int mpits_get_value_from_dict(const mpits_map map, const char* key, char** value) {
  MapWrapper* mw = (MapWrapper*)map;
  int not_found = 1;

  if( mw->data.find(key) != mw->data.end() ) {
    not_found = 0;
    *value = (char*)mw->data[key].c_str();
  } else {
    *value = NULL;
  }
  return not_found;
}

int mpits_remove_element_from_dict(mpits_map map, const char* key) {
  int not_okay = 1;

  MapWrapper* mw = (MapWrapper*)map;
  if( mw->data.find(key) != mw->data.end() ) {
    mw->data.erase(key);
    not_okay = 0;
  }
  return not_okay;
}

int mpits_get_keys_from_dict(const mpits_map map, char ***keys, int *nkeys) {
  int not_okay = 1;
  MapWrapper* mw = (MapWrapper*)map;
  if( mw->data.size() > 0 ) {
    *keys = (char**) calloc(*nkeys, sizeof(char*));
    if (*keys == NULL) {
      return not_okay;
    }

    int key_cnt = 0;
    for (const auto& pair : mw->data) {
      (*keys)[key_cnt++] = (char*)pair.first.c_str();
    }
    not_okay = 0;
  }
  return not_okay;
}

int mpits_dict_is_empty(const mpits_map map) {
  MapWrapper* mw = (MapWrapper*)map;
  return (mw->data.size()==0);
}

int mpits_dict_get_nkeys(const mpits_map map) {
  MapWrapper* mw = (MapWrapper*)map;
  return mw->data.size();
}

int mpits_dict_has_key(const mpits_map map, const char *key) {
  MapWrapper* mw = (MapWrapper*)map;
  return (mw->data.find(key) != mw->data.end());
}

void mpits_print_dictionary(const mpits_map map, FILE* f) {
  MapWrapper* mw = (MapWrapper*)map;
  for (const auto& pair : mw->data) {
    std::cout << "key: " << pair.first << " value: " << pair.second << std::endl;
  }


}


