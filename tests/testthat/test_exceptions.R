mod <- pt2_read_mod(pt2_demo())

test_that("pt2_command cannot be called on an unsupported type", {
  expect_error({
    pt2_command(1L)
  })
})

test_that("pt2_cell cannot be called on an unsupported type", {
  expect_error({
    pt2_cell(1L)
  })
})

test_that("Malformed string cannot be coerced to pt2_cell", {
  expect_error({
    as_pt2cell("C1A10")
  })
})

test_that("Cannot convert more than 1 element to pt2_cell", {
  expect_error({
    as_pt2cell(c("C101000", "C101000"))
  })
})

test_that("pt2_cell indices cannot be out of range", {
  expect_error({
    pt2_cell(pt2_pattern(mod, 0L), 0L, 64L)
  })
})

test_that("pt2_cell cannot be called on a range of indices", {
  expect_error({
    pt2_cell(pt2_pattern(mod, 0L), 0L, j = 0L:1L)
  })
})

test_that("Internal check fails when mod is not external pointer", {
  expect_error({
    modtest <- 1L
    class(modtest) <- "pt2mod" # create fake pt2mod object
    ProTrackR2:::.check_mod(modtest)
  })
})

test_that("Internal check fails when mod is not of correct S3 class", {
  expect_error({
    ProTrackR2:::.check_mod(1L)
  })
})

test_that("pt2_instrument cannot be called on an unsupported type", {
  expect_error({
    pt2_instrument(1L)
  })
})

test_that("NA cannot be assigned to pt2_instrument", {
  expect_error({
    pt2_instrument(mod$patterns[[1]][1,1]) <- NA_integer_
  })
})

test_that("pt2_instrument cannot be assigned an NA", {
  expect_error({
    pt2_instrument(1L)
    cell <- pt2_cell(pt2_pattern(mod, 0L), 0L, j = 0L)
    pt2_instrument(cell) <- NA_integer_
  })
})

test_that(
  "Cannot read module from text connection", {
    expect_error({
      con <- file(pt2_demo(), "rt")
      tryCatch({
        mod <- pt2_read_mod(con)
      }, finally = {
        close(con)
      })
    })
  })

test_that(
  "Cannot read module from too small file", {
    expect_error({
      con <- rawConnection(raw(10L), "rb")
      tryCatch({
        mod <- pt2_read_mod(con)
      })
    })
  })

test_that(
  "Cannot write module to text connection", {
    expect_error({
      con <- file(tempfile(), "wt")
      tryCatch({
        mod <- pt2_write_mod(mod, con)
      }, finally = {
        close(con)
      })
    })
  })

test_that(
  "Cannot read sample from text connection", {
    expect_error({
      con <- file(pt2_demo(), "rt")
      tryCatch({
        mod <- pt2_read_sample(con)
      }, finally = {
        close(con)
      })
    })
  })

test_that(
  "Cannot write sample to text connection", {
    expect_error({
      con <- file(tempfile(), "wt")
      smp <- pt2_sample(mod, 0L)
      tryCatch({
        mod <- pt2_write_sample(smp, con)
      }, finally = {
        close(con)
      })
    })
  })

test_that("as_modplug_pattern cannot be called on an unsupported type", {
  expect_error({
    as_modplug_pattern(1L)
  })
})

test_that("pt2_note cannot be called on an unsupported type", {
  expect_error({
    pt2_note(1L)
  })
})

test_that("pt2mod has no more than 2 elements", {
  expect_error({
    mod[[3L]]
  })
})

test_that("Cannot add more than 100 patterns to a module", {
  expect_error({
    mod$patterns[[101]] <- pt2_pattern()
  })
})

test_that("Cannot set looped state to more than one value", {
  expect_error({
    pt2_is_looped(mod$samples[[1]]) <- c(TRUE, TRUE)
  })
})

test_that("You cannot assign more than 100 patterns to a module", {
  expect_error({
    mod$patterns <- mod$patterns[rep(1, times = 102L)]
  })
})

test_that("You cannot select unknown elements from a module", {
  expect_error({
    mod[["foobar"]]
  })
})
