#include <cpp11.hpp>
#include "get_mod.h"
#include "mod_header.h"
#include "pt_cell.h"
#include "pt2-clone.h"
using namespace cpp11;

[[cpp11::register]]
SEXP cells_as_raw_(SEXP mod, int pattern, bool compact, bool as_pattern,
                   int row, int channel) {
  module_t *my_song = get_mod(mod);

  int npat = n_patterns_internal(mod);
  if (pattern < 0 || pattern >= npat)
    stop("Index out of range!");
  
  note_t * pat = my_song->patterns[pattern];
  int result_size;
  int cell_count = MOD_ROWS*PAULA_VOICES;
  int offset = 0;
  if (!as_pattern) {
    if (channel < 0 || channel > PAULA_VOICES ||
        row < 0 || row > MOD_ROWS)
      stop("Index out of range!");
    offset = row*PAULA_VOICES + channel;
    cell_count = 1;
  }
  pat += offset;
  if (compact) result_size = cell_count*4; else result_size = cell_count*sizeof(note_t);
  writable::raws patdat((R_xlen_t)result_size);
  uint8_t * patdest = (uint8_t *)RAW(as_sexp(patdat));
  if (compact) {
    pt_encode_compact_cell_internal(pat, patdest, cell_count);
  } else {
    memcpy(patdest, (uint8_t *)pat, patdat.size());
  }

  patdat.attr("class") = "pt2pat";
  patdat.attr("compact_notation") = compact;
  return patdat;
}

[[cpp11::register]]
SEXP set_new_pattern_(SEXP mod, int pattern_idx, raws data_new) {
  
  if (data_new.size() != sizeof(note_t)*PAULA_VOICES*MOD_ROWS)
    stop("Received incorrect amount of data for pattern");
  
  uint32_t idx = pattern_idx;
  if (idx < 0 || idx > MAX_PATTERNS)
    stop("Pattern index out of range");
  
  module_t *my_song = get_mod(mod);
  
  uint8_t * patsrc = (uint8_t *)RAW(as_sexp(data_new));
  note_t * patdest = my_song->patterns[idx];
  memcpy((uint8_t *)patdest, patsrc, data_new.size());
  
  return R_NilValue;
}
