test_that("Module renders as expected", {
  snap <- "intro.ogg"
  announce_snapshot_file(snap)
  mod <- pt2_read_mod(pt2_demo())
  tempwav <- tempfile(fileext = ".wav")
  tempogg <- tempfile(fileext = ".ogg")
  rndr <- pt2_render(mod, 10)
  audio::save.wave(rndr, tempwav)
  av::av_audio_convert(tempwav, tempogg, verbose = FALSE)
  ogg_compare <- function(old, new) {
    identical(av::read_audio_bin(old), av::read_audio_bin(new))
  }
  expect_snapshot_file(tempogg, snap, compare = ogg_compare)
})