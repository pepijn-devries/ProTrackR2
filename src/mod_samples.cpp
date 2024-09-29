#include <cpp11.hpp>
#include "get_mod.h"
#include "pt2-clone/pt2_replayer_light.h"
using namespace cpp11;

moduleSample_t * get_mod_sampinf_internal(module_t * my_song, integers idx) {
  if (idx.size() != 1) Rf_error("This function only accepts a single index.");
  uint32_t i = idx.at(0);
  if (i < 0 || i >= MOD_SAMPLES) Rf_error("Index out of range");
  
  return &my_song->samples[i];
}

SEXP mod_sample_info_internal(module_t * my_song, integers idx) {
  moduleSample_t * samp = get_mod_sampinf_internal(my_song, idx);
  
  writable::list attr({
    "length"_nm       = (int)samp->length,
      "loopStart"_nm  = (int)samp->loopStart,
      "loopLength"_nm = (int)samp->loopLength,
      "fineTune"_nm   = (int)samp->fineTune,
      "volume"_nm     = (int)samp->volume,
      "text"_nm       = r_string(samp->text)
  });
  return attr;
}

SEXP mod_sample_as_raw_internal(module_t * my_song, integers idx) {
  moduleSample_t * samp = get_mod_sampinf_internal(my_song, idx);
  int8_t *sampleData = &my_song->sampleData[samp->offset];
  uint32_t len = samp->length;
  writable::raws sampledata((R_xlen_t)len);
  uint8_t * sampdest = (uint8_t *)RAW(as_sexp(sampledata));
  
  memcpy(sampdest, sampleData, len);

  SEXP attr = mod_sample_info_internal(my_song, idx);
  sexp result = as_sexp(sampledata);
  result.attr("class") = "pt2samp";
  result.attr("sample_info") = attr;
  return result;
}

[[cpp11::register]]
SEXP mod_sample_as_raw_(SEXP mod, integers idx) {
  if (idx.size() != 1) Rf_error("This function only accepts a single index.");
  module_t *my_song = get_mod(mod);
  return mod_sample_as_raw_internal(my_song, idx);
}

SEXP mod_sample_as_int_internal(module_t * my_song, integers idx) {
  moduleSample_t * samp = get_mod_sampinf_internal(my_song, idx);
  int8_t *sampleData = &my_song->sampleData[samp->offset];
  uint32_t len = samp->length;
  writable::integers sampledata((R_xlen_t)len);
  for (uint32_t j = 0; j < len; j++) {
    sampledata.at((int)j) = (int)sampleData[j];
  }
  
  SEXP attr = mod_sample_info_internal(my_song, idx);
  sexp result = as_sexp(sampledata);
  result.attr("sample_info") = attr;
  return result;
}

[[cpp11::register]]
SEXP mod_sample_info_(SEXP mod, integers idx) {
  if (idx.size() != 1) Rf_error("This function only accepts a single index.");
  module_t *my_song = get_mod(mod);
  return mod_sample_info_internal(my_song, idx);
}

[[cpp11::register]]
SEXP mod_sample_as_int_(SEXP mod, integers idx) {
  module_t *my_song = get_mod(mod);
  return mod_sample_as_int_internal(my_song, idx);
}
