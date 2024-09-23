#' Retrieve a pattern from a Protracker module
#' 
#' Get a pattern table (sequence of notes and effects on each of the 4 channels) at
#' a specific index from a Protracker module.
#' @param mod A `pt2mod` class objects from which to retrieve a pattern table
#' @param i The index (`integer`) of the pattern. Note that the index starts at 0.
#' @param ... Ignored
#' @returns A `pt2pat` object representing the pattern.
#' @examples
#' mod <- pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
#' 
#' pat <- pt2_pattern(mod, 0L)
#' @author Pepijn de Vries
#' @export
pt2_pattern <- function(mod, i, ...) {
  structure(
    list(mod = mod, i = i),
    class = "pt2pat"
  )
}
