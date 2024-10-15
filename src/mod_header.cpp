#include <cpp11.hpp>
#include "get_mod.h"
#include "pt2-clone/pt2_replayer_light.h"
using namespace cpp11;

[[cpp11::register]]
SEXP mod_name_(SEXP mod) {
  module_t *my_song = get_mod(mod);
  writable::strings result(r_string((my_song->header).name));
  return result;
}

[[cpp11::register]]
SEXP mod_length_(SEXP mod) {
  module_t *my_song = get_mod(mod);
  writable::integers result({(my_song->header).songLength});
  return result;
}

[[cpp11::register]]
SEXP set_mod_length_(SEXP mod, integers modlen) {
  if (modlen.size() != 1) Rf_error("Length should hold only one value");
  module_t *my_song = get_mod(mod);
  uint32_t modlen2 = modlen.at(0);
  if (modlen2 < 1 || modlen2 > N_PATT_SEQ)
    Rf_error("Specified length out of range");
  my_song->header.songLength = modlen2;
  return mod;
}

[[cpp11::register]]
SEXP mod_pattab_(SEXP mod) {
  module_t *my_song = get_mod(mod);
  writable::integers result;
  result.reserve((R_xlen_t)N_PATT_SEQ);
  uint16_t *pattab = (my_song->header).patternTable;
  for (int i = 0; i < N_PATT_SEQ; i++) {
    result.push_back(pattab[i]);
  }
  return result;
}

[[cpp11::register]]
SEXP update_pattern_sequence_(SEXP mod, integers ptn_sequence) {
  if (ptn_sequence.size() != N_PATT_SEQ)
    Rf_error("Pattern sequence table should have a length of 128 integers");
  module_t *my_song = get_mod(mod);
  uint16_t *pattab = (my_song->header).patternTable;
  for (int i = 0; i < N_PATT_SEQ; i++) {
    pattab[i] = (uint16_t)ptn_sequence.at(i);
  }
  
  return mod;
}

int n_patterns_internal(SEXP mod) {
  module_t *my_song = get_mod(mod);
  uint16_t *pattab = (my_song->header).patternTable;
  int result = 0;
  for (int i = 0; i < N_PATT_SEQ; i++) {
    result = (pattab[i] > result) ? pattab[i] : result;
  }
  return result + 1;
}

void set_mod_name_internal(module_t * my_song, strings name) {
  if (name.size() != 1) Rf_error("Arguments should have length 1");
  memset(&my_song->header.name, 0, 20);
  int len = name.at(0).size();
  len = len > 20 ? 20 : len;
  memcpy(&my_song->header.name, ((std::string)name.at(0)).c_str(), len);
  return;
}