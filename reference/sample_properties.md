# Get or set ProTracker sample properties

Get or set properties of a ProTracker sample. See 'details' section for
available properties and associated functions.

## Usage

``` r
pt2_finetune(sample, ...)

pt2_finetune(sample, ...) <- value

pt2_volume(sample, ...)

pt2_volume(sample, ...) <- value

pt2_loop_start(sample, ...)

pt2_loop_start(sample, ...) <- value

pt2_loop_length(sample, ...)

pt2_loop_length(sample, ...) <- value

pt2_is_looped(sample, ...)

pt2_is_looped(sample, ...) <- value
```

## Arguments

- sample:

  A module sample of class `pt2samp`.

- ...:

  Ignored

- value:

  Replacement value for the sample property

## Value

Returns an `integer` or `logical` value. Depending on the called
function. See the detail section for specifics.

## Details

ProTracker audio samples hold some meta data that affect their playback.
The following functions can be used to get or set these properties:

- `pt2_finetune()`: When a sample is out of tune, you can use the use
  the 'finetune' value to tune the sample. It is an integer value
  ranging between -8 and +7.

- `pt2_volume()`: An integer value to adjust the sample volume. It
  ranges between 0 (silent) to 64 (maximum volume).

- `pt2_loop_start()` and `pt2_loop_length()`: defines if a sample is
  looped. The loop start is defined as an even integer indicating the
  sample index (zero based) where the loop should start. The loop length
  is the number of samples, over which to loop. The loop length should
  also be an even number greater than 0. The sum of the loop start and
  the loop length should never be larger than the sample's length.

- `pt2_is_looped()`: a `logical` value, indicating if the sample is
  looped. If you set the loop status to `TRUE` when the sample is not
  looped yet, the new loop will start at index 0, and will have a length
  equal to the sample length.

## See also

[`pt2_name()`](https://pepijn-devries.github.io/ProTrackR2/reference/mod_info.md)

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())

pt2_finetune(mod$samples[[1]])
#> [1] 0
pt2_finetune(mod$samples[[1]]) <- -8L

pt2_volume(mod$samples[[2]])
#> [1] 43
pt2_volume(mod$samples[[2]]) <- 64L

pt2_loop_start(mod$samples[[1]])
#> [1] 762
pt2_loop_start(mod$samples[[1]]) <- 400L

pt2_loop_length(mod$samples[[1]])
#> [1] 276
pt2_loop_length(mod$samples[[1]]) <- 274L

pt2_is_looped(mod$samples[[2]])
#> [1] FALSE
pt2_is_looped(mod$samples[[2]]) <- TRUE
```
