ProTrackR2 v0.0.6.0012
-------------

 * Implemented `as_pt2cell()` and `as_pt2celllist()`
 * Implemented replacement function  `pt2_pattern_table<-()` and `pt2_name<-()`
 * Implemented several new `play()` and `pt2_render()` methods
 * Implemented sample handling functions:
   * `pt2_finetune()`
   * `pt2_volume()`
   * `pt2_loop_start()` and `pt2_loop_length()`
   * `pt2_is_looped()`
   * `pt_sample_to_audio()`
 * Added testthat workflow
 * Added codecov workflow
 * Fixes in (amongst others) selection and assignment operators

ProTrackR2 v0.0.5
-------------

 * Added functions to extract or replace
   notes, instruments and/or effects in cells
 * Added vignette about effect commands
 * Added fixes for problems detected by
   clang-UBSAN, gcc-UBSAN, LTO and valgrind

ProTrackR2 v0.0.4
-------------

 * Added select and assignment operators
   for patterns and samples
 * Added validation routine for samples
 * Allow user to set the length of a module
 * Extended documentation
 * Several minor bug fixes

ProTrackR2 v0.0.3
-------------

 * Calculate song duration
 * Start at module positions other than 0
 * Custom starting speed and/or tempo
 * Format patterns conform OpenMPT specs

ProTrackR2 v0.0.1
-------------

 * Initial commit, which can:
   * load and save ProTracker modules
   * load and save audio samples
   * render audio from ProTracker modules
   * play ProTracker modules
