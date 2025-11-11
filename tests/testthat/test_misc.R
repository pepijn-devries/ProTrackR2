mod <- pt2_read_mod(pt2_demo())

test_that("Can add a 100th pattern to a mod", {
  expect_no_error({
    mod$patterns[[100]] <- pt2_pattern()
    pt2_pattern_table(mod)
  })
})

test_that("Commands can be extracted and assigned", {
  expect_no_error({
    comm <- pt2_command(mod$patterns[[1]][])
    pt2_command(comm)
    comm[]
    comm[[1]] <- "111"
  })
})