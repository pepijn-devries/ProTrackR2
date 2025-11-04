mod <- pt2_read_mod(pt2_demo())

test_that("Mod length is OK", {
  expect_equal(pt2_length(mod), 9L)
})

test_that("Mod length can be set", {
  expect_equal({
    pt2_length(mod) <- 10L
    pt2_length(mod)
  }, 10L)
})

test_that("Mod name is ok", {
  expect_equal(pt2_name(mod), "intro")
})

test_that("Mod name can be set", {
  expect_equal({
    pt2_name(mod) <- "foobar"
    pt2_name(mod)
  }, "foobar")
})
