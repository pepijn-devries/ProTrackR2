#include <cpp11.hpp>
#include "pt2-clone/modloaders/pt2_load_mod31.h"
#include "pt2-clone/pt2_replayer_light.h"
using namespace cpp11;

[[cpp11::register]]
integers test(raws data) {
  uint8_t * buffer = (uint8_t *)RAW(as_sexp(data));
  module_t * song = loadMod31(buffer, data.size());
  writable::integers result;
  result.push_back(song->currSpeed);
  note_t *note = &song->patterns[0][0];
  result.push_back(note->period);
  modFree2(song);
  return result;
}
