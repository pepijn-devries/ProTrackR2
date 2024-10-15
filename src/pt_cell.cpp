#include <cpp11.hpp>
#include <sstream>
#include <iomanip>
#include "get_mod.h"
#include "pt2-clone/pt2_structs.h"
#include "pt2-clone/pt2_replayer_light.h"
#include "pt2-clone/pt2_pattern_viewer.h"
#include "pt2-clone/pt2_tables.h"
#include "pt2-clone/pt2_module_saver.h"
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
  if (padding.size() < 1 || empty.size() < 1)
    Rf_error("Arguments must have at least one element");
  if (sformat.size() != 4) Rf_error("'fmt' must have a length of 4.");
  
  note_t * cell2 = cell + offset;
  std::string notestr = (std::string)r_string(noteNames1[periodToNote(cell2->period)]);
  std::string sempt = (std::string)empty.at(0);
  const char * empt = sempt.c_str();
  if (notestr.c_str()[0] == '-')
    std::replace(notestr.begin(), notestr.end(), '-', empt[0]);
  
  auto sprf = package("base")["sprintf"];
  auto gsub = package("base")["gsub"];
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
  
  sexp instr_str = sprf(r_string("%02i"), (int)cell2->sample);
  if ((int)cell2->sample == 0 && empty.size() > 1) {
    instr_str = gsub("0", empty.at(1), instr_str);
  }

  sexp efft_str = sprf(
    fmt_efft, writable::integers({(int)cell2->command}),
    writable::integers({(int)cell2->param})
  );
  
  if ((int)cell2->command == 0 && (int)cell2->param == 0 && empty.size() > 2) {
    efft_str = gsub("0", empty.at(2), efft_str, "perl"_nm = true);
  }
  
  sexp fmt = sprf("fmt"_nm = r_string("%s%s%s%s%s"),
                   fmt_note, fmt_padd, fmt_inst, fmt_padd, efft_str);
  int second_pad = 0;
  if (padding.size() > 1) second_pad = 1;
  sexp result = sprf(
    "fmt"_nm = strings(fmt),
    r_string(notestr),
    padding.at(0), instr_str,
    padding.at(second_pad));
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

[[cpp11::register]]
SEXP pt_decode_compact_cell(raws source) {
  int n_notes = (int)(source.size()/4);
  writable::raws celldat((R_xlen_t)(n_notes * sizeof(note_t)));
  uint8_t * src = (uint8_t *)RAW(as_sexp(source));
  note_t * dest = (note_t *)RAW(as_sexp(celldat));
  
  for(int i = 0; i < n_notes; i++, dest++, src += 4){
    dest->period = ((src[0] & 0x0F) << 8) | src[1];
    dest->sample = ((src[0] & 0xF0) | (src[2] >> 4)) & 31;
    dest->command = src[2] & 0x0F;
    dest->param = src[3];
  }

  return celldat;
}

[[cpp11::register]]
SEXP pt_encode_compact_cell(raws source){
  int n_notes = (int)(source.size()/sizeof(note_t));
  note_t * src  = (note_t *)RAW(as_sexp(source));
  writable::raws celldat((R_xlen_t)(n_notes * 4));
  uint8_t * dest = (uint8_t *)RAW(as_sexp(celldat));
  
  cellCompacter(src, dest, n_notes);
  
  return celldat;
}

void pt_encode_compact_cell_internal(note_t * source, uint8_t * dest, uint32_t n_notes) {
  cellCompacter(source, dest, n_notes);
  return;
}