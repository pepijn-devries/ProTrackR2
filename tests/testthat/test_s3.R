mod <- pt2_read_mod(pt2_demo())

test_that("S3 methods don't throw errors", {
  expect_no_error({
    sink(tempfile())
    patterns <- mod$patterns
    pattern  <- patterns[[1]]
    cells    <- pattern[1:4,1]
    cell     <- cells[[1]]
    samples  <- mod$samples
    sample   <- samples[[1]]
    cmnd     <- pt2_command(cell)
    
    ## Let's go wild with S3 methods:
    print(mod)
    print(patterns)
    print(pattern)
    print(cells)
    print(cell)
    print(samples)
    print(sample)
    print(cmnd)
    
    format(mod)
    format(patterns)
    format(pattern)
    format(cells)
    format(cell)
    format(samples)
    format(sample)
    format(cmnd)
    
    as.character(pattern)
    as.character(cells)
    as.character(cell)
    as.character(cmnd)
    
    as.raw(mod)
    as.raw(pattern)
    as.raw(cells)
    as.raw(cell)
    as.raw(sample)
    as.raw(cmnd)
    
    as.integer(sample)
    
    sink()
  })
})

test_that("Select and replace operators work OK", {
  expect_no_error({
    mod$patterns[[1]][] <- "--- 01 111"
  })
})