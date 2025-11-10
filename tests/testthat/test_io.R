mod     <- pt2_read_mod(pt2_demo())
sample  <- pt2_sample(mod, 0L)
smpfile_raw <- tempfile(fileext = ".raw")
smpfile_iff <- tempfile(fileext = ".iff")
smpfile_wav <- tempfile(fileext = ".wav")
smpfile_aiff <- "https://github.com/pepijn-devries/ProTrackR2/raw/refs/heads/master/data-raw/test-sample.aiff"

test_that("Writing and reading a mod works", {
  expect_no_error({
    tf <- tempfile(fileext = ".mod")
    pt2_write_mod(mod, tf)
    pt2_read_mod(tf)
  })
})

test_that("Writing raw samples will warn user", {
  expect_warning({
    pt2_write_sample(sample, smpfile_raw)
    pt2_write_sample(sample, smpfile_iff)
    pt2_write_sample(sample, smpfile_wav)
  })
})

test_that("Reading sample works", {
  skip_on_cran()
  skip_if_offline()
  expect_true({
    samp_raw <- pt2_read_sample(smpfile_raw)
    samp_iff <- pt2_read_sample(smpfile_iff)
    samp_wav <- pt2_read_sample(smpfile_wav)
    samp_aiff <- pt2_read_sample(smpfile_aiff)
    all(samp_iff == samp_wav) &&
      all(samp_iff == samp_aiff) &&
      all(samp_iff == samp_raw) &&
      pt2_loop_start(samp_iff) == 762 &&
      pt2_loop_length(samp_iff) == 276
  })
})
