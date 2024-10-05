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

#' Create a new (empty) Protracker module
#' 
#' Creates an empty Protracker module, it is returned as a `pt2mod` class object.
#' @param name Name for the new module. It will be truncated if longer than 20 characters.
#' @param ... Ignored
#' @returns A `pt2mod` class module, with no samples and one empty pattern.
#' @examples
#' mod <- pt2_new_mod("my_song")
#' @author Pepijn de Vries
#' @export
pt2_new_mod <- function(name, ...) {
  new_mod_(name)
}

#' Read and write Protracker audio samples
#' 
#' Functions to read and write Protracker audio samples. Reading is supported
#' for common types of WAV, IFF and AIFF files. Writing is supported for
#' WAV and IFF files.
#' @param sample An object of class `pt2samp`.
#' @param file Filename of the file to read from or write to. For `pt2_write_sample()`
#' the file extension will be used to determine which file format to write.
#' @param ... Ignored
#' @returns `pt2_read_sample()` returns a `pt2samp` class object when successful.
#' `pt_write_sample()` returns `NULL` invisibly.
#' @examples
#' mod <- pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
#' my_sample <- pt2_sample(mod, 1L)
#' my_sample_file <- tempfile(fileext = ".iff")
#' pt2_write_sample(my_sample, my_sample_file)
#' @author Pepijn de Vries
#' @rdname samp_io
#' @export
pt2_read_sample <- function(file, ...) {
  x <- raw()
  samp_name <- ""
  if (inherits(file, "connection")) {
    if (!summary(file)$mode %in% c("rb", "r+b"))
      stop("Need a readable binary connection")
  } else {
    samp_name <- basename(file) |> substr(1L, 21L)
    file <- file(file, "rb", ...)
  }
  repeat {
    chunk <- readBin(file, "raw", 1024*1024)
    x <- c(x, chunk)
    if (length(chunk) == 0) break
  }
  close(file)
  result <- open_samp_(x)
  if (attributes(result)$sample_info$text == "")
    attributes(result)$sample_info$text <- samp_name
  result
}

#' @rdname samp_io
#' @export
pt2_write_sample <- function(sample, file, ...) {
  suffix <- "RAW"
  if (inherits(file, "connection")) {
    if (!summary(file)$mode %in% c("wb", "w+b"))
      stop("Need a writable binary connection")
  } else {
    suffix <- regmatches(file, gregexpr("([^.]*)$", file)) |> toupper()
    file <- file(file, "wb", ...)
  }
  sample <- as.raw(sample) |> unclass()
  if (suffix %in% c("IFF", "AIFF", "WAV")) {
    sample <- sample_file_format_(sample, suffix)
  } else {
    sample <- as.raw(sample)
    warning("Unsupported file type, writing as raw.")
  }
  writeBin(sample |> as.raw(), file)
  close(file)
}