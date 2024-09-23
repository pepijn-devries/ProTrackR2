#' Obtain sample data and info from a Protracker module
#' 
#' Obtain sample data and info from a Protracker module at a specific index.
#' Protracker modules can hold up to 31 samples. The index should range from 0 to 30.
#' @param mod An object of class `pt2mod` from which to obtain sample data and information
#' @param i The index of the requested sample (between 0 and 30).
#' @param ... Ignored.
#' @returns Returns a sample object of class `pt2samp`.
#' @examples
#' mod <- pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
#' 
#' smp <- pt2_sample(mod, 0L)
#' @author Pepijn de Vries
#' @include helpers.R
#' @export
pt2_sample <- function(mod, i, ...) {
  .check_mod(mod)
  structure(
    list(mod = mod, i = as.integer(i)),
    class = "pt2samp"
  )
}
