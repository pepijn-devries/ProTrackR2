# Play a ProTrackR2 class objects as audio

Renders a ProTrackR2 class object as
[`audio::audioSample()`](https://rdrr.io/pkg/audio/man/audioSample.html)
and plays it.

## Usage

``` r
# S3 method for class 'pt2mod'
play(x, duration = NA, options = pt2_render_options(), position = 0L, ...)

# S3 method for class 'pt2samp'
play(x, duration = 5, options = pt2_render_options(), note = "C-3", ...)

# S3 method for class 'pt2patlist'
play(x, duration = NA, options = pt2_render_options(), samples, ...)

# S3 method for class 'pt2pat'
play(x, duration = NA, options = pt2_render_options(), samples, ...)

# S3 method for class 'pt2celllist'
play(x, duration = 5, options = pt2_render_options(), samples, ...)

# S3 method for class 'pt2cell'
play(x, duration = 5, options = pt2_render_options(), samples, ...)
```

## Arguments

- x:

  Object to be played.

- duration:

  Duration of the rendered output in seconds. When set to `NA` the
  duration of the module is calculated and used for rendering.

- options:

  A list of options used for rendering the audio. Use
  [`pt2_render_options()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_render_options.md)
  to obtain default options, or modify them.

- position:

  Starting position in the pattern sequence table
  ([`pt2_pattern_table()`](https://pepijn-devries.github.io/ProTrackR2/reference/mod_info.md)).
  Should be a non negative value smaller than the mule length
  ([`pt2_length()`](https://pepijn-devries.github.io/ProTrackR2/reference/mod_info.md)).

- ...:

  Arguments passed to
  [`pt2_render()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_render.md).

- note:

  Note to be played when `x` is a `pt2samp` class object. Defaults to
  `"C-3"`.

- samples:

  When rendering or playing patterns (or elements of it), samples are
  needed to interpret the pattern. Pass the samples as a sample list
  (class `pt2samplist`).

## Value

Returns an `[audio::`\$.audioInstance`]` object which allows you to
control the playback (pause, resume, rewind).

## Author

Pepijn de Vries

## Examples

``` r
if (interactive()) {
  mod <- pt2_read_mod(pt2_demo())

  ## ctrl will contain the audioInstance that will let
  ## you control the audio playback:
  ctrl <- play(mod)

  ## You can also play individual samples
  samp <- mod$samples[[3]]
  play(samp, note = "C-2")
  play(samp, note = "E-2")
  play(samp, note = "G-2")
  
  ## As well as an individual pattern
  play(mod$patterns[[1]], samples = mod$samples)
  
  ## Or a subset of a pattern
  play(mod$patterns[[1]][17:32, 1:2], samples = mod$samples)
  
  ## Or even an individual cell
  play(mod$patterns[[1]][1, 1][[1]], samples = mod$samples)
}
```
