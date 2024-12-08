mod      <- pt2_read_mod(pt2_demo())
pattern  <- pt2_pattern(mod, 0L)
cell     <- pt2_cell(pattern, 0L, 0L)
celllist <- pattern[, 1L]

test_that("Raw cell can be generated from external pointer", {
  expect_true({
    cell_raw1 <- as.raw(cell, compact = FALSE)
    cell_raw2 <- as.raw(cell, compact = TRUE)
    length(cell_raw1) == 6L && length(cell_raw2) == 4L
  })
})

test_that("pt2_command exists as typeof `raw` and `list`", {
  expect_true({
    typeof(pt2_command(as.raw(cell))) == "raw" && typeof(pt2_command(celllist)) == "list"
  })
})

test_that("pt2_command can assign correctly to raw type", {
  expect_true({
    cell_raw <- as.raw(cell)
    pt2_command(cell_raw) <- "C10"
    as.character(cell_raw) == "C-3 01 C10"
  })
})

test_that("pt2_instrument can be called on `raw` and `externalptr`", {
  expect_true({
    pt2_instrument(as.raw(cell)) == 1L && typeof(pt2_instrument(celllist)) == "integer"
  })
})

test_that("pt2_instrument can assign correctly to raw type", {
  expect_true({
    cell_raw <- as.raw(cell)
    pt2_instrument(cell_raw) <- 2L
    as.character(cell_raw) == "C-3 02 A08"
  })
})