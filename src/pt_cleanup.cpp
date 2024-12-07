#include <cpp11.hpp>
#include "pt2-clone.h"
using namespace cpp11;

[[cpp11::register]]
SEXP pt_cleanup_(void) {
  // Free the buffers allocated by SetupAudio()
  audioClose();

  return R_NilValue;
}
