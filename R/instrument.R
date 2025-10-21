#' Extract or replace a sample index from a ProTracker pattern
#' 
#' As explained in `vignette("s3class")`, the ProTracker pattern table
#' consists of cells containing information about the note and instrument
#' to be played. This function extracts the sample index (instrument)
#' from such a cell.
#' @param x An object of class `pt2cell`, which can be extracted
#' from a pattern table with [`pt2_cell()`]. A cell list (class `pt2celllist`)
#' is also allowed. See `vignette("sel_assign")` for more details about
#' selecting cells and cell lists.
#' @param silent Don't warn about replacement values not being used or recycled.
#' @param ... Ignored.
#' @param value Replacement value for the instrument (sample id). An `integer` value
#' ranging from `0` to `31`.
#' @returns Returns the `integer` sample index in `x`. The index
#' has a base of `1`. An index of `0` means 'no sample'.
#' In case of the assignment operator (`<-`) an updated version of
#' `x` is returned
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' 
#' ## select a specific cell from the first pattern
#' cell <- pt2_cell(mod$patterns[[1]], 0L, 0L)
#' 
#' ## get the sample number used in this cell
#' pt2_instrument(cell)
#' 
#' ## Replace the instrument in all cells of
#' ## pattern 1 with sample number 3:
#' pt2_instrument(mod$patterns[[1]][]) <- 3
#' @include cell.R
#' @export
pt2_instrument <- function(x, ...) {
  if (!inherits(x, c("pt2cell", "pt2celllist")))
    stop("`x` should inherit `pt2cell` or `pt2celllist`")
  if (typeof(x) == "raw") {
    raw_fun <- .get_raw_fun(x)
    x <- raw_fun(x, compact = FALSE) |>
      unclass()
    l <- pt_cell_bytesize()
    idx <- (seq_len(length(x)/l) - 1L)*l + 2L
    as.integer(x[idx])
  } else {
    .cell_helper(x, pt_instr_)
  }
}

#' @rdname pt2_instrument
#' @export
`pt2_instrument<-` <- function(x, silent = TRUE, ..., value) {
  value <- as.integer(value)
  if (any(is.na(value) | value < 0L | value > 31L))
    stop("Replacement value should not contain `NA` or values <0 or >31")
  if (typeof(x) == "raw") {
    cur_notation <- attributes(x)$compact_notation
    cur_class <- class(x)
    raw_fun <- .get_raw_fun(x)
    x <- raw_fun(x, compact = FALSE) |>
      unclass()
    l <- pt_cell_bytesize()
    idx <- (seq_len(length(x)/l) - 1L)*l + 2L
    x[idx] <- as.raw(value)
    class(x) <- cur_class
    attributes(x)$compact_notation <- FALSE
    x <- raw_fun(x, compact = cur_notation)
    return(x)
  } else {
    .cell_helper(x, pt_set_instr_, replacement = value, warn = !silent)
  }
  x
}