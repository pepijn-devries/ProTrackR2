#include <cpp11.hpp>
#include "get_mod.h"
#include "pt2-clone.h"
using namespace cpp11;

moduleSample_t * get_mod_sampinf_internal(module_t * my_song, int idx) {
  if (idx < 0 || idx >= MOD_SAMPLES) stop("Index out of range");
  
  return &my_song->samples[idx];
}

list mod_sample_info_internal2(moduleSample_t * samp) {

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

list mod_sample_info_internal(module_t * my_song, int idx) {
  moduleSample_t * samp = get_mod_sampinf_internal(my_song, idx);
  return mod_sample_info_internal2(samp);
}

raws mod_sample_as_raw_internal(module_t * my_song, int idx) {
  moduleSample_t * samp = get_mod_sampinf_internal(my_song, idx);
  int8_t *sampleData = &my_song->sampleData[samp->offset];
  uint32_t len = samp->length;
  writable::raws sampledata((R_xlen_t)len);
  uint8_t * sampdest = (uint8_t *)RAW(as_sexp(sampledata));
  
  memcpy(sampdest, sampleData, len);

  SEXP attr = mod_sample_info_internal(my_song, idx);
  sampledata.attr("class") = "pt2samp";
  sampledata.attr("sample_info") = attr;
  return sampledata;
}

[[cpp11::register]]
raws mod_sample_as_raw_(SEXP mod, int idx) {
  module_t *my_song = get_mod(mod);
  return mod_sample_as_raw_internal(my_song, idx);
}

integers mod_sample_as_int_internal(module_t * my_song, int idx) {
  moduleSample_t * samp = get_mod_sampinf_internal(my_song, idx);
  int8_t *sampleData = &my_song->sampleData[samp->offset];
  uint32_t len = samp->length;
  writable::integers sampledata((R_xlen_t)len);
  for (uint32_t j = 0; j < len; j++) {
    sampledata.at((int)j) = (int)sampleData[j];
  }
  
  SEXP attr = mod_sample_info_internal(my_song, idx);
  sampledata.attr("sample_info") = attr;
  return sampledata;
}

[[cpp11::register]]
list mod_sample_info_(SEXP mod, int idx) {
  module_t *my_song = get_mod(mod);
  return mod_sample_info_internal(my_song, idx);
}

[[cpp11::register]]
integers mod_sample_as_int_(SEXP mod, int idx) {
  module_t *my_song = get_mod(mod);
  return mod_sample_as_int_internal(my_song, idx);
}

[[cpp11::register]]
logicals validate_sample_raw_(raws smp_data) {
  bool result = true;
  list my_attr   = list(smp_data.attr("sample_info"));
  int length     = writable::integers(my_attr["length"]).at(0);
  int loopStart  = writable::integers(my_attr["loopStart"]).at(0);
  int loopLength = writable::integers(my_attr["loopLength"]).at(0);
  int volume     = writable::integers(my_attr["volume"]).at(0);
  int fineTune   = writable::integers(my_attr["fineTune"]).at(0);
  r_string text  = writable::strings(my_attr["text"]).at(0);

  // Sample size should be even
  result = result &&
    !(length % 2 == 1 || smp_data.size() != length || length < 0 || length > config.maxSampleLength);

  result = result &&
    !(loopStart % 2 == 1 || loopStart < 0 || loopStart > length);

  result = result &&
    !(loopLength % 2 == 1 || loopLength < 2 || (loopStart + loopLength) > length);

  result = result &&
    !(fineTune < 0 || fineTune > 0xf);

  result = result &&
    !(volume < 0 || volume > 64);

  result = result &&
    !(text.size() > 22);

  writable::logicals result_sexp((R_xlen_t)1);
  result_sexp.at(0) = result;

  return result_sexp;
}

[[cpp11::register]]
SEXP mod_set_sample_(SEXP mod, int idx, raws smp_data) {
  module_t *my_song = get_mod(mod);
  moduleSample_t * samp = get_mod_sampinf_internal(my_song, idx);
  int8_t *sampleData = &my_song->sampleData[samp->offset];
  // check validity of smp_data
  validate_sample_raw_(smp_data);
  
  list my_attr = list(smp_data.attr("sample_info"));
  
  memset(&samp->text, 0, 22);
  
  if (my_attr == R_NilValue) {
    samp->length     = smp_data.size();
    samp->loopStart  = 0;
    samp->loopLength = 2;
    samp->fineTune   = 0;
    samp->volume     = 64;
  } else {
    samp->length     = writable::integers(my_attr["length"]).at(0);
    samp->loopStart  = writable::integers(my_attr["loopStart"]).at(0);
    samp->loopLength = writable::integers(my_attr["loopLength"]).at(0);
    samp->fineTune   = writable::integers(my_attr["fineTune"]).at(0);
    samp->volume     = writable::integers(my_attr["volume"]).at(0);
    r_string name    = writable::strings(my_attr["text"]).at(0);
    int len = name.size();
    len = len > 22 ? 22 : len;
    memcpy(&samp->text, ((std::string)name).c_str(), len);
  }
  uint8_t * buffer = (uint8_t *)RAW(as_sexp(smp_data));
  memcpy(sampleData, buffer, smp_data.size());
  
  return mod;
}
