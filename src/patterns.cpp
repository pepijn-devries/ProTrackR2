#include <cpp11.hpp>
#include "get_mod.h"
#include "mod_header.h"
#include "pt2-clone/pt2_replayer_light.h"
using namespace cpp11;

[[cpp11::register]]
SEXP pattern_as_raw_(SEXP mod, integers pattern) {
  module_t *my_song = get_mod(mod);
  if (pattern.size() != 1)
    Rf_error("argument must have size 1");
  
  int npat = n_patterns_internal(mod);
  if (pattern.at(0) < 0 || pattern.at(0) >= npat)
    Rf_error("Index out of range!");
  
  uint8_t * pat = (uint8_t *)my_song->patterns[pattern.at(0)];
  writable::raws patdat((R_xlen_t)MOD_ROWS*PAULA_VOICES*sizeof(note_t));
  for (int i = 0; i < patdat.size(); i++) {
    patdat[i] = pat[i];
  }

  return patdat;
}