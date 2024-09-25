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

SEXP pt_cell_as_char_internal(
    note_t *cell, int offset, strings padding, strings empty, list sformat) {
  if (padding.size() != 1 || empty.size() != 1)
    Rf_error("Arguments must have length 1");
  if (sformat.size() != 4) Rf_error("'fmt' must have a length of 4.");
  
  note_t * cell2 = cell + offset;
  std::string notestr = (std::string)r_string(noteNames1[periodToNote(cell2->period)]);
  const char * empt = ((std::string)empty.at(0)).c_str();
  std::replace(notestr.begin(), notestr.end(), '-', empt[0]);
  
  auto sprf = package("base")["sprintf"];
  if (strings(sformat["note"]).size() < 1 || strings(sformat["padding"]).size() < 1 ||
      strings(sformat["instrument"]).size() < 1 || strings(sformat["effect"]).size() < 1)
    Rf_error("Mallformat pt2cell format");
  
  r_string fmt_note = strings(sformat["note"]).at(0);
  r_string fmt_inst = strings(sformat["instrument"]).at(0);
  r_string fmt_efft = strings(sformat["effect"]).at(0);
  r_string fmt_padd = strings(sformat["padding"]).at(0);
  
  if (!notestr.rfind((std::string)empty.at(0), 0) &&
      strings(sformat["note"]).size() > 1)
    fmt_note = strings(sformat["note"]).at(1);
  if ((int)cell2->command == 0 && (int)cell2->param == 0 &&
      strings(sformat["effect"]).size() > 1)
    fmt_efft = strings(sformat["effect"]).at(1);
  sexp fmt  = sprf("fmt"_nm = r_string("%s%s%s%s%s"),
                   fmt_note, fmt_padd, fmt_inst, fmt_padd, fmt_efft);
  sexp result = sprf(
    "fmt"_nm = strings(fmt),
    r_string(notestr),
    padding, writable::integers({(int)cell2->sample}),
    padding, writable::integers({(int)cell2->command}),
    writable::integers({(int)cell2->param}));
  return result;
}

[[cpp11::register]]
SEXP pt_cell_as_char_(
    SEXP mod, integers pattern, integers channel, integers row, strings padding,
    strings empty_char, list sformat) {
  note_t * cell = pt_cell_internal(mod, pattern, channel, row);
  
  return pt_cell_as_char_internal(cell, 0, padding, empty_char, sformat);
}

[[cpp11::register]]
SEXP pt_rawcell_as_char_(raws pattern, strings padding, strings empty_char, list sformat) {
  note_t * cell = (note_t *)RAW(as_sexp(pattern));
  int n_notes = (int)(pattern.size()/sizeof(note_t));
  writable::strings result((R_xlen_t)n_notes);
  for (int i = 0; i < n_notes; i++) {
    result[i] = strings(pt_cell_as_char_internal(cell, i, padding, empty_char, sformat)).at(0);
  }
  return result;
}