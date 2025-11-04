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
    size    <- ifelse(compact, 4L, pt_cell_bytesize())
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

.get_raw_fun <- function(x) {
  fun <- as.raw.pt2cell
  if (inherits(x, "pt2celllist")) fun <- as.raw.pt2celllist
  return (fun)
}

#' Convert object into a pt2cell or pt2celllist class object
#' 
#' Protracker uses pattern tables to annotate music progression.
#' Each cell in the table contains information about the note,
#' sample identifier and sound effect. This function coerces objects
#' to `pt2cell` or `pt2cell` class objects, such that they can be inserted into
#' patterns.
#' @param x A (`vector` of) `character` string(s), to be coerced to a
#' `pt2cell` class object. The first two or three characters represent
#' the note (see `pt2_note()`). this is followed by two numerical characters
#' representing the sample number. And finally three hexadecimal characters
#' representing an effect or trigger. These three parts can optionally be
#' padded with spaces. A valid string would for instance be `"C#2 01 C1A`.
#' @param ... Ignored
#' @returns An object of class `pt2cell`, `pt2celllist`.
#' @examples
#' as_pt2cell("A-3 02 121")
#' as_pt2cell("--- 01 000")
#' as_pt2celllist(c("A-3 02 121", "--- 01 000"))
#' @export
as_pt2cell <- function(x, ...) {
  UseMethod("as_pt2cell")
}

#' @method as_pt2cell character
#' @export
as_pt2cell.character <- function(x, ...) {
  if (length(x) != 1) stop("Can only coerce a one-length vector to `pt2cell`")
  x <- gsub("[ \t\n]", "", x) |>
    toupper()
  if (nchar(x) == 7L) x <- paste(substr(x, 0L, 1L), "-", substr(x, 2L, 7L), sep = "")
  if (nchar(x) != 8L)
    stop("pt2cell should contain 8 non-whitespace characters")
  x <- gsub("(?<=^.)[^#]", "-", x, perl = TRUE)
  cell <- structure(raw(4), compact_notation = TRUE, class = "pt2cell")
  pt2_instrument(cell) <- as.integer(substr(x, 4L, 5L))
  pt2_command(cell)    <- substr(x, 6L, 8L)
  pt2_note(cell)       <- substr(x, 1L, 3L)
  return(cell)
}

#' @rdname as_pt2cell
#' @export
as_pt2celllist <- function(x, ...) {
  UseMethod("as_pt2celllist")
}

#' @method as_pt2celllist character
#' @export
as_pt2celllist.character <- function(x, ...) {
  celllist <- lapply(x, as_pt2cell)
  structure(
    do.call(c, celllist),
    compact_notation = attributes(celllist[[1]])$compact_notation,
    class = "pt2celllist")
}