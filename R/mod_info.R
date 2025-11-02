#' Obtain ProTracker module information
#' 
#' Obtain information about a ProTracker module or embedded samples.
#'
#' You can use the following functions to get or set information on
#' a ProTracker modules (represented by `pt2mod` class objects):
#'
#'   * `pt2_length()`: get or set the length the pattern table.
#'   * `pt2_n_pattern()`: number of distinct patterns. Same as
#'     `length(mod$patterns)`.
#'   * `pt2_pattern_table()`: get or set table of pattern indexes. Patterns
#'      will be played in this order. Normally, only the first `pt2_length()`
#'      number of listed patterns are played. Patterns beyond this length are
#'      only played if you explicitly start the module at that position,
#'      or the module contains jump commands.
#'   * `pt2_name()`: get or set the name of a module. Can also be used
#'     on samples in a module. Names in a ProTracker module are limited.
#'     Module names are automatically truncated to 20 UTF8 characters.
#'     For samples, the names are truncated to 22 characters.
#' @param x,mod A `pt2mod` class object for which to obtain information. For `x`
#' also samples of class `pt2samp` are allowed as input.
#' @param value Replacement value. In case of:
#'  * `pt2_length<-`: new length of a module in number of patterns in the pattern
#'  * `pt2_pattern_table<-`: a new patter table. A 128 long vector of integers between
#'    0 and 99.
#'  * `pt2_name<-`: a new name (or names) for x
#' @param ... Ignored
#' @returns Returns information about the specified ProTracker module
#' @author Pepijn de Vries
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' 
#' pt2_length(mod)
#' pt2_n_pattern(mod)
#' pt2_n_sample(mod)
#' pt2_pattern_table(mod)
#' pt2_name(mod)
#' pt2_name(pt2_sample(mod, 4L))
#' 
#' mod2 <- pt2_new_mod("new")
#' pt2_length(mod2) <- 3L
#' pt2_pattern_table(mod2)[1L:3L] <- c(0L, 2L, 1L)
#' pt2_name(mod2) <- "foobar"
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
`pt2_length<-` <- function(mod, ..., value) {
  set_mod_length_(mod, as.integer(value))
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
#' @include helpers.R
`pt2_pattern_table<-` <- function(mod, ..., value) {
  .check_mod(mod)
  update_pattern_sequence_(mod, as.integer(value))
  mod
}

#' @rdname mod_info
#' @name pt2_name
#' @export
pt2_name <- function(x, ...) {
  UseMethod("pt2_name")
}

#' @rdname mod_info
#' @name pt2_name<-
#' @export
`pt2_name<-` <- function(x, ..., value) {
  UseMethod("pt2_name<-")
}

#' @rdname mod_info
#' @method pt2_name pt2mod
#' @export
#' @include helpers.R
pt2_name.pt2mod <- function(x, ...) {
  .check_mod(x)
  mod_name_(x)
}

#' @rdname mod_info
#' @method pt2_name<- pt2mod
#' @export
#' @include helpers.R
`pt2_name<-.pt2mod` <- function(x, ..., value) {
  .check_mod(x)
  set_mod_name_(x, value)
}
