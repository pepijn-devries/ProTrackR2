# Calculate the duration of the module

How long a module will play depends on several aspects such as the
length of the pattern sequence table
([`pt2_pattern_table()`](https://pepijn-devries.github.io/ProTrackR2/reference/mod_info.md),
[`pt2_length()`](https://pepijn-devries.github.io/ProTrackR2/reference/mod_info.md)),
the speed and tempo at which the patterns are defined, loops, pattern
breaks and delay effects. The duration in seconds of the module is
calculated by this function.

## Usage

``` r
pt2_duration(x, options = pt2_render_options(), position = 0L, ...)
```

## Arguments

- x:

  Object for which to determine the duration. Should be of class
  `pt2mod`.

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

  Ignored

## Value

The duration in seconds (as a `difftime` class object)

## Author

Pepijn de Vries

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())

pt2_duration(mod)
#> Time difference of 69.12204 secs
```
