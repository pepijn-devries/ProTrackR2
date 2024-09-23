#include <cpp11.hpp>
#include "pt2-clone/pt2_config.h"
#include "pt2-clone/pt2_audio.h"
using namespace cpp11;

[[cpp11::register]]
SEXP pt_init_(void) {
  loadConfig();
  setupAudio();
  return R_NilValue;
}
