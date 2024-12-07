#include <cpp11.hpp>
#include "get_mod.h"
#include "mod_header.h"
#include "pt2-clone/pt2_module_loader.h"
#include "pt2-clone/pt2_module_saver.h"
#include "pt2-clone/pt2_replayer_light.h"
using namespace cpp11;

SEXP create_mod_pointer(module_t * my_song) {
  if (my_song == NULL) Rf_error("Failed to create module");
  external_pointer<module_t, modFree2>mod(my_song);
  sexp result = as_sexp(mod);
  result.attr("class") = "pt2mod";
  return result;
}

[[cpp11::register]]
SEXP open_mod_(raws data) {
  uint8_t * buffer = (uint8_t *)RAW(as_sexp(data));
  uint32_t dat_size = data.size();
  module_t * my_song = modLoad2(buffer, &dat_size);
  return create_mod_pointer(my_song);
}

[[cpp11::register]]
raws mod_as_raw_(SEXP mod) {
  module_t *my_song = get_mod(mod);
  int n_pat = n_patterns_internal(mod);
  int mod_header = 154;
  int sample_header = 30;
  int pattern_size = 1024;
  int sample_data = 0;
  for (int i = 0; i < MOD_SAMPLES; i++) {
    sample_data += my_song->samples[i].length;
  }
  int mod_size = mod_header + MOD_SAMPLES * sample_header +
    n_pat * pattern_size + sample_data;
  writable::raws buf((R_xlen_t)mod_size);
  uint8_t * rbuf = (uint8_t *)RAW(buf.data());

  uint32_t n_written = modSave2(my_song, rbuf, mod_size);

  if (n_written != (uint32_t)mod_size) Rf_error("Unexpected data size");

  return buf;
}

[[cpp11::register]]
SEXP new_mod_(strings name) {
  module_t * my_song = createEmptyMod();
  set_mod_name_internal(my_song, name);
  return create_mod_pointer(my_song);
}