#' Implementation of basic S3 methods
#' 
#' Implementation of basic S3 methods, such as, `format`, `print`, `as.raw` and `as.character` (
#' see usage section for a complete overview). See `vignette('s3class')` for an overview
#' of ProTrackR2 S3 class objects. See usage section for an overview of available methods.
#' @param x Object to apply S3 method to. See 'usage' section for allowed object types.
#' @param padding A `vector` of `character` strings used to pad between note and instrument number (element 1)
#' and between instrument number and effect command (element 2). Values are recycled.
#' @param empty_char A `vector` of single `character` values used to represent empty pattern elements.
#' First element is used for notes, second for instrument number, the third for effect commands
#' (see also `vignette("effect_commands")`). Values are recycled.
#' @param fmt Experimental feature to format a `pt2cell`. It should be a named `list` containing
#' formatting strings for elements in the cell. It should contain the elements `"note"`, `"padding"`,
#' `"instrument"` and `"effect"`. Its implementation may change in future releases.
#' @param max.print Maximum number of elements to be printed.
#' @param sep A separator `character` string for concatenating pattern table columns (i.e. channels).
#' @param show_header A `logical` value indicating if a header should be shown for the
#' pattern table.
#' @param show_row A `logical` value indicating if the row of a pattern table should
#' be labelled with its index.
#' @param compact Should the pattern be formatted using a compact notation (as used for
#' file storage), or a none-compact format as used by the player? This can be
#' set with the `compact` argument.
#' @param ... Passed on to other methods.
#' @returns The following is returned by the different methods:
#'   * `format`: a formatted `character` representation of the object
#'   * `print`: same as `format`
#'   * `as.character`: same as `format`
#'   * `as.raw`: a `raw` representation of the object. In many cases it inherits the same
#'     class as `x`
#'   * `as.integer`: converted `raw` 8 bit sample data to signed pulse code modulation `integer`
#'     values between -128 and +127.
#'   * `length` returns number of elements in `x`
#' @method format pt2mod
#' @rdname s3methods
#' @examples
#' ## First create some ProTrackR2 objects to which
#' ## S3 methods can be applied:
#' 
#' mod      <- pt2_read_mod(pt2_demo())
#' patterns <- mod$patterns
#' pattern  <- patterns[[1]]
#' cells    <- pattern[1:4,1]
#' cell     <- cells[[1]]
#' samples  <- mod$samples
#' sample   <- samples[[1]]
#' cmnd     <- pt2_command(cell)
#' 
#' ## Let's go wild with S3 methods:
#' print(mod)
#' print(patterns)
#' print(pattern)
#' print(cells)
#' print(cell)
#' print(samples)
#' print(sample)
#' print(cmnd)
#' 
#' format(mod)
#' format(patterns)
#' format(pattern)
#' format(cells)
#' format(cell)
#' format(samples)
#' format(sample)
#' format(cmnd)
#' 
#' as.character(pattern)
#' as.character(cells)
#' as.character(cell)
#' as.character(cmnd)
#' 
#' as.raw(mod)
#' as.raw(pattern)
#' as.raw(cells)
#' as.raw(cell)
#' as.raw(sample)
#' as.raw(cmnd)
#' 
#' as.integer(sample)
#' @export
format.pt2mod <- function(x, ...) {
  dur <- pt2_duration(x) |>
    as.numeric("secs")
  sprintf("pt2mod '%s' [%02.f:%02.f]",
          pt2_name(x), dur/60, dur%%60)
}

#' @method print pt2mod
#' @rdname s3methods
#' @export
print.pt2mod <- function(x, ...) {
  format(x, ...) |> paste0("\n") |> cat()
}

#' @method format pt2pat
#' @rdname s3methods
#' @export
format.pt2pat <- function(x, padding = " ", empty_char = "-", fmt = getOption("pt2_cell_format"), ...) {
  .cell_format(x, padding, empty_char, fmt, as.raw.pt2pat) |>
    matrix(ncol = 4, byrow = T)
}

#' @method as.character pt2command
#' @rdname s3methods
#' @export
as.character.pt2command <- function(x, ...) {
  result <- character()
  for (i in seq_len(length(x))) {
    cmnd <- as.integer(unclass(x[[i]]))
    cmnd <- sprintf("%X%02X", cmnd[[1]], cmnd[[2]])
    result <- c(result, cmnd)
  }
  result
}

