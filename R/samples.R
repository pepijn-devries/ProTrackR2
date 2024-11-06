#' Obtain sample data and info from a ProTracker module
#' 
#' Obtain sample data and info from a ProTracker module at a specific index.
#' ProTracker modules can hold up to 31 samples. The index should range from 0 to 30.
#' @param mod An object of class `pt2mod` from which to obtain sample data and information
#' @param i The index of the requested sample (between 0 and 30).
#' @param ... Ignored.
#' @returns Returns a sample object of class `pt2samp`.
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
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

#' @rdname mod_info
#' @name pt2_name
#' @include mod_info.R
#' @export
pt2_name.pt2samp <- function(x, ...) {
  if (typeof(x) == "raw")
    attributes(x)$text else
      mod_sample_info_(x$mod, as.integer(x$i))$text
}

#' @rdname mod_info
#' @name pt2_name
#' @export
pt2_n_sample <- function(mod, ...) {
  count = 0
  for (i in 0L:30L) {
    if (mod_sample_info_(mod, i)$length > 0) count = count + 1
  }
  count
}
