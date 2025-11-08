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

test_that("Pattern can be coerced to modplug", {
  expect_equal({
    mp_pat <- as_modplug_pattern(pt2_pattern(mod, 0L))[[1]]
  }, "ModPlug Tracker MOD")
})