#' @method format pt2command
#' @rdname s3methods
#' @export
format.pt2command <- function(x, fmt = getOption("pt2_effect_format"), ...) {
  matrix(x, ncol = 2L, byrow = TRUE) |>
    apply(1, .command_format, fmt, simplify = FALSE) |>
    unlist()
}

#' @method print pt2command
#' @rdname s3methods
#' @export
print.pt2command <- function(x, max.print = 10L, ...) {
  len  <- length(x)
  last <- NULL
  if (length(x) > max.print) {
    x <- x[seq_len(max.print)]
    last <- sprintf("Reached `max.print` %s more records not showing",
                    len - max.print)
  }
  x |>
    format(...) |>
    c(last) |>
    paste0(collapse = "\n") |>
    paste0("\n") |>
    cat()
}

#' @method print pt2pat
#' @rdname s3methods
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
#' @rdname s3methods
#' @export
as.character.pt2pat <- function(x, ...) {
  x <- format(x, ...)
  if (requireNamespace("cli", quietly = TRUE))
    x <- apply(x, 2, cli::ansi_strip, simplify = TRUE)
  x
}

#' @method as.raw pt2celllist
#' @rdname s3methods
#' @export
as.raw.pt2celllist <- function(x, ...) {
  compact <- list(...)$compact
  if (is.null(compact)) compact <- TRUE
  UseMethod("as.raw.pt2celllist", compact)
}

#' @method as.raw.pt2celllist logical
#' @rdname s3methods
#' @export
as.raw.pt2celllist.logical <- function(x, compact = TRUE, ...) {
  d <- attr(x, "celldim")
  cur_notation <- attributes(x)$compact_notation
  width <- ifelse(cur_notation, 4L, pt_cell_bytesize())
  x <-
    matrix(unclass(x), ncol = width, byrow = TRUE) |>
    apply(1, \(y) {
      class(y) <- "pt2cell"
      attributes(y)$compact_notation <- cur_notation
      as.raw.pt2cell(y, compact = compact)
    }, simplify = FALSE) |> unlist()
  structure(x, class = "pt2celllist", celldim = d, compact_notation = compact)
}

#' @method as.raw pt2pat
#' @rdname s3methods
#' @export
as.raw.pt2pat <- function(x, ...) {
  compact <- list(...)$compact
  if (is.null(compact)) compact <- TRUE
  UseMethod("as.raw.pt2pat", compact)
}

#' @method as.raw.pt2pat logical
#' @rdname s3methods
#' @export
as.raw.pt2pat.logical <- function(x, compact = TRUE, ...) {
  if (typeof(x) == "raw") {
    cur_notation <- attributes(x)$compact_notation
    if (cur_notation == compact) return (x)
    x <- pt_decode_compact_cell(x)
    class(x) <- "pt2pat"
    attributes(x)$compact_notation <- !cur_notation
    x
  } else {
    cells_as_raw_(x$mod, as.integer(x$i), compact, 0L, 0L)
  }
}

#' @method format pt2cell
#' @rdname s3methods
#' @export
format.pt2cell <- function(x, padding = " ", empty_char = "-", fmt = getOption("pt2_cell_format"), ...) {
  .cell_format(x, padding, empty_char, fmt, as.raw.pt2cell)
}

#' @method print pt2cell
#' @rdname s3methods
#' @export
print.pt2cell <- function(x, ...) {
  format(x, ...) |> paste0("\n") |> cat()
}

#' @method as.character pt2cell
#' @rdname s3methods
#' @export
as.character.pt2cell <- function(x, ...) {
  x <- format(x, ...)
  if (requireNamespace("cli", quietly = TRUE))
    x <- cli::ansi_strip(x)
  x
}

#' @method as.character pt2celllist
#' @rdname s3methods
#' @export
as.character.pt2celllist <- function(x, ...) {
  result <- character()
  for (i in seq_len(length(x))) {
    result <- c(result, as.character(x[[i]]))
  }
  result
}

#' @method as.raw pt2command
#' @rdname s3methods
#' @export
as.raw.pt2command <- function(x, ...) {
  x # pt2command is already raw
}

