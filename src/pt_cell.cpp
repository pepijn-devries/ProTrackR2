#include <cpp11.hpp>
#include <cpp11/matrix.hpp>
#include <sstream>
#include <iomanip>
#include "get_mod.h"
#include "pt2-clone/pt2_structs.h"
#include "pt2-clone/pt2_replayer_light.h"
#include "pt2-clone/pt2_pattern_viewer.h"
#include "pt2-clone/pt2_tables.h"
#include "pt2-clone/pt2_module_saver.h"
using namespace cpp11;

note_t * pt_cell_internal(SEXP mod, int pattern, int channel, int row) {
  module_t *my_song = get_mod(mod);
  if (channel < 0 || channel >= PAULA_VOICES)
    Rf_error("Channel index out of range");
  if (row < 0 || row >= MOD_ROWS)
    Rf_error("Row index out of range");
  note_t *pat = my_song->patterns[pattern];
  note_t *cell = & pat[channel + row * PAULA_VOICES];
  return cell;
}

[[cpp11::register]]
SEXP pt_cell_(SEXP mod, int pattern, int channel, int row) {
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

int cell_check_input(
    list mod, integers pattern, integers channel, integers row) {
  int input_size = pattern.size();
  if (input_size < 1 ||
      channel.size() != input_size ||
      row.size() != input_size ||
      LENGTH(mod) != input_size)
    Rf_error("All input should have the same size");
  return input_size;
}

[[cpp11::register]]
integers note_to_period_(strings note, std::string empty_char, int finetune) {
  if (empty_char.length() != 1)
    Rf_error("`empty_char` should consist of 1 character.");
  if (finetune < -8 || finetune > 7)
    Rf_error("`finetune` is out of range [-8, +7].");
  if (finetune < 0) {
    finetune += 16;
  }
  writable::integers result((R_xlen_t)note.size());
  char empty = empty_char.c_str()[0];
  
  for (int i = 0; i < note.size(); i++) {
    std:: string s = (std::string)note.at(i);
    for (auto & c: s) {
      c = toupper(c);
      if (c == empty) c = '-';
    }
    result.at(i) = NA_INTEGER;
    for (int j = 2; j < 38; j++) {
      if (s.compare(noteNames1[j]) == 0) {
        result.at(i) = periodTable[j - 2 + finetune * 37];
        break;
      }
    }
  }
  return result;
}

[[cpp11::register]]
strings pt_note_string_(
    list mod, integers pattern, integers channel, integers row) {
  int input_size = cell_check_input(mod, pattern, channel, row);
  
  writable::strings result((R_xlen_t)pattern.size());
  
  for (int i = 0; i < input_size; i++) {
    note_t * note = pt_cell_internal(mod.at(i), pattern.at(i), channel.at(i), row.at(i));
    std::string notestr(noteNames1[periodToNote(note->period)]);
    result.at(i) = notestr;
  }
  
  return result;
}

[[cpp11::register]]
std::string pt_note_string_raw_(raws data) {
  note_t * note = (note_t *)(RAW(as_sexp(data)));
  std::string notestr(noteNames1[periodToNote(note->period)]);
  return notestr;
}

[[cpp11::register]]
SEXP pt_set_note_(
    list mod, integers pattern, integers channel, integers row, strings replacement,
    bool warn) {
  int input_size = cell_check_input(mod, pattern, channel, row);
  integers replacement_int = note_to_period_(replacement, std::string("-"), 0);
  
  int j = 0;
  bool all_used = false;
  bool recycled = false;
  for (int i = 0; i < input_size; i++) {
    if (j + 1 > replacement.size()) {
      j = 0;
      recycled = true;
    }
    note_t * note = pt_cell_internal(mod.at(i), pattern.at(i), channel.at(i), row.at(i));
    int rep = replacement_int.at(j);
    if (rep == NA_INTEGER) rep = 0;
    note->period = rep;
    j++;
    if (j + 1 >= replacement.size()) all_used = true;
  }
  if (warn) {
    if (!all_used) Rf_warning("Not all replacement values are used");
    if (recycled) Rf_warning("Replacement values are recycled");
  }
  return R_NilValue;
}

[[cpp11::register]]
integers pt_instr_(
    list mod, integers pattern, integers channel, integers row) {
  int input_size = cell_check_input(mod, pattern, channel, row);
  
  writable::integers result((R_xlen_t)pattern.size());
  
  for (int i = 0; i < input_size; i++) {
    note_t * note = pt_cell_internal(mod.at(i), pattern.at(i), channel.at(i), row.at(i));
    result.at(i) = note->sample;
  }
  
  return result;
}

[[cpp11::register]]
SEXP pt_set_instr_(
    list mod, integers pattern, integers channel, integers row, integers replacement,
    bool warn) {
  int input_size = cell_check_input(mod, pattern, channel, row);
  
  int j = 0;
  bool all_used = false;
  bool recycled = false;
  for (int i = 0; i < input_size; i++) {
    if (j + 1 > replacement.size()) {
      j = 0;
      recycled = true;
    }
    note_t * note = pt_cell_internal(mod.at(i), pattern.at(i), channel.at(i), row.at(i));
    note->sample  = replacement.at(j);
    j++;
    if (j + 1 >= replacement.size()) all_used = true;
  }
  if (warn) {
    if (!all_used) Rf_warning("Not all replacement values are used");
    if (recycled) Rf_warning("Replacement values are recycled");
  }
  return R_NilValue;
}

[[cpp11::register]]
raws pt_eff_command_(
    list mod, integers pattern, integers channel, integers row) {
  int input_size = cell_check_input(mod, pattern, channel, row);
  
  writable::raws result((R_xlen_t)2*pattern.size());
  
  for (int i = 0; i < input_size; i++) {
    note_t * note = pt_cell_internal(mod.at(i), pattern.at(i), channel.at(i), row.at(i));
    result.at(i*2)     = note->command;
    result.at(i*2 + 1) = note->param;
  }
  result.attr("class") = "pt2command";
  return result;
}

[[cpp11::register]]
SEXP pt_set_eff_command_(
    list mod, integers pattern, integers channel, integers row, raws replacement,
    bool warn) {
  int input_size = cell_check_input(mod, pattern, channel, row);
  
  if (replacement.size() % 2 != 0)
    Rf_error("Replacement value should consist of a multitude of 2 raws.");
  int j = 0;
  bool all_used = false;
  bool recycled = false;
  for (int i = 0; i < input_size; i++) {
    if (j*2 + 1 > replacement.size()) {
      j = 0;
      recycled = true;
    }
    note_t * note = pt_cell_internal(mod.at(i), pattern.at(i), channel.at(i), row.at(i));
    note->command = replacement.at(j*2);
    note->param   = replacement.at(j*2 + 1);
    j++;
    if (j*2 + 1 >= replacement.size()) all_used = true;
  }
  if (warn) {
    if (!all_used) Rf_warning("Not all replacement values are used");
    if (recycled) Rf_warning("Replacement values are recycled");
  }
  return R_NilValue;
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
  r_string fmt_inst = "%s";
  r_string fmt_inst_temp = strings(sformat["instrument"]).at(0);
  r_string fmt_efft = strings(sformat["effect"]).at(0);
  r_string fmt_padd = strings(sformat["padding"]).at(0);
  
  if (!notestr.rfind((std::string)empty.at(0), 0) &&
      strings(sformat["note"]).size() > 1)
    fmt_note = strings(sformat["note"]).at(1);
  if ((int)cell2->command == 0 && (int)cell2->param == 0 &&
      strings(sformat["effect"]).size() > 1)
    fmt_efft = strings(sformat["effect"]).at(1);
  
  sexp instr_str;
  if ((int)cell2->sample == 0 && empty.size() > 1) {
    instr_str = writable::strings(r_string((std::string)empty.at(1) + (std::string)empty.at(1)));
  } else {
    instr_str = sprf(fmt_inst_temp, (int)cell2->sample);
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
    SEXP mod, int pattern, int channel, int row, strings padding,
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

[[cpp11::register]]
SEXP celllist_to_raw_(list celllist, bool compact) {
  int size_out = celllist.size();
  if (compact) size_out *= 4;
  else size_out *= sizeof(note_t);
  
  writable::raws data_out((R_xlen_t)size_out);
  uint8_t * celldst = (uint8_t *)RAW(as_sexp(data_out));
  
  for (int i = 0; i < celllist.size(); i++) {
    SEXP element = celllist.at(i);
    if (!Rf_inherits(element, "pt2cell")) Rf_error("Invalid pt2celllist.");
    if (TYPEOF(element) == RAWSXP) {
      Rf_error("Raw to raw is not implemented in C++. Contact package maintainer if you see this error");
    } else {

      list cell = list(element);
      SEXP mod = cell["mod"];
      module_t *my_song = get_mod(mod);
      note_t * pat = my_song->patterns[integers(cell["i"]).at(0)];
      pat += (integers(cell["k"]).at(0) + integers(cell["j"]).at(0)*PAULA_VOICES);
      if (compact) {
        pt_encode_compact_cell_internal(pat, celldst, 1);
        celldst += 4;
      } else {
        memcpy(celldst, pat, sizeof(note_t));
        celldst += sizeof(note_t);
      }
    }
  }
  data_out.attr("class") = "pt2celllist";
  data_out.attr("compact_notation") = compact;
  return data_out;
}

[[cpp11::register]]
SEXP replace_cells_(list pattern, integers_matrix<> idx, raws replacement) {
  if (idx.slice_size() < 1L)
    Rf_error("Need at least one element to replace");

  module_t *my_song = get_mod(pattern["mod"]);
  uint32_t i = integers(pattern["i"]).at(0); 
  note_t * pat_base = my_song->patterns[i];
  uint8_t * source = (uint8_t *)RAW(as_sexp(replacement));
  
  uint32_t m = 0; // replacement index
  bool recycled = false;
  bool allused = false;
  for (int32_t l = 0; l < idx.slice_size(); l++) {
    if (m == 0 && l != 0) recycled = true;
    uint32_t j = idx(l, 0);
    uint32_t k = idx(l, 1);
    note_t  * target = pat_base + j * PAULA_VOICES + k;
    memcpy((uint8_t *)target, source + m * sizeof(note_t), sizeof(note_t));
    m++;
    if (m >= replacement.size()/sizeof(note_t)) {
      m = 0; // Recycle replacement values;
      allused = true;
    }
  }
  if (!allused) Rf_warning("Not all replacement values used");
  if (recycled) Rf_warning("Replacement values are recycled");
  
  return pattern;
}
