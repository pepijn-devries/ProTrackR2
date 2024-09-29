#pragma once

#include <cpp11.hpp>
#include "pt2-clone/pt2_structs.h"
using namespace cpp11;

SEXP pt_encode_compact_cell(raws source);
void pt_encode_compact_cell_internal(note_t * source, uint8_t * dest, uint32_t n_notes);