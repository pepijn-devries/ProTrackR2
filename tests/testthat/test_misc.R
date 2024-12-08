mod <- pt2_read_mod(pt2_demo())

test_that("Can add a 100th pattern to a mod", {
  expect_no_error({
    mod$patterns[[100]] <- pt2_new_pattern()
    pt2_pattern_table(mod)
  })
})

