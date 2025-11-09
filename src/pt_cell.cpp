#include <cpp11.hpp>
#include <cpp11/matrix.hpp>
#include <sstream>
#include <iomanip>
#include "get_mod.h"
#include "pt2-clone.h"
using namespace cpp11;

#define CELL_FORMAT_BUFFER_SIZE 100

[[cpp11::register]]
int pt_cell_bytesize() {
  return (sizeof(note_t));
}

[[cpp11::register]]
integers note_to_period_(strings note, std::string empty_char, int finetune) {
  if (empty_char.length() != 1)
    stop("`empty_char` should consist of 1 character.");
  if (finetune < -8 || finetune > 7)
    stop("`finetune` is out of range [-8, +7].");
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
std::string pt_note_string_raw_(raws data) {
  note_t * note = (note_t *)(RAW(as_sexp(data)));
  std::string notestr(noteNames1[periodToNote(note->period)]);
  return notestr;
}

SEXP pt_cell_as_char_internal(
    note_t *cell, int offset, strings padding, strings empty, list sformat) {
  if (padding.size() < 1 || empty.size() < 1)
    stop("Arguments must have at least one element");
  if (sformat.size() != 4) stop("'fmt' must have a length of 4.");
  note_t * cell2 = cell + offset;
  std::string notestr = (std::string)r_string(noteNames1[periodToNote(cell2->period)]);
  std::string sempt = (std::string)empty.at(0);
  const char * empt = sempt.c_str();
  if (notestr.c_str()[0] == '-')
    std::replace(notestr.begin(), notestr.end(), '-', empt[0]);
  
  if (strings(sformat["note"]).size() < 1 || strings(sformat["padding"]).size() < 1 ||
      strings(sformat["instrument"]).size() < 1 || strings(sformat["effect"]).size() < 1)
    stop("Mallformat pt2cell format");
  
  char buf[CELL_FORMAT_BUFFER_SIZE + 2] = {0};
  std::string fmt_note = strings(sformat["note"])[0];
  std::string fmt_inst = "%s";
  std::string fmt_inst_temp = strings(sformat["instrument"])[0];
  std::string fmt_efft = strings(sformat["effect"])[0];
  std::string fmt_padd = strings(sformat["padding"])[0];
  
  if (!notestr.rfind((std::string)empty[0], 0) &&
      strings(sformat["note"]).size() > 1)
    fmt_note = strings(sformat["note"])[1];
  if ((int)cell2->command == 0 && (int)cell2->param == 0 &&
      strings(sformat["effect"]).size() > 1)
    fmt_efft = strings(sformat["effect"])[1];
  size_t sz;
  std::string instr_str;
  if ((int)cell2->sample == 0 && empty.size() > 1) {
    instr_str = (std::string)empty[1] + (std::string)empty[1];
  } else {
    sz = snprintf(buf, CELL_FORMAT_BUFFER_SIZE + 1,
                  fmt_inst_temp.c_str(), (int)cell2->sample);
    instr_str = std::string(buf);
  }

  sz = snprintf(buf, CELL_FORMAT_BUFFER_SIZE + 1, fmt_efft.c_str(),
                (int)cell2->command, (int)cell2->param);
  std::string efft_str = std::string(buf);

  if ((int)cell2->command == 0 && (int)cell2->param == 0 && empty.size() > 2) {
    std::replace(efft_str.begin(), efft_str.end(), '0',
                 ((std::string)empty[2]).c_str()[0]);
  }

  std::string fmt = fmt_note + fmt_padd + fmt_inst + fmt_padd + efft_str;
  
  int second_pad = 0;
  if (padding.size() > 1) second_pad = 1;
  sz = snprintf(buf, CELL_FORMAT_BUFFER_SIZE + 1, fmt.c_str(),
                notestr.c_str(),
                ((std::string)padding[0]).c_str(),
                instr_str.c_str(),
                ((std::string)padding[second_pad]).c_str());
  if (sz > CELL_FORMAT_BUFFER_SIZE)
    stop("Cannot format cells to strings with more than %i characters",
         CELL_FORMAT_BUFFER_SIZE);
  r_string result = buf;
  writable::strings result_r(result);
  return result_r;
}

[[cpp11::register]]
strings pt_rawcell_as_char_(raws pattern, strings padding, strings empty_char, list sformat) {
  note_t * cell = (note_t *)RAW(as_sexp(pattern));
  int n_notes = (int)(pattern.size()/sizeof(note_t));
  writable::strings result((R_xlen_t)n_notes);
  for (int i = 0; i < n_notes; i++) {
    result[i] = strings(pt_cell_as_char_internal(cell, i, padding, empty_char, sformat)).at(0);
  }
  return result;
}

[[cpp11::register]]
raws pt_decode_compact_cell(raws source) {
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
raws pt_encode_compact_cell(raws source){
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
