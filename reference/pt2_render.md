# Render ProTracker modules and other objects to a playable format

Renders a 16bit pulse-code modulation waveform from a ProTracker module.
The rendered format can be played on a modern machine.

## Usage

``` r
pt2_render(x, duration = NA, options = pt2_render_options(), ...)

# S3 method for class 'pt2mod'
pt2_render(
  x,
  duration = NA,
  options = pt2_render_options(),
  position = 0L,
  ...
)

# S3 method for class 'pt2samp'
pt2_render(x, duration = 5, options = pt2_render_options(), note = "C-3", ...)

# S3 method for class 'pt2patlist'
pt2_render(x, duration = NA, options = pt2_render_options(), samples, ...)

# S3 method for class 'pt2pat'
pt2_render(x, duration = NA, options = pt2_render_options(), samples, ...)

# S3 method for class 'pt2celllist'
pt2_render(x, duration = 5, options = pt2_render_options(), samples, ...)

# S3 method for class 'pt2cell'
pt2_render(x, duration = 5, options = pt2_render_options(), samples, ...)
```

## Arguments

- x:

  The object to be rendered

- duration:

  Duration of the rendered output in seconds. When set to `NA` the
  duration of the module is calculated and used for rendering.

- options:

  A list of options used for rendering the audio. Use
  [`pt2_render_options()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_render_options.md)
  to obtain default options, or modify them.

- ...:

  Ignored

- position:

  Starting position in the pattern sequence table
  ([`pt2_pattern_table()`](https://pepijn-devries.github.io/ProTrackR2/reference/mod_info.md)).
  Should be a non negative value smaller than the mule length
  ([`pt2_length()`](https://pepijn-devries.github.io/ProTrackR2/reference/mod_info.md)).

- note:

  Note to be played when `x` is a `pt2samp` class object. Defaults to
  `"C-3"`.

- samples:

  When rendering or playing patterns (or elements of it), samples are
  needed to interpret the pattern. Pass the samples as a sample list
  (class `pt2samplist`).

## Value

Rendered audio inheriting the
[`audio::audioSample()`](https://rdrr.io/pkg/audio/man/audioSample.html)
class.

## Author

Pepijn de Vries

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())
aud <- pt2_render(mod)
aud_samp <- pt2_render(mod$samples[[1]])
```
