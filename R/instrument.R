#' Extract a sample index from a ProTracker pattern
#' 
#' As explained in `vignette("s3class")`, the ProTracker pattern table
#' consists of cells containing information about the note and instrument
#' to be played. This function extracts the sample index (instrument)
#' from such a cell.
#' @param x An object of class `pt2_cell`, which can be extracted
#' from a pattern table with [`pt2_cell()`].
#' @param ... Ignored.
#' @returns Returns the `integer` sample index in `x`. The index
#' has a base of `1`. An index of `0` means 'no sample'.
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' 
#' cell <- pt2_cell(mod$patterns[[1]], 0L, 0L)
#' 
#' pt2_instrument(cell)
#' @export
pt2_instrument <- function(x, ...) {
  if (!inherits(x, "pt2cell"))
    stop("`x` should inherit `pt2cell`")
  if (typeof(x) == "raw") {
    pt_instr_raw_(x)
  } else {
    pt_instr_(x$mod, x$i, x$k, x$j)
  }
}