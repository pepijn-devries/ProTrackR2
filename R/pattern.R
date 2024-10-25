#' Retrieve a pattern from a ProTracker module
#' 
#' Get a pattern table (sequence of notes and effects on each of the 4 channels) at
#' a specific index from a ProTracker module.
#' @param mod A `pt2mod` class objects from which to retrieve a pattern table
#' @param i The index (`integer`) of the pattern. Note that the index starts at 0.
#' @param ... Ignored
#' @returns A `pt2pat` object representing the pattern.
#' @examples
#' mod <- pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
#' 
#' pt2_pattern(mod, 0L)
#' @author Pepijn de Vries
#' @export
pt2_pattern <- function(mod, i, ...) {
  structure(
    list(mod = mod, i = as.integer(i)),
    class = "pt2pat"
  )
}

#' Create a new ProTracker pattern
#' 
#' Creates a new ProTracker pattern, consisting of four channels and 64 rows.
#' @param ... Currently ignored
#' @param compact Should the pattern be formatted using a compact notation (as used for
#' file storage), or a none-compact format as used by the player? This can be
#' set with the `compact` argument.
#' @returns Returns a new clean `pt2pat` object.
#' @examples
#' pt2_new_pattern()
#' @author Pepijn de Vries
#' @export
pt2_new_pattern <- function(..., compact = TRUE) {
  x <- raw(ifelse(compact, 1024L, 1536L))
  class(x) <- "pt2pat"
  attributes(x)$compact_notation <- compact
  x
}
