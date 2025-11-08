# Coerce ProTracker sample to audio sample

Coerce a sample from a ProTracker module to an `audio` package sample.
Note that this function differs from
[`pt2_render()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_render.md)
as it will not mimic Commodore Amiga hardware. It will just pass the
pure sample data.

## Usage

``` r
pt2_sample_to_audio(
  sample,
  note = "C-3",
  finetune = 0,
  options = pt2_render_options(),
  loop = 20L,
  ...
)
```

## Arguments

- sample:

  A ProTracker sample of class `pt2samp`.

- note:

  A character representing a note (`"C-3"` by default). It is used to
  calculate the playback rate for the sample.

- finetune:

  An `integer` fine tune value (between -8 and 7). Used to tune the
  sample's note.

- options:

  Options used for calculating play back rate. See
  [`pt2_render_options()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_render_options.md)
  for all available options. But note that not all options affect the
  play back rate.

- loop:

  An `integer` value indicating how often the sample should be looped
  (if it is looped). It will be ignored when `loop` has a value of zero
  (or less), or when the sample is not looped (see
  [`pt2_is_looped()`](https://pepijn-devries.github.io/ProTrackR2/reference/sample_properties.md)).

- ...:

  Ignored

## Value

Returns an audio sample of class
[`audio::audioSample()`](https://rdrr.io/pkg/audio/man/audioSample.html).

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())
aud <- pt2_sample_to_audio(mod$samples[[1]])
```
