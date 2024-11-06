#' Select a cell from a ProTracker pattern table
#'
#' A cell is an element at a specific row and column (channel). It holds information
#' aboute the note to be played, the instrument (sample) number and the effect to be applied.
#' For mor information about cells (class `pt2cell`) consult `vignette("s3class")`.
#' For more information about selecting elements from ProTrackR2 class objects check out
#' `vignette("select_opts")`.
#' @param pattern A `pt2pat` class object to extract a cell (`pt2cell`) from.
#' @param i,j Indices for extracting or replacing ProTrackR2 object elements. The indices
#' starts at 0, for consistency with ProTracker!
#' @param ... Ignored
#' @returns Returns a cell object from the table as class `pt2cell`.
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' 
#' pt2_cell(mod$patterns[[1]], 0L, 0L)
#' @author Pepijn de Vries
#' @export
pt2_cell <- function(pattern, i, j, ...) {
  
  if (!inherits(pattern, "pt2pat"))
    stop("`pattern` should be a p2pat class object")
  
  if (length(i) != 1L || length(j) != 1L)
    stop("Only one index per call allowed")
  
  i <- as.integer(i)
  j <- as.integer(j)
  if (is.na(i) || i < 0L || i > 63L || is.na(j) || j < 0L || j > 3L)
    stop("Index out of range")
  
  if (is.raw(pattern)) {
    compact <- attributes(pattern)$compact_notation
    size    <- ifelse(compact, 4L, 6L)
    pattern <- unclass(pattern)
    result  <- pattern[seq_len(size) + (i*4L + j) * size]
    class(result) <- "pt2cell"
    attributes(result)$compact_notation <- compact
  } else {
    result <- c(unclass(pattern), j = i, k = j)
    class(result) <- "pt2cell"
  }
  result
}
