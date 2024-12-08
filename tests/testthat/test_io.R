mod     <- pt2_read_mod(pt2_demo())
sample  <- pt2_sample(mod, 0L)
smpfile <- tempfile(fileext = ".raw")

test_that("Writing a mod works", {
  expect_no_error({
    pt2_write_mod(mod, tempfile(fileext = ".mod"))
  })
})

test_that("Writing raw samples will warn user", {
  expect_warning({
    pt2_write_sample(sample, smpfile)
  })
})

test_that("Reading sample works", {
  expect_no_error({
    pt2_read_sample(smpfile)
  })
})
