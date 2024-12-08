test_that("Module renders as expected", {
  snap <- "intro.mp3"
  announce_snapshot_file(snap)
  mod <- pt2_read_mod(pt2_demo())
  tempwav <- tempfile(fileext = ".wav")
  tempmp3 <- tempfile(fileext = ".mp3")
  rndr <- pt2_render(mod, 10)
  audio::save.wave(rndr, tempwav)
  av::av_audio_convert(tempwav, tempmp3, verbose = FALSE)
  mp3_compare <- function(old, new) {
    bn <- av::read_audio_bin(new)
    bo <- av::read_audio_bin(old)
    result <- table(factor(abs((c(bn) - c(bo))/2^31) > 0.025, c("TRUE", "FALSE")))
    ## mp3 compression is not identical on all operating systems.
    ## Therefore, tolerate some difference.
    ## Less than 2.5% of the samples deviates more than 2.5%
    return (result[[1]]/sum(result, na.rm = TRUE) < 0.25)
  }
  expect_snapshot_file(tempmp3, snap, compare = mp3_compare)
})