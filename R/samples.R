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
  ) |>
    as.raw()
}

#' @rdname mod_info
#' @include mod_info.R
#' @method pt2_name pt2samp
#' @export
pt2_name.pt2samp <- function(x, ...) {
  attributes(x)$sample_info$text
}

#' @rdname mod_info
#' @include mod_info.R
#' @method pt2_name<- pt2samp
#' @export
`pt2_name<-.pt2samp` <- function(x, ..., value) {
  attr(x, "sample_info")$text <- substr(value, 0, 22)
  validate_sample_raw_(x)
  x
}

#' @rdname mod_info
#' @include mod_info.R
#' @method pt2_name pt2samplist
#' @export
pt2_name.pt2samplist <- function(x, ...) {
  result = list()
  for (i in seq_len(length(x))) {
    result[[i]] <- pt2_name(x[[i]])
  }
  unlist(result)
}

#' @rdname mod_info
#' @include mod_info.R
#' @method pt2_name<- pt2samplist
#' @export
`pt2_name<-.pt2samplist` <- function(x, ..., value) {
  if (length(x) != length(value)) stop("Replacement should have the same length as `x`")
  for (i in seq_len(length(x))) {
    pt2_name(x[[i]]) <- value[i]
  }
  x
}

#' @rdname mod_info
#' @export
pt2_n_sample <- function(mod, ...) {
  count = 0
  for (i in 0L:30L) {
    if (mod_sample_info_(mod, i)$length > 0) count = count + 1
  }
  count
}

#' Get or set ProTracker sample properties
#'
#' Get or set properties of a ProTracker sample. See 'details' section
#' for available properties and associated functions.
#' 
#' ProTracker audio samples hold some meta data that affect their
#' playback. The following functions can be used to get or set these
#' properties:
#' 
#'  * `pt2_finetune()`: When a sample is out of tune, you can use the
#'    use the 'finetune' value to tune the sample. It is an integer value
#'    ranging between -8 and +7.
#'  * `pt2_volume()`: An integer value to adjust the sample volume. It ranges
#'    between 0 (silent) to 64 (maximum volume).
#'  * `pt2_loop_start()` and `pt2_loop_length()`: defines if a sample is looped.
#'    The loop start is defined as an even integer indicating the sample index
#'    (zero based) where the loop should start. The loop length is the number
#'    of samples, over which to loop. The loop length should also be an even
#'    number greater than 0. The sum of the loop start and the loop length
#'    should never be larger than the sample's length.
#'  * `pt2_is_looped()`: a `logical` value, indicating if the sample is looped.
#'    If you set the loop status to `TRUE` when the sample is not looped yet,
#'    the new loop will start at index 0, and will have a length equal to the
#'    sample length.
#' @param sample A module sample of class `pt2samp`.
#' @param ... Ignored
#' @param value Replacement value for the sample property
#' @returns Returns an `integer` or `logical` value. Depending on the
#' called function. See the detail section for specifics.
#' @seealso [pt2_name()]
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' 
#' pt2_finetune(mod$samples[[1]])
#' pt2_finetune(mod$samples[[1]]) <- -8L
#' 
#' pt2_volume(mod$samples[[2]])
#' pt2_volume(mod$samples[[2]]) <- 64L
#' 
#' pt2_loop_start(mod$samples[[1]])
#' pt2_loop_start(mod$samples[[1]]) <- 400L
#' 
#' pt2_loop_length(mod$samples[[1]])
#' pt2_loop_length(mod$samples[[1]]) <- 274L
#' 
#' pt2_is_looped(mod$samples[[2]])
#' pt2_is_looped(mod$samples[[2]]) <- TRUE
#' @rdname sample_properties
#' @export
pt2_finetune <- function(sample, ...) {
  return(attr(sample, "sample_info")$fineTune)
}

#' @rdname sample_properties
#' @export
`pt2_finetune<-` <- function(sample, ..., value) {
  attr(sample, "sample_info")$fineTune <- as.integer(value)
  validate_sample_raw_(sample)
  return(sample)
}

