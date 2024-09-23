#' Obtain Protracker module information
#' 
#' Obtain information about a protracker module or embedded samples.
#' @param x,mod A `pt2mod` class object for which to obtain information. For `x`
#' also samples of class `pt2samp` are allowed as input.
#' @param ... Ignored
#' @returns Returns information about the specified Protracker module
#' @author Pepijn de Vries
#' @examples
#' mod <- pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
#' 
#' pt2_n_pattern(mod)
#' pt2_pattern_table(mod)
#' pt2_name(mod)
#' @rdname mod_info
#' @export
#' @include helpers.R
pt2_length <- function(mod, ...) {
  .check_mod(mod)
  mod_length_(mod)
}

#' @rdname mod_info
#' @export
#' @include helpers.R
pt2_n_pattern <- function(mod, ...) {
  .check_mod(mod)
  max(mod_pattab_(mod)) + 1
}

#' @rdname mod_info
#' @export
#' @include helpers.R
pt2_pattern_table <- function(mod, ...) {
  .check_mod(mod)
  mod_pattab_(mod)
}

#' @rdname mod_info
#' @export
pt2_name <- function(x, ...) {
  UseMethod("pt2_name", x)
}

#' @rdname mod_info
#' @name pt2_name
#' @export
#' @include helpers.R
pt2_name.pt2mod <- function(x, ...) {
  .check_mod(x)
  mod_name_(x)
}

#' @rdname mod_info
#' @name pt2_name
#' @export
pt2_name.pt2samp <- function(x, ...) {
  mod_sample_info_(x$mod, as.integer(x$i))$text
}
