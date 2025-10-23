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
#' @param x An object of class `pt2cell`, which can be extracted
#' from a pattern table with [`pt2_cell()`]. A cell list (class `pt2celllist`)
#' is also allowed. See `vignette("sel_assign")` for more details about
#' selecting cells and cell lists.
#' @param silent Don't warn about replacement values not being used or recycled.
#' @param ... Ignored
#' @param value A `character` string to replace the selected notes from `x`.
#' @returns Returns a string representing the note's key.
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' 
#' ## select a specific cell from the first pattern
#' cell <- pt2_cell(mod$patterns[[1]], 0L, 0L)
#' 
#' ## get the note played by this particular cell
#' pt2_note(cell)
#' 
#' ## Replace the notes in the first pattern
#' ## with those of the first bar of
#' ## 'Frère Jacques'
#' pt2_note(mod$patterns[[1]][]) <-
#'   c("C-2", "---", "---", "---",
#'     "D-2", "---", "---", "---",
#'     "E-2", "---", "---", "---",
#'     "C-2", "---", "---", "---")
#' @export
pt2_note <- function(x, ...) {
  if (!inherits(x, c("pt2cell", "pt2celllist")))
    stop("`x` should inherit `pt2cell` or `pt2celllist`.")
  if (typeof(x) == "raw") {
    x <- .get_raw_fun(x)(x, compact = FALSE)
    if (inherits(x, "pt2celllist")) {
      unclass(x) |>
        matrix(ncol = pt_cell_bytesize(), byrow = TRUE) |>
        apply(1L, pt_note_string_raw_, simplify = FALSE) |>
        unlist()
    } else {
      pt_note_string_raw_(x)
    }
  } else {
    .cell_helper(x, pt_note_string_)
  }
}

#' @rdname pt2_note
#' @export
`pt2_note<-` <- function(x, silent = TRUE, ..., value) {
  if (typeof(x) == "raw") {
    cur_notation <- attributes(x)$compact_notation
    cur_class <- class(x)
    raw_fun <- .get_raw_fun(x)
    x <- raw_fun(x, compact = FALSE) |>
      unclass()
    value <- note_to_period_(value, "-", 0)
    value[is.na(value)] <- 0L
    
    l <- pt_cell_bytesize()
    idx <- (seq_len(length(x)/l) - 1L)*l
    x[idx + l - 1L] <- bitwAnd(value, 0xff) |> as.raw()
    x[idx + l] <- bitwShiftR(value, l + 2L) |> as.raw()
    
    class(x) <- cur_class
    attributes(x)$compact_notation <- FALSE
    x <- raw_fun(x, compact = cur_notation)
    return(x)
  } else {
    .cell_helper(x, pt_set_note_, replacement = value, warn = !silent)
  }
  x
}

#' Get a corresponding period value from a note string
#' 
#' Back in the days, ProTracker was hardware driven on a Commodore Amiga. It made
#' advantage of a custom chipset where each chip had specific tasks. One of the chips
#' (named Paula) could play 8 bit audio samples stored in memory directly to one of
#' the four audio channels. On that chip you could set the integer 'period' value which is
#' inversely related to the sample rate at which the sample is played. Hence, it
#' defines the pitch of the sample. ProTracker used the period value to play different
#' notes. With this function you can convert a `character` string representing a
#' note to its corresponding period value used by Paula.
#' @param note A `character` string representing notes (see also [`pt2_note()`]).
#' @param empty_char A `character` that is used to represent empty values.
#' @param finetune ProTracker used `integer` `finetune` values to tweak the playback rate.
#' it should be in the range of `-8`, up to `+7`.
#' @param ... Ignored.
#' @returns Returns a `vector` of `integer` period values.
#' @examples
#' pt2_note_to_period(c("A#2", "C-1"))
#' @export
pt2_note_to_period <- function(note, empty_char = "-", finetune = 0, ...) {
  note_to_period_(note, empty_char, finetune)
}