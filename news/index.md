# Changelog

## ProTrackR2 v0.1.0.0001

- Deprecated
  [`pt2_new_pattern()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_new_pattern.md)
  in favour of
  [`pt2_pattern()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_pattern.md)
- Fix for undefined behaviour

## ProTrackR2 v0.1.0

CRAN release: 2025-11-11

- Implemented
  [`as_pt2cell()`](https://pepijn-devries.github.io/ProTrackR2/reference/as_pt2cell.md)
  and
  [`as_pt2celllist()`](https://pepijn-devries.github.io/ProTrackR2/reference/as_pt2cell.md)
- Implemented replacement function `pt2_pattern_table<-()` and
  `pt2_name<-()`
- Implemented several new
  [`play()`](https://pepijn-devries.github.io/ProTrackR2/reference/play.md)
  and
  [`pt2_render()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_render.md)
  methods
- Implemented sample handling functions:
  - [`pt2_finetune()`](https://pepijn-devries.github.io/ProTrackR2/reference/sample_properties.md)
  - [`pt2_volume()`](https://pepijn-devries.github.io/ProTrackR2/reference/sample_properties.md)
  - [`pt2_loop_start()`](https://pepijn-devries.github.io/ProTrackR2/reference/sample_properties.md)
    and
    [`pt2_loop_length()`](https://pepijn-devries.github.io/ProTrackR2/reference/sample_properties.md)
  - [`pt2_is_looped()`](https://pepijn-devries.github.io/ProTrackR2/reference/sample_properties.md)
  - `pt_sample_to_audio()`
- Fixes in (amongst others):
  - Selection and assignment operators
  - Sample read and write functions
- Added testthat workflow
- Added codecov workflow

## ProTrackR2 v0.0.5

CRAN release: 2024-11-09

- Added functions to extract or replace notes, instruments and/or
  effects in cells
- Added vignette about effect commands
- Added fixes for problems detected by clang-UBSAN, gcc-UBSAN, LTO and
  valgrind

## ProTrackR2 v0.0.4

CRAN release: 2024-10-28

- Added select and assignment operators for patterns and samples
- Added validation routine for samples
- Allow user to set the length of a module
- Extended documentation
- Several minor bug fixes

## ProTrackR2 v0.0.3

- Calculate song duration
- Start at module positions other than 0
- Custom starting speed and/or tempo
- Format patterns conform OpenMPT specs

## ProTrackR2 v0.0.1

- Initial commit, which can:
  - load and save ProTracker modules
  - load and save audio samples
  - render audio from ProTracker modules
  - play ProTracker modules
