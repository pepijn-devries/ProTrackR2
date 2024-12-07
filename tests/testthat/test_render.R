test_that("Module renders as expected", {
  snap <- "intro.rdata"
  announce_snapshot_file(snap)
  mod <- pt2_read_mod(pt2_demo())
  temprda <- tempfile(fileext = ".Rdata")
  rndr <- pt2_render(mod, 0.5)
  save(rndr, file = temprda, compress = TRUE)
  expect_snapshot_file(temprda, snap)
})