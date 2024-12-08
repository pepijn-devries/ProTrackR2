load("_openmpt/testcasestate.Rdata")
openmpt_test_basurl <- "https://resources.openmpt.org/player_tests/mod/"

test_that("Openmpt test cases result correspond with earlier results", {
  expect_no_error({
    skip_on_cran()
    skip_on_covr()
    skip_on_ci()
    skip_if_offline()
    skip_if(!interactive(), "Session not interactive")
    skip_if(length(unclass(packageVersion("ProTrackR2"))[[1]]) > 3,
            "Skipping during development")
    for (f in openmpt_testfiles$filename) {
      testfile <- paste0(openmpt_test_basurl, f)
      mod <- pt2_read_mod(testfile)
      result <- ""
      idx <- which(openmpt_testfiles$filename == f)
      cat(openmpt_testfiles$description[idx])
      cat("\n")
      while (!grepl("^[Y|N|S].*$", toupper(result))) {
        instance <- play(
          mod,
          duration = 4 + as.numeric(pt2_duration(mod)),
          options = pt2_render_options(stereo_separation = 100L))
        sprintf("(%02i/%02i) This is test '%s', currently evaluated as '%s'. Agree? [Y/N/r(=replay)/s(=skip all)]? ",
                idx, nrow(openmpt_testfiles), f, openmpt_testfiles$state[idx]) |> cat()
        result <- readLines(n = 1L)
        close(instance)
      }
      if (toupper(result) == "S") break
      if (toupper(result) == "N") stop("Test does not match earlier result")
    }
  })
})

test_that("openmpt cases render without errors", {
  expect_no_error({
    skip_if_offline()
    skip_on_cran()
    for (f in openmpt_testfiles$filename) {
      testfile <- paste0(openmpt_test_basurl, f)
      mod <- pt2_read_mod(testfile)
      pt2_render(mod)
    }
  })
})