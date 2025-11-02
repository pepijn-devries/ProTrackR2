#include <cpp11.hpp>
#include "get_mod.h"
#include "pt2-clone.h"
using namespace cpp11;

[[cpp11::register]]
strings mod_name_(SEXP mod) {
  module_t *my_song = get_mod(mod);
  writable::strings result(r_string((my_song->header).name));
  return result;
}

[[cpp11::register]]
integers mod_length_(SEXP mod) {
  module_t *my_song = get_mod(mod);
  writable::integers result({(my_song->header).songLength});
  return result;
}

[[cpp11::register]]
SEXP set_mod_length_(SEXP mod, int modlen) {
  module_t *my_song = get_mod(mod);
  if (modlen < 1 || modlen > N_PATT_SEQ)
    stop("Specified length out of range");
  my_song->header.songLength = modlen;
  return mod;
}

[[cpp11::register]]
integers mod_pattab_(SEXP mod) {
  module_t *my_song = get_mod(mod);
  writable::integers result((R_xlen_t)N_PATT_SEQ);
  uint16_t *pattab = (my_song->header).patternTable;
  for (int i = 0; i < N_PATT_SEQ; i++) {
    result[i] = pattab[i];
  }
  return result;
}

[[cpp11::register]]
SEXP update_pattern_sequence_(SEXP mod, integers ptn_sequence) {
  if (ptn_sequence.size() != N_PATT_SEQ)
    stop("Pattern sequence table should have a length of %i integers", N_PATT_SEQ);
  module_t *my_song = get_mod(mod);
  uint16_t *pattab = (my_song->header).patternTable;
  for (int i = 0; i < N_PATT_SEQ; i++) {
    int replacement = ptn_sequence[i];
    if (replacement < 0 || replacement >= MAX_PATTERNS)
      stop("Replacement value is out of range [0-%i]!", MAX_PATTERNS - 1);
    pattab[i] = (uint16_t)replacement;
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

void set_mod_name_internal(module_t * my_song, std::string name) {
  memset(&my_song->header.name, 0, 20);
  int len = name.size();
  len = len > 20 ? 20 : len;
  memcpy(&my_song->header.name, name.c_str(), len);
  return;
}

[[cpp11::register]]
SEXP set_mod_name_(SEXP mod, strings name) {
  if (name.size() != 1) stop("Renaming a mod requires only on string");
  module_t *my_song = get_mod(mod);
  set_mod_name_internal(my_song, (std::string)(name[0]));
  return mod;
}