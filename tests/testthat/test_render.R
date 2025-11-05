mod <- pt2_read_mod(pt2_demo())

test_that("Module renders as expected", {
  skip_if(Sys.getenv("THIS_IS_RHUB") == "yes", "Skip this test on RHUB as the `av` package leaks memory")
  skip_on_cran()
  skip_if_not_installed("av")
  snap <- "intro.mp3"
  announce_snapshot_file(snap)
  tempwav <- tempfile(fileext = ".wav")
  tempmp3 <- tempfile(fileext = ".mp3")
  rndr <- pt2_render(mod, 10)
  audio::save.wave(rndr, tempwav)
  av::av_audio_convert(tempwav, tempmp3, bit_rate = 124000, verbose = FALSE)
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

test_that("Sample, cell, cell list, pattern and pattern list render without errors", {
  expect_no_error({
    pt2_render(mod$samples[[1L]], samples = mod$samples)
    pt2_render(mod$patterns[1L:2L], samples = mod$samples)
    pt2_render(mod$patterns[[1L]], samples = mod$samples)
    pt2_render(mod$patterns[[1L]][1L:8L,1L], samples = mod$samples)
    pt2_render(mod$patterns[[1L]][1L,1L][[1L]], samples = mod$samples)
  })
})

test_that("Duration is calculated correctly", {
  expect_equal({
    pt2_duration(mod) |> as.numeric()
  }, 69.12204, tolerance = 1e-3)
})