#' Read and write Protracker modules
#' 
#' Functions to read and write Protracker module. The read function will
#' read a number of mod files that are compatible with Protracker, this
#' includes files compressed with PowerPacker (PP). The write function
#' will only write modules conform Protracker specifications.
#' @param mod An object of class `pt2mod`.
#' @param file Filename of the file to read from or write to.
#' @param ... Ignored
#' @returns `pt2_read_mod()` returns a `pt2mod` class object when successful.
#' `pt_write_mod()` returns `NULL` invisibly.
#' @examples
#' mod <- pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
#' @author Pepijn de Vries
#' @rdname mod_io
#' @export
pt2_read_mod <- function(file, ...) {
  x <- raw()
  if (inherits(file, "connection")) {
    if (!summary(file)$mode %in% c("rb", "r+b"))
      stop("Need a readable binary connection")
  } else {
    file <- file(file, "rb", ...)
  }
  repeat {
    chunk <- readBin(file, "raw", 1024*1024)
    x <- c(x, chunk)
    if (length(chunk) == 0) break
  }
  close(file)
  # 2108 is minimum size of modules
  if (length(x) >= 2108) open_mod_(x) else stop("Corrupt mod file")
}

#' @rdname mod_io
#' @export
pt2_write_mod <- function(mod, file, ...) {
  if (inherits(file, "connection")) {
    if (!summary(file)$mode %in% c("wb", "w+b"))
      stop("Need a writable binary connection")
  } else {
    file <- file(file, "wb", ...)
  }
  dat <- mod_as_raw_(mod)
  writeBin(dat, file)
  close(file)
  return (invisible(NULL))
}
