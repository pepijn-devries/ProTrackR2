#include <cpp11.hpp>
#include <sstream>
#include <iomanip>
#include "get_mod.h"
#include "pt2-clone/pt2_replayer_light.h"
#include "pt2-clone/pt2_pattern_viewer.h"
#include "pt2-clone/pt2_tables.h"
using namespace cpp11;

note_t * pt_cell_internal(SEXP mod, integers pattern, integers channel, integers row) {
  module_t *my_song = get_mod(mod);
  if (pattern.size() != 1 || channel.size() != 1 || row.size() != 1)
    Rf_error("This function only accepts a single index.");
  if (channel.at(0) < 0 || channel.at(0) >= PAULA_VOICES)
    Rf_error("Channel index out of range");
  if (row.at(0) < 0 || row.at(0) >= MOD_ROWS)
    Rf_error("Row index out of range");
  note_t *pat = my_song->patterns[(int)pattern.at(0)];
  note_t *cell = & pat[channel.at(0) + row.at(0)*PAULA_VOICES];
  return cell;
}

[[cpp11::register]]
SEXP pt_cell_(SEXP mod, integers pattern, integers channel, integers row) {
  note_t * cell = pt_cell_internal(mod, pattern, channel, row);

  writable::list result({
    "param"_nm     = (int)cell->param,
      "sample"_nm     = (int)cell->sample,
      "command"_nm    = (int)cell->command,
      "period"_nm     = (int)cell->period,
      "note"_nm       = (int)periodToNote(cell->period),
      "note_nm"_nm    = r_string(noteNames1[periodToNote(cell->period)])
  });
  return result;
}

SEXP pt_cell_as_char_internal(note_t *cell, int offset) {
  std::stringstream ss;
  note_t * cell2 = cell + offset;
  ss <<
    (std::string)(noteNames1[periodToNote(cell2->period)]) <<
      std::setw(2) << std::setfill('0') << (int)cell2->sample <<
        std::setw(1) << std::uppercase << std::setfill('0') << std::hex << (int)cell2->command <<
          std::setw(2) << std::uppercase << std::setfill('0') << std::hex << (int)cell2->param;
  writable::strings result(r_string(ss.str()));
  return result;
}

[[cpp11::register]]
SEXP pt_cell_as_char_(SEXP mod, integers pattern, integers channel, integers row) {
  note_t * cell = pt_cell_internal(mod, pattern, channel, row);
  return pt_cell_as_char_internal(cell, 0);
}

[[cpp11::register]]
SEXP pt_rawcell_as_char_(raws pattern) {
  note_t * cell = (note_t *)RAW(as_sexp(pattern));
  int n_notes = (int)(pattern.size()/sizeof(note_t));
  writable::strings result((R_xlen_t)n_notes);
  for (int i = 0; i < n_notes; i++) {
    result[i] = strings(pt_cell_as_char_internal(cell, i)).at(0);
  }
  return result;
}