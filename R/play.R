#' Play a ProTrackR2 class objects as audio
#' 
#' Renders a ProTrackR2 class object as [`audio::audioSample()`] and plays it.
#'
#' @param x Object to be played.
#' @inheritParams pt2_render
#' @param ... Arguments passed to [`pt2_render()`].
#' @returns Returns an `[audio::`$.audioInstance`]` object which
#' allows you to control the playback (pause, resume, rewind).
#' @author Pepijn de Vries
#' @examples
#' if (interactive()) {
#'   mod <- pt2_read_mod(pt2_demo())
#' 
#'   ## ctrl will contain the audioInstance that will let
#'   ## you control the audio playback:
#'   ctrl <- play(mod)
#' 
#'   ## You can also play individual samples
#'   samp <- mod$samples[[3]]
#'   play(samp, note = "C-2")
#'   play(samp, note = "E-2")
#'   play(samp, note = "G-2")
#'   
#'   ## As well as an individual pattern
#'   play(mod$patterns[[1]], samples = mod$samples)
#'   
#'   ## Or a subset of a pattern
#'   play(mod$patterns[[1]][17:32, 1:2], samples = mod$samples)
#'   
#'   ## Or even an individual cell
#'   play(mod$patterns[[1]][1, 1][[1]], samples = mod$samples)
#' }
#' @importFrom audio play
#' @export play
#' @name play
#' @rdname play
#' @include render.R
NULL

#' @export
#' @name play
#' @rdname play
#' @method play pt2mod
play.pt2mod <- function(x, duration = NA, options = pt2_render_options(), position = 0L, ...) {
  x <- pt2_render(x, duration = duration, options = options, position = position, ...)
  rate <- attributes(x)$rate
  audio::play.audioSample(x, rate = rate)
}

#' @export
#' @name play
#' @rdname play
#' @method play pt2samp
play.pt2samp <- function(x, duration = 5, options = pt2_render_options(), note = "C-3", ...) {
  x <- pt2_render(x, duration = duration, options = options, note = note, ...)
  rate <- attributes(x)$rate
  audio::play.audioSample(x, rate = rate)
}

#' @export
#' @name play
#' @rdname play
#' @method play pt2patlist
play.pt2patlist <- function(x, duration = NA, options = pt2_render_options(), samples, ...) {
  x <- pt2_render(x, duration = duration, options = options, samples = samples, ...)
  rate <- attributes(x)$rate
  audio::play.audioSample(x, rate = rate)
}

#' @export
#' @name play
#' @rdname play
#' @method play pt2pat
play.pt2pat <- function(x, duration = NA, options = pt2_render_options(), samples, ...) {
  x <- pt2_render(x, duration = duration, options = options, samples = samples, ...)
  rate <- attributes(x)$rate
  audio::play.audioSample(x, rate = rate)
}

#' @export
#' @name play
#' @rdname play
#' @method play pt2celllist
play.pt2celllist <- function(x, duration = 5, options = pt2_render_options(), samples, ...) {
  x <- pt2_render(x, duration = duration, options = options, samples = samples, ...)
  rate <- attributes(x)$rate
  audio::play.audioSample(x, rate = rate)
}

#' @export
#' @name play
#' @rdname play
#' @method play pt2cell
play.pt2cell <- function(x, duration = 5, options = pt2_render_options(), samples, ...) {
  x <- pt2_render(x, duration = duration, options = options, samples = samples, ...)
  rate <- attributes(x)$rate
  audio::play.audioSample(x, rate = rate)
}
