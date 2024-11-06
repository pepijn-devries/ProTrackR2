#' Extract a note from a ProTracker module
#' 
#' Gets note information from a cell in a pattern table in a ProTracker Module.
#' 
#' A string representing the note's key is returned by the function.
#' The first letter indicates the position of the note in the
#' [diatonic scale](https://en.wikipedia.org/wiki/Diatonic_scale).
#' The second character indicates if it is a sharp key (with a hash
#' symbol, and a dash if it is not). The third character indicates
#' the octave of the note. In ProTracker allowed notes range from
#' `"C-1"` to `"B-3"`.
#' @param x An object of class `pt2_cell`, which can be extracted
#' from a pattern table with [`pt2_cell()`].
#' @param ... Ignored
#' @returns Returns a string representing the note's key.
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' 
#' cell <- pt2_cell(mod$patterns[[1]], 0L, 0L)
#' 
#' pt2_note(cell)
#' @export
pt2_note <- function(x, ...) {
  if (!inherits(x, "pt2cell"))
    stop("`x` should inherit `pt2cell`")
  if (typeof(x) == "raw") {
    pt_note_string_raw_(x)
  } else {
    pt_note_string_(x$mod, x$i, x$k, x$j)
  }
}