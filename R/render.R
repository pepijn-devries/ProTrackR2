#' Render Protracker modules to a playable format
#' 
#' Renders a 16bit pulse-code modulation waveform from a Protracker module.
#' The rendered format can be played on a modern machine.
#' @param x The object to be rendered
#' @param duration Duration of the rendered output in seconds. When set to `NA`
#' the duration of the module is calculated and used for rendering.
#' @param options A list of options used for rendering the audio. Use
#' [`pt2_render_options()`] to obtain default options, or modify them.
#' @param position Starting position in the pattern sequence table (`pt2_pattern_table()`).
#' Should be a non negative value smaller than the mule length (`pt2_length()`).
#' @param ... Ignored
#' @returns Rendered audio inheriting the [`audio::audioSample()`] class.
#' @examples
#' mod <- pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
#' aud <- pt2_render(mod)
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

#' @importFrom audio play
#' @export play
#' @name play
#' @rdname play
NULL

#' Play a Protracker module
#' 
#' Renders a Protracker module as [`audio::audioSample()`] and plays it.
#'
#' @param x Object to be played.
#' @inheritParams pt2_render
#' @param ... Arguments passed to [`pt2_render()`].
#' @returns Returns an `[audio::`$.audioInstance`]` object which
#' allows you to control the playback (pause, resume, rewind).
#' @author Pepijn de Vries
#' @export
#' @name play
#' @rdname play
#' @examples
#' \dontrun{
#' mod <- pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
#' 
#' ## ctrl will contain the audioInstance that will let you control the audio playback:
#' ctrl <- play(mod)
#' }
#' @method play pt2mod
play.pt2mod <- function(x, duration = NA, options = pt2_render_options(), position = 0L, ...) {
  x <- pt2_render(x, duration = duration, options = options, position = position, ...)
  rate <- attributes(x)$rate
  audio::play.audioSample(x, rate = rate)
}

#' Retrieve options for rendering
#' 
#' Retrieve options for rendering Protracker modules. See also
#' [`pt2_render()`].
#'
#' @param ... Specify custom options.
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
    led_filter = FALSE
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
#' mod <- pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
#' 
#' pt2_duration(mod)
#' @export
pt2_duration <- function(x, options = pt2_render_options(), position = 0L, ...) {
  dur <- mod_duration(x, options, as.integer(position)) |>
    as.difftime(units = "secs")
  return(dur)
}