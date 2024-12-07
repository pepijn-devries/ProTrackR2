test_that("Module renders as expected", {
  snap <- "intro.gz"
  announce_snapshot_file(snap)
  mod <- pt2_read_mod(pt2_demo())
  tempgz <- tempfile(fileext = ".gz")
  rndr <- pt2_render(mod, 0.5)
  con <- gzfile(tempgz, "wb", compression = 9)
  writeBin(c(rndr), con, endian = "little")
  close(con)
  expect_snapshot_file(tempgz, snap)
})