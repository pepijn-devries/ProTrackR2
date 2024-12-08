test_that("Module renders as expected", {
  snap <- "intro.mp3"
  announce_snapshot_file(snap)
  mod <- pt2_read_mod(pt2_demo())
  tempwav <- tempfile(fileext = ".wav")
  tempmp3 <- tempfile(fileext = ".mp3")
  rndr <- pt2_render(mod, 10)
  audio::save.wave(rndr, tempwav)
  av::av_audio_convert(tempwav, tempmp3, verbose = FALSE)
  expect_snapshot_file(tempmp3, snap)
})