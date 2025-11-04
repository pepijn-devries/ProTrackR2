mod <- pt2_read_mod(pt2_demo())

test_that("Sample name is ok", {
  expect_equal(pt2_name(mod$samples[[1]]), "by pepijn de vries")
})

test_that("Sample list names are ok", {
  expect_equal(pt2_name(mod$samples)[[1]], "by pepijn de vries")
})

test_that("Sample name can be set", {
  expect_equal({
    pt2_name(mod$samples[[1]]) <- "foobar"
    pt2_name(mod$samples[[1]])
  }, "foobar")
})

test_that("Sample list name can be set", {
  expect_identical({
    pt2_name(mod$samples) <- rep("foobar", 31)
    pt2_name(mod$samples)
  }, rep("foobar", 31))
})

test_that("Pattern table can be updated", {
  expect_equal({
    pt2_pattern_table(mod)[[1]] <- 3L
    pt2_pattern_table(mod)[1:4]
  }, c(3L, 0L, 1L, 1L))
})

test_that("Sample number is correct", {
  expect_equal({
    pt2_n_sample(mod)
  }, 4L)
})

test_that("Sample properties can be changed", {
  expect_no_error({
    pt2_finetune(mod$samples[[1]])
    pt2_finetune(mod$samples[[1]]) <- -8L
    
    pt2_volume(mod$samples[[2]])
    pt2_volume(mod$samples[[2]]) <- 64L
    
    pt2_loop_start(mod$samples[[1]])
    pt2_loop_start(mod$samples[[1]]) <- 400L
    
    pt2_loop_length(mod$samples[[1]])
    pt2_loop_length(mod$samples[[1]]) <- 274L
    
    pt2_is_looped(mod$samples[[2]])
    pt2_is_looped(mod$samples[[2]]) <- TRUE
  })
})

test_that("Sample can be coerced", {
  expect_s3_class({
    pt2_sample_to_audio(mod$samples[[1]])
  }, "audioSample")
})