test_that("A character string can be coerced to pt2cell", {
  expect_identical({
    as_pt2cell("C-3 01 A0F") |> as.character()
  }, "C-3 01 A0F")
})

test_that("A character string can be coerced to pt2cell", {
  expect_identical({
    as_pt2celllist("C-3 01 A0F") |> as.character()
  }, "C-3 01 A0F")
})