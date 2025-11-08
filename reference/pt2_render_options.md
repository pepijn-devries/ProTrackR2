# Retrieve options for rendering

Retrieve options for rendering ProTracker modules. See also
[`pt2_render()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_render.md).

## Usage

``` r
pt2_render_options(...)
```

## Arguments

- ...:

  Specify custom options.

## Value

Returns a named `list` of options that can be used for rendering
ProTracker modules (see
[`pt2_render()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_render.md)
and
[`play()`](https://pepijn-devries.github.io/ProTrackR2/reference/play.md)).
It contains the following elements:

- `sample_rate`: an integer value specifying the sample rate of the
  output in Hz.

- `stereo_separation`: an integer percentage determining how much the
  tracker channels will be separated to the left and right stereo output
  channels.

- `amiga_filter`: a `character` string specifying the hardware filter to
  be emulated. Can be `"A500"` for emulating Amiga 500 hardware filters,
  or `"A1200"` for emulating Amiga 1200 hardware filters.

- `speed`: An integer value specifying the initial speed of the module
  measured in 'ticks' per row. Should be in range of `1` and `31`.

- `tempo`: An integer value specifying the initial tempo of the module.
  When speed is set to `6`, it measures the tempo as beats per minute.
  Should be in the range of `32` and `255`

- `led_filter`: A `logical` value specifying the state of the hardware
  LED filter to be emultated.

- `timing_mode`: on the original Commodore Amiga timing in tracker
  modules could be handled using different approaches. The first is the
  'vertical blanking' method, were timing was based on each time the
  monitor blanks (before being redrawn). This method thus depends on the
  monitor that was used. PAL monitors operated at approximately 50 Hz,
  wereas NTCS monitors used 60 Hz. Alternatively, the Complex Interface
  Adapter (CIA) offer hardware-level timing and was system independent.
  You can set the timing mode by specifying `"cia"` (default) here, or
  `"vblank"` (currently, only PAL is supported).

## Author

Pepijn de Vries

## Examples

``` r
pt2_render_options(stereo_separation = 100)
#> $sample_rate
#> [1] 44100
#> 
#> $stereo_separation
#> [1] 100
#> 
#> $amiga_filter
#> [1] "A500"
#> 
#> $speed
#> [1] 6
#> 
#> $tempo
#> [1] 125
#> 
#> $led_filter
#> [1] FALSE
#> 
#> $timing_mode
#> [1] "cia"
#> 
```
