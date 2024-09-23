#' Render Protracker modules to a playable format
#' 
#' Renders a 16bit pulse-code modulation waveform from a Protracker module.
#' The rendered format can be played on a modern machine.
#' @param x The object to be rendered
#' @param duration Duration of the rendered output in seconds.
#' @param ... Ignored
#' @returns Rendered audio inheriting the [`audio::audioSample()`] class.
#' @examples
#' mod <- pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
#' aud <- pt2_render(mod)
#' @author Pepijn de Vries
#' @export
pt2_render <- function(x, duration = 120, ...) {
  UseMethod("pt2_render", x)
}

#' @rdname pt2_render
#' @name pt2_render
#' @export
pt2_render.pt2mod <- function(x, duration = 120, ...) {
  render_mod_(x, duration) |>
    matrix(nrow = 2, byrow = FALSE) |>
    audio::audioSample(rate = 48000)
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
#' @param duration Duration in seconds to play.
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
play.pt2mod <- function(x, duration = 120, ...) {
  x <- pt2_render(x, duration = duration, ...)
  rate <- attributes(x)$rate
  audio::play.audioSample(x, rate = rate)
}