#' @method format pt2mod
#' @export
format.pt2mod <- function(x, ...) {
  pt2_name(x)
}

#' @method print pt2mod
#' @export
print.pt2mod <- function(x, ...) {
  format(x, ...) |> print()
}

#' @method format pt2pat
#' @export
format.pt2pat <- function(x, ...) {
  pattern_as_raw_(x$mod, as.integer(x$i)) |>
    pt_rawcell_as_char_() |>
    matrix(ncol = 4, byrow = T)
}

#' @method print pt2pat
#' @export
print.pt2pat <- function(x, ...) {
  format(x, ...) |> print()
}

#' @method format pt2samp
#' @export
format.pt2samp <- function(x, ...) {
  si <- mod_sample_info_(x$mod, as.integer(x$i))
  sprintf("PT2 Sample '%s' (%i)", si$text, si$length)
}

#' @method print pt2samp
#' @export
print.pt2samp <- function(x, ...) {
  format(x, ...) |> print()
}