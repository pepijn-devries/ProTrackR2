#include <cpp11.hpp>
#include "get_mod.h"
#include "mod_header.h"
#include "pt_cell.h"
#include "pt2-clone/pt2_replayer_light.h"
using namespace cpp11;

[[cpp11::register]]
SEXP pattern_as_raw_(SEXP mod, integers pattern, logicals compact) {
  module_t *my_song = get_mod(mod);
  if (pattern.size() != 1 || compact.size() != 1)
    Rf_error("argument must have size 1");
  
  int npat = n_patterns_internal(mod);
  if (pattern.at(0) < 0 || pattern.at(0) >= npat)
    Rf_error("Index out of range!");
  
  note_t * pat = my_song->patterns[pattern.at(0)];
  
  int result_size = MOD_ROWS*PAULA_VOICES;
  if (compact.at(0)) result_size *= 4; else result_size *= sizeof(note_t);
  writable::raws patdat((R_xlen_t)result_size);
  uint8_t * patdest = (uint8_t *)RAW(as_sexp(patdat));
  if (compact.at(0)) {
    pt_encode_compact_cell_internal(pat, patdest, MOD_ROWS*PAULA_VOICES);
  } else {
    memcpy(patdest, (uint8_t *)pat, patdat.size());
  }

  sexp result = as_sexp(patdat);
  result.attr("class") = "pt2pat";
  result.attr("compact_notation") = compact;
  return result;
}

[[cpp11::register]]
SEXP set_new_pattern_(SEXP mod, integers pattern_idx, raws data_new) {
  
  if (data_new.size() != sizeof(note_t)*PAULA_VOICES*MOD_ROWS)
    Rf_error("Received incorrect amount of data for pattern");
  
  if (pattern_idx.size() != 1)
    Rf_error("Can only handle one index per call");

  uint32_t idx = pattern_idx.at(0);
  if (idx < 0 || idx > MAX_PATTERNS)
    Rf_error("Pattern index out of range");
  
  module_t *my_song = get_mod(mod);
  
  uint8_t * patsrc = (uint8_t *)RAW(as_sexp(data_new));
  note_t * patdest = my_song->patterns[idx];
  memcpy((uint8_t *)patdest, patsrc, data_new.size());
  
  return R_NilValue;
}