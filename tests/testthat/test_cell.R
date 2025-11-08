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

test_that("Dash can be missing when coercing to pt2cell", {
  expect_identical({
    as_pt2celllist("C201A10") |> as.character()
  }, "C-2 01 A10")
})

as_pt2cell("C201A10") |> as.character()