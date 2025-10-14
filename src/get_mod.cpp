#include <cpp11.hpp>
#include "get_mod.h"
using namespace cpp11;

module_t * get_mod(SEXP mod) {
  module_t *my_song = reinterpret_cast<module_t *>(R_ExternalPtrAddr(mod));
  if (my_song == NULL) stop("Invalid pointer");
  return my_song;
}