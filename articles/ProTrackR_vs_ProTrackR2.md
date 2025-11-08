# ProTrackR vs ProTrackR2

## ProTracker2 and its predecessor

The predecessor to this package
([ProTrackR](https://pepijn-devries.github.io/ProTrackR/)) was entirely
programmed in R. Although technically possible, it was challenging and
slow with recursive algorithms. The new version is a complete overhaul
in C/C++, based on Olav Sørensen’s [ProTracker
clone](https://16-bits.org/pt2.php). With it, came some design changes
which are worth mentioning here.

## Overview of differences

The table below summarises the differences between ProTrackR2 and its
predecessor.

| Feature                             | ProTrackR                         | ProTrackR2                         |
|-------------------------------------|-----------------------------------|------------------------------------|
| [Effect commands](#effect-commands) | Limited set implemented           | All PT2.3d effects implemented     |
| [Infrastructure](#infrastructure)   | R script                          | Compiled C/C++                     |
| [File readers](#file-readers)       | Optimized for format preservation | Optimized for PT2.3d compatibility |
| [Audio output](#audio-output)       | tuneR S4 Wave class               | audio s3 audioSample class         |
| OpenMPT test cases                  | Passes 6 out of 12 selected tests | Passes at least 17 of the 23 tests |

## Effect commands

ProTracker uses specific codes to apply certain effects or position
jumps. `ProTrackR` implements only a subset of these effects, whereas
`ProTrackR2` has implemented all ProTracker compatible effects.

The predecessor only partly implemented arpeggio and setting finetune.
It did not implement glissando, sample filtering (E8) and loop reversal
(EF). All these effects are implemented in the current package.

For a full overview of effect commands see
[`vignette("effect_commands")`](https://pepijn-devries.github.io/ProTrackR2/articles/effect_commands.md).

## Infrastructure

By switching to `C` and `C++` compiled code, the new package gained a
significant performance boost (see [benchmark results](#benchmark)).
Where the in the predecessor the module was represented by a vector of
`raw` data, an `externalptr` to a `C struct` is used in the current
package. This required a slightly different approach to handle these
objects. In order to avoid confusion about the syntax, it was completely
redesigned in the successor.

### Class type objects

ProTrackR uses [s4 class](https://adv-r.hadley.nz/s4.html) objects. It
has elegant semantics and due to its strict nature, has a lot of
build-in safety mechanisms. But consequently, they are not
straightforward and easy to work with.

Therefore, in ProTrackR2 I chose to work with the [s3
class](https://adv-r.hadley.nz/s3.html) objects. They are easier to work
with and perhaps more intuitive to most R users. If you are coming to
ProTrackR2 from ProTrackR, here is an overview of equivalent classes:

[TABLE]

These different class styles also have consequences for you workflow.
Below are some examples of how to achieve similar operations using the
different packages.

``` r
library(ProTrackR)
library(ProTrackR2)

## *Old* way of creating a blank module:
mod_old <- new("PTModule")
## *New* way of creating a blank module:
mod_new <- pt2_new_mod("my mod")

## *Old* reading a module:
mod_old <- read.module(pt2_demo())
## *New* way of creating a blank module:
mod_new <- pt2_read_mod(pt2_demo())

## *Old* way of getting a sample
samp_old <- PTSample(mod_old, 1L)
## *New* way of getting a sample
samp_new <- mod_new$samples[[1L]]

## *Old* way of getting a pattern
patt_old <- PTPattern(mod_old, 1L)
## *New* way of getting a pattern
patt_new <- mod_new$patterns[[1L]]

## *Old* way of getting a cell
cell_old <- PTCell(mod_old, 1L, 1L, 1L)
## *New* way of getting a cell
##
##   First cell in cell-list ---------------+
##   First column in pattern ----------+    |
##      First row in pattern ------+   |    |
##             First pattern -+    |   |    |
##                            |    |   |    |
##                            V    V   V    V
cell_new <- mod_new$patterns[[1L]][1L, 1L][[1L]]

print(cell_old)
#> [1] "C-3 01 A08"
print(cell_new)
#> C-3 01 A08
```

### Benchmark

A benchmark test where the same module (the one provided with this
package) is rendered with both `ProTrackR` and `ProTrackR2`. The
settings for both tests were similar and performed on the same system
and repeated 10 times. On average `ProTrackR2` renders 8.8 times faster
than `ProTrackR`.

## File readers

While reading ProTracker modules, the predecessor preserved the data in
the file. It only modified / fixed data when requested by the user. The
current package will always sanitise data while reading it, making it
compatible with ProTracker 2.3d. The current reader is also a bit more
flexible and allows to read more exotic formats. It even allows you to
read files compressed with PowerPacker.

### Alternatives

If you want even more flexibility, check out the [openmpt
package](https://pepijn-devries.github.io/openmpt/). It uses
[libopenmpt](https://lib.openmpt.org/) to read and play modules. This
library has a more extensive set of supported [file
formats](https://wiki.openmpt.org/Manual:_Module_formats). The downside
is that it does not allow you to modify or save modules.

## Audio output

The predecessor used [tuneR](https://cran.r-project.org/package=tuneR)
objects to store rendered audio. In the current package we use
[audio](https://cran.r-project.org/package=tuneR) objects. This switch
was made as the `S3` class objects from ‘audio’ are easier to handle
than the stricter and formal `S4` class objects from ‘tuneR’. If you
wish to use the advanced features from the ‘tuneR’ package, this is
still possible as both formats can be converted relatively easy.

``` r
## Load demo module
mod <- pt2_read_mod(pt2_demo())

## render 'audioSample' object
mod_audio <- pt2_render(mod)

## Convert from 'audio::audioSample' to 'tuneR::Wave' object:
if (requireNamespace("tuneR")) {
  mod_tuneR <-
    tuneR::Wave(
      left      = as.integer(2^15*unclass(mod_audio[1,])),
      right     = as.integer(2^15*unclass(mod_audio[2,])),
      samp.rate = attributes(mod_audio)$rate,
      bit       = attributes(mod_audio)$bits
    )
}
```
