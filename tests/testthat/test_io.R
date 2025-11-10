mod     <- pt2_read_mod(pt2_demo())
sample  <- pt2_sample(mod, 0L)
smpfile_raw <- tempfile(fileext = ".raw")
smpfile_iff <- tempfile(fileext = ".iff")
smpfile_wav <- tempfile(fileext = ".wav")
smpfile_aiff <- "https://github.com/pepijn-devries/ProTrackR2/blob/master/data-raw/test-sample.aiff"

test_that("Writing a mod works", {
  expect_no_error({
    pt2_write_mod(mod, tempfile(fileext = ".mod"))
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
  skip_if_offline()
  expect_no_error({
    samp_raw <- pt2_read_sample(smpfile_raw)
    samp_iff <- pt2_read_sample(smpfile_iff)
    samp_wav <- pt2_read_sample(smpfile_wav)
    samp_aiff <- pt2_read_sample(smpfile_aiff)
  })
})