#' @rdname sample_properties
#' @export
pt2_volume <- function(sample, ...) {
  return(attr(sample, "sample_info")$volume)
}

#' @rdname sample_properties
#' @export
`pt2_volume<-` <- function(sample, ..., value) {
  attr(sample, "sample_info")$volume <- as.integer(value)
  validate_sample_raw_(sample)
  return(sample)
}

#' @rdname sample_properties
#' @export
pt2_loop_start <- function(sample, ...) {
  return(attr(sample, "sample_info")$loopStart)
}

#' @rdname sample_properties
#' @export
`pt2_loop_start<-` <- function(sample, ..., value) {
  attr(sample, "sample_info")$loopStart <- as.integer(value - (value %% 2))
  validate_sample_raw_(sample)
  return(sample)
}

#' @rdname sample_properties
#' @export
pt2_loop_length <- function(sample, ...) {
  return(attr(sample, "sample_info")$loopLength)
}

#' @rdname sample_properties
#' @export
`pt2_loop_length<-` <- function(sample, ..., value) {
  attr(sample, "sample_info")$loopLength <- as.integer(value - (value %% 2L))
  validate_sample_raw_(sample)
  return(sample)
}

#' @rdname sample_properties
#' @export
pt2_is_looped <- function(sample, ...) {
  si <- attr(sample, "sample_info")
  return(!(si$loopStart == 0L && si$loopLength == 2L))
}

#' @rdname sample_properties
#' @export
`pt2_is_looped<-` <- function(sample, ..., value) {
  if (!is.logical(value) || length(value) != 1)
    stop("Replacement value needs to be a single logical value")
  is_looped <- pt2_is_looped(sample)
  if (is_looped && !value) {
    attr(sample, "sample_info")$loopStart <- 0L
    attr(sample, "sample_info")$loopLength <- 2L
  }
  if (!is_looped && value) {
    attr(sample, "sample_info")$loopStart <- 0L
    attr(sample, "sample_info")$loopLength <- attr(sample, "sample_info")$length
  }
  validate_sample_raw_(sample)
  return(sample)
}

#' Coerce ProTracker sample to audio sample
#'
#' Coerce a sample from a ProTracker module to an `audio` package
#' sample. Note that this function differs from [`pt2_render()`] as
#' it will not mimic Commodore Amiga hardware. It will just
#' pass the pure sample data.
#' @param sample A ProTracker sample of class `pt2samp`.
#' @param note A character representing a note (`"C-3"` by default).
#' It is used to calculate the playback rate for the sample.
#' @param finetune An `integer` fine tune value (between -8 and 7). Used
#' to tune the sample's note.
#' @param loop An `integer` value indicating how often the sample
#' should be looped (if it is looped). It will be ignored when `loop` has
#' a value of zero (or less), or when the sample is not looped
#' (see [`pt2_is_looped()`]).
#' @param options Options used for calculating play back rate. See
#' [`pt2_render_options()`] for all available options. But note that
#' not all options affect the play back rate.
#' @param ... Ignored
#' @returns Returns an audio sample of class [`audio::audioSample()`].
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' aud <- pt2_sample_to_audio(mod$samples[[1]])
#' @export
pt2_sample_to_audio <- function(
    sample, note = "C-3", finetune = 0, options = pt2_render_options(),
    loop = 20L, ...) {
  per <- pt2_note_to_period(note, "-", finetune)
  sample_freq <- pt_get_PAL_hz() / per
  sample_info <- attributes(sample)$sample_info
  is_looped   <- pt2_is_looped(sample)
  sample <- -1 + (as.integer(sample) + 128L) / 127.5
  attributes(sample) <- NULL
  idx <- length(sample)
  if (is_looped && loop > 0L) {
    loop_idx <- with(sample_info, loopStart + seq_len(loopLength))
    loop_idx <- rep(loop_idx, times = loop)
    idx <- c(seq_len(sample_info$loopStart),
             loop_idx)
  }
  sample <- sample[idx]
  audio::audioSample(sample, rate = sample_freq)
}