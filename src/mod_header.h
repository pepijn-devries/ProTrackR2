#pragma once

#include <cpp11.hpp>
#include "pt2-clone/pt2_structs.h"
using namespace cpp11;

int n_patterns_internal(SEXP mod);
void set_mod_name_internal(module_t * my_song, strings name);
