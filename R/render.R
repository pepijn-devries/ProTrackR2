#' Render ProTracker modules and other objects to a playable format
#' 
#' Renders a 16bit pulse-code modulation waveform from a ProTracker module.
#' The rendered format can be played on a modern machine.
#' @param x The object to be rendered
#' @param duration Duration of the rendered output in seconds. When set to `NA`
#' the duration of the module is calculated and used for rendering.
#' @param options A list of options used for rendering the audio. Use
#' [`pt2_render_options()`] to obtain default options, or modify them.
#' @param position Starting position in the pattern sequence table (`pt2_pattern_table()`).
#' Should be a non negative value smaller than the mule length (`pt2_length()`).
#' @param note Note to be played when `x` is a `pt2samp` class object. Defaults
#' to `"C-3"`.
#' @param samples When rendering or playing patterns (or elements of it), samples are needed
#' to interpret the pattern. Pass the samples as a sample list (class `pt2samplist`).
#' @param ... Ignored
#' @returns Rendered audio inheriting the [`audio::audioSample()`] class.
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' aud <- pt2_render(mod)
#' aud_samp <- pt2_render(mod$samples[[1]])
#' @author Pepijn de Vries
#' @export
pt2_render <- function(x, duration = NA, options = pt2_render_options(), ...) {
  UseMethod("pt2_render", x)
}

#' @rdname pt2_render
#' @name pt2_render
#' @export
pt2_render.pt2mod <- function(x, duration = NA, options = pt2_render_options(), position = 0L, ...) {
  render_mod_(x, as.numeric(duration), options, as.integer(position)) |>
    matrix(nrow = 2, byrow = FALSE) |>
    audio::audioSample(rate = options$sample_rate)
}

#' @rdname pt2_render
#' @name pt2_render
#' @export
pt2_render.pt2samp <- function(x, duration = 5, options = pt2_render_options(), note = "C-3", ...) {
  mod <- pt2_new_mod("render.samp")
  mod$samples[[1]] <- x
  mod$patterns[[1]][1,c(1,2)] <- paste0(note, " 01 F00")
  pt2_render(mod, duration, options, 0L, ...)
}

#' @rdname pt2_render
#' @name pt2_render
#' @export
pt2_render.pt2patlist <- function(x, duration = NA, options = pt2_render_options(), samples, ...) {
  mod <- pt2_new_mod("render.patlist")
  mod$samples <- samples
  idx <- seq_len(length(x))
  mod$patterns[idx] <- x
  pt2_length(mod) <- length(x)
  pt2_pattern_table(mod)[idx] <- idx - 1L
  pt2_render(mod, duration, options, 0L, ...)
}

#' @rdname pt2_render
#' @name pt2_render
#' @export
pt2_render.pt2pat <- function(x, duration = NA, options = pt2_render_options(), samples, ...) {
  mod <- pt2_new_mod("render.pat")
  mod$samples <- samples
  mod$patterns[[1]] <- x
  pt2_render(mod, duration, options, 0L, ...)
}

#' @rdname pt2_render
#' @name pt2_render
#' @export
pt2_render.pt2celllist <- function(x, duration = 5, options = pt2_render_options(), samples, ...) {
  d <- attr(x, "celldim")
  mod <- pt2_new_mod("render.pat")
  mod$samples <- samples
  if (is.null(d)) {
    mod$patterns[[1]][1:64, 1:4][seq_len(length(x))] <- x
  } else {
    mod$patterns[[1]][seq_len(d[[1]]), seq_len(d[[2]])] <- x
  }
  pt2_render(mod, duration, options, 0L, ...)
}

#' @rdname pt2_render
#' @name pt2_render
#' @export
pt2_render.pt2cell <- function(x, duration = 5, options = pt2_render_options(), samples, ...) {
  mod <- pt2_new_mod("render.pat")
  mod$samples <- samples
  mod$patterns[[1]][1,1][[1]] <- x
  pt2_render(mod, duration, options, 0L, ...)
}

#' Retrieve options for rendering
#' 
#' Retrieve options for rendering ProTracker modules. See also
#' [`pt2_render()`].
#'
#' @param ... Specify custom options.
#' @returns Returns a named `list` of options that can be used for rendering
#' ProTracker modules (see [`pt2_render()`] and [`play()`]).
#' It contains the following elements:
#'   * `sample_rate`: an integer value specifying the sample rate of the output in Hz.
#'   * `stereo_separation`: an integer percentage determining how much the
#'     tracker channels will be separated to the left and right stereo output channels.
#'   * `amiga_filter`: a `character` string specifying the hardware filter to be emulated.
#'     Can be `"A500"` for emulating Amiga 500 hardware filters, or `"A1200"` for emulating
#'     Amiga 1200 hardware filters.
#'   * `speed`: An integer value specifying the initial speed of the module measured in 'ticks'
#'     per row. Should be in range of `1` and `31`.
#'   * `tempo`: An integer value specifying the initial tempo of the module. When speed is set
#'     to `6`, it measures the tempo as beats per minute. Should be in the range of `32` and `255`
#'   * `led_filter`: A `logical` value specifying the state of the hardware LED filter to be emultated.
#'   * `timing_mode`: on the original Commodore Amiga timing in tracker modules could be
#'     handled using different approaches. The first is the 'vertical blanking' method, were timing
#'     was based on each time the monitor blanks (before being redrawn). This method thus depends
#'     on the monitor that was used. PAL monitors operated at approximately 50 Hz, wereas NTCS monitors
#'     used 60 Hz. Alternatively, the Complex Interface Adapter (CIA) offer hardware-level timing and
#'     was system independent. You can set the timing mode by specifying `"cia"` (default) here, or
#'     `"vblank"` (currently, only PAL is supported).
#' @author Pepijn de Vries
#' @examples
#' pt2_render_options(stereo_separation = 100)
#' @export
pt2_render_options <- function(...) {
  result <- getOption("pt2_render")
  if (is.null(result)) result <- list()
  defaults <- list(
    sample_rate = 44100L,
    stereo_separation = 20L,
    amiga_filter = "A500",
    speed = 6L,
    tempo = 125L,
    led_filter = FALSE,
    timing_mode = "cia"
  )
  for (i in names(defaults)) {
    if (is.null(result[[i]])) result[[i]] <- defaults[[i]]
  }
  repl <- list(...)
  for (i in names(repl)) {
    result[[i]] <- repl[[i]]
  }
  return (result)
}

#' Calculate the duration of the module
#' 
#' How long a module will play depends on several aspects such as
#' the length of the pattern sequence table (`pt2_pattern_table()`, `pt2_length()`),
#' the speed and tempo at which the patterns are defined,
#' loops, pattern breaks and delay effects. The duration in seconds of the
#' module is calculated by this function.
#' @inheritParams pt2_render
#' @param x Object for which to determine the duration. Should be of class `pt2mod`.
#' @returns The duration in seconds (as a `difftime` class object)
#' @author Pepijn de Vries
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' 
#' pt2_duration(mod)
#' @export
pt2_duration <- function(x, options = pt2_render_options(), position = 0L, ...) {
  dur <- mod_duration(x, options, as.integer(position)) |>
    as.difftime(units = "secs")
  return(dur)
}
