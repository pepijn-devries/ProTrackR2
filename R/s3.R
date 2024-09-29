#' @method format pt2mod
#' @export
format.pt2mod <- function(x, ...) {
  sprintf("pt2mod '%s' len %i patt %i samp %i",
          pt2_name(x), pt2_length(x), pt2_n_pattern(x), pt2_n_sample(x))
  
}

#' @method print pt2mod
#' @export
print.pt2mod <- function(x, ...) {
  format(x, ...) |> paste0("\n") |> cat()
}

#' @method format pt2pat
#' @export
format.pt2pat <- function(x, padding = " ", empty_char = "-", fmt = getOption("pt2_cell_format"), ...) {
  if (is.null(fmt)) {
    fmt <- list(note       = "%s",
                padding    = "%s",
                instrument = "%02i",
                effect     = "%X%02X")
    if (requireNamespace("cli", quietly = TRUE)) {
      fmt_note <- getOption("pt2_note_format")
      fmt_inst <- getOption("pt2_instr_format")
      fmt_efft <- getOption("pt2_effect_format")
      fmt_padd <- getOption("pt2_padding_format")
      if (is.null(fmt_note)) fmt_note <- c(cli::col_blue("%s"), cli::col_silver("%s"))
      if (is.null(fmt_inst)) fmt_inst <- cli::col_cyan("%02i")
      if (is.null(fmt_efft)) fmt_efft <- c(cli::col_green("%X%02X"), cli::col_silver("%X%02X"))
      if (is.null(fmt_padd)) fmt_padd <- cli::col_silver("%s")
      fmt <- list(note = fmt_note, padding = fmt_padd, instrument = fmt_inst, effect = fmt_efft)
    }
  }
  as.raw.pt2pat(x, compact = FALSE) |>
    pt_rawcell_as_char_(
      as.character(padding),
      as.character(empty_char),
      as.list(fmt)) |>
    matrix(ncol = 4, byrow = T)
}

#' @method print pt2pat
#' @export
print.pt2pat <- function(x, sep = "  ", show_header = TRUE, show_row = TRUE, ...) {
  h <- NULL
  row_text <- NULL
  x <- format(x, ...)
  if (show_row) row_text <- sprintf("%02i", seq_len(nrow(x)) - 1L)
  if (show_header) {
    if (!is.null(row_text)) h <- "rw"
    y <- x[[1]]
    if (requireNamespace("cli", quietly = TRUE))
      y <- cli::ansi_strip(y)
    h <- c(h, paste0("chnnl ", 1:4, strrep(" ", nchar(y) - 7)))
  }
  rbind(h, cbind(row_text, x)) |>
    apply(1, paste0, collapse = sep[[1]]) |>
    paste0(collapse = "\n") |>
    cat()
}

#' @method as.character pt2pat
#' @export
as.character.pt2pat <- function(x, ...) {
  x <- format(x, ...)
  if (requireNamespace("cli", quietly = TRUE))
    x <- apply(x, 2, cli::ansi_strip, simplify = TRUE)
  x
}

#' @method as.raw pt2pat
#' @export
as.raw.pt2pat <- function(x, ...) {
  compact <- list(...)$compact
  if (is.null(compact)) compact <- TRUE
  UseMethod("as.raw.pt2pat", compact)
}

#' @method as.raw.pt2pat logical
#' @name as.raw
#' @export
as.raw.pt2pat.logical <- function(x, compact = TRUE, ...) {
  if (typeof(x) == "raw") {
    cur_notation <- attributes(x)$compact_notation
    if (cur_notation == compact) return (x)
    if (cur_notation) {
      x <- pt_decode_compact_cell(x)
    } else {
      x <- pt_encode_compact_cell(x)
    }
    class(x) <- "pt2pat"
    attributes(x)$compact_notation <- !cur_notation
    x
  } else {
    pattern_as_raw_(x$mod, as.integer(x$i), compact)
  }
}

#' @method format pt2samp
#' @export
format.pt2samp <- function(x, ...) {
  si <- if (typeof(x) == "raw") {
    attributes(x)$sample_info
  } else {
    mod_sample_info_(x$mod, as.integer(x$i))
  }
  sprintf("PT2 Sample '%s' (%i)", si$text, si$length)
}

#' @method print pt2samp
#' @export
print.pt2samp <- function(x, ...) {
  format(x, ...) |> print()
}

#' @method as.raw pt2mod
#' @export
as.raw.pt2mod <- function(x, ...) {
  mod_as_raw_(x)
}

#' @method as.raw pt2samp
#' @export
as.raw.pt2samp <- function(x, ...) {
  if (typeof(x) == "raw") return (x)
  mod_sample_as_raw_(x$mod, x$i)
}

#' @method as.integer pt2samp
#' @export
as.integer.pt2samp <- function(x, ...) {
  if (typeof(x) == "raw") {
    a <- attributes(x)
    x <-
      unclass(x) |>
      as.integer()
    x[x > 127L] <- x[x > 127L] - 256L
    attributes(x) <- a
    x
  } else {
    mod_sample_as_int_(x$mod, x$i)
  }
}
