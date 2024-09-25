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
format.pt2pat <- function(x, padding = " ", empty_char = "-", fmt = getOption("pt2_cell_format"), ...) {
  if (is.null(fmt)) {
    fmt <- list(note       = "%s",
                padding    = "%s",
                instrument = "%02i",
                effect     = "%X%02X")
    # TODO rewrite C++ to handle list
    if (requireNamespace("cli", quietly = TRUE)) {
      fmt_note <- getOption("pt2_note_format")
      fmt_inst <- getOption("pt2_instr_format")
      fmt_efft <- getOption("pt2_effect_format")
      fmt_padd <- getOption("pt2_padding_format")
      if (is.null(fmt_note)) fmt_note <- c(cli::col_blue("%s"), cli::col_silver("%s"))
      if (is.null(fmt_inst)) fmt_inst <- crayon::cyan("%02i")
      if (is.null(fmt_efft)) fmt_efft <- c(cli::col_green("%X%02X"), cli::col_silver("%X%02X"))
      if (is.null(fmt_padd)) fmt_padd <- cli::col_silver("%s")
      fmt <- list(note = fmt_note, padding = fmt_padd, instrument = fmt_inst, effect = fmt_efft)
    }
  }
  pattern_as_raw_(x$mod, as.integer(x$i)) |>
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