#' @method as.raw pt2cell
#' @rdname s3methods
#' @export
as.raw.pt2cell <- function(x, ...) {
  compact <- list(...)$compact
  if (is.null(compact)) compact <- TRUE
  UseMethod("as.raw.pt2cell", compact)
}

#' @method as.raw.pt2cell logical
#' @rdname s3methods
#' @export
as.raw.pt2cell.logical <- function(x, compact = TRUE, ...) {
  cur_notation <- attributes(x)$compact_notation
  if (is.null(cur_notation))
    stop("Unknown notation of `pt2cell`")
  if (cur_notation == compact) return (x)
  if (cur_notation) {
    x <- pt_decode_compact_cell(x)
  } else {
    x <- pt_encode_compact_cell(x)
  }
  class(x) <- "pt2cell"
  attributes(x)$compact_notation <- !cur_notation
  x
}

#' @method format pt2samp
#' @rdname s3methods
#' @export
format.pt2samp <- function(x, ...) {
  si <- attributes(x)$sample_info
  sprintf("PT2 Sample '%s' [%i]", si$text, si$length)
}

#' @method print pt2samp
#' @rdname s3methods
#' @export
print.pt2samp <- function(x, ...) {
  format(x, ...) |> print()
}

#' @method format pt2patlist
#' @rdname s3methods
#' @export
format.pt2patlist <- function(x, ...) {
  sprintf("pattern list [n=%i]", length(x))
}

#' @method print pt2patlist
#' @rdname s3methods
#' @export
print.pt2patlist <- function(x, ...) {
  format(x, ...) |> paste0("\n") |> cat()
}

#' @method format pt2celllist
#' @rdname s3methods
#' @export
format.pt2celllist <- function(x, ...) {
  sprintf("cell list [n=%i]", length(x))
}

#' @method print pt2celllist
#' @rdname s3methods
#' @export
print.pt2celllist <- function(x, ...) {
  format(x, ...) |> paste0("\n") |> cat()
}

#' @method format pt2samplist
#' @rdname s3methods
#' @export
format.pt2samplist <- function(x, ...) {
  sprintf("sample list [n=%i]", length(x))
}

#' @method print pt2samplist
#' @rdname s3methods
#' @export
print.pt2samplist <- function(x, ...) {
  format(x, ...) |> paste0("\n") |> cat()
}

#' @method as.raw pt2mod
#' @rdname s3methods
#' @export
as.raw.pt2mod <- function(x, ...) {
  mod_as_raw_(x)
}

#' @method as.raw pt2samp
#' @rdname s3methods
#' @export
as.raw.pt2samp <- function(x, ...) {
  if (typeof(x) == "raw") return (x)
  mod_sample_as_raw_(x$mod, x$i)
}

#' @method as.integer pt2samp
#' @rdname s3methods
#' @export
as.integer.pt2samp <- function(x, ...) {
  a <- attributes(x)
  x <-
    unclass(x) |>
    as.integer()
  x[x > 127L] <- x[x > 127L] - 256L
  attributes(x) <- a[!names(a) %in% "class"]
  x
}

.command_format <- function(x, fmt = getOption("pt2_effect_format")) {
  if (is.null(fmt)) fmt <- c(cli::col_green("%X%02X"), cli::col_silver("%X%02X"))
  x <- as.raw.pt2command(x) |> as.integer()
  fmt <- ifelse(all(x == 0L), fmt[[2]], fmt[[1]])
  sprintf(fmt, x[[1]], x[[2]])
}

.cell_format <- function(x, padding = " ", empty_char = "-",
                         fmt = getOption("pt2_cell_format"), as.raw.fun) {
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
  as.raw.fun(x, compact = FALSE) |>
    pt_rawcell_as_char_(
      as.character(padding),
      as.character(empty_char),
      as.list(fmt))
}

#' @method length pt2celllist
#' @rdname s3methods
#' @export
length.pt2celllist <- function(x, ...) {
  fct  <- ifelse(attributes(x)$compact_notation, 4L, pt_cell_bytesize())
  fct  <- ifelse(typeof(x) == "raw", fct, 1L)
  length(unclass(x))/fct
}

#' @method length pt2command
#' @rdname s3methods
#' @export
length.pt2command <- function(x, ...) {
  fct  <- ifelse(typeof(x) == "raw", 2L, 1L)
  length(unclass(x))/fct
}
