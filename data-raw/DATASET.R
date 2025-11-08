## Benchmark rendering by ProTrackR and ProTrackR2 packages
if (!requireNamespace("ProTrackR")) {
  stop("Install required packages first and retry")
} else {
  mod1 <- ProTrackR::mod.intro
  mod2 <- ProTrackR2::pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
  dur1 <- dur2 <- numeric()
  for (i in 1:10) {
    starttime <- Sys.time()
    wav1 <- ProTrackR::modToWave(mod1, target.rate = 48000, max.duration = 120) |>
      suppressMessages()
    endtime <- Sys.time()
    dur1 <- c(dur1, as.numeric(endtime - starttime, "secs"))
  }
  for (i in 1:10) {
    starttime <- Sys.time()
    wav2 <- ProTrackR2::pt2_render(mod2, duration = 120)
    endtime <- Sys.time()
    dur2 <- c(dur2, as.numeric(endtime - starttime, "secs"))
  }
  mean(dur1)/mean(dur2)
  if (requireNamespace("av")) {
    tempsmp <- tempfile(fileext = ".wav")
    pt2_write_sample(mod2$samples[[1]], tempsmp)
    samp <- av::read_audio_bin(tempsmp)
    av::av_audio_convert(
      tempsmp, "data-raw/test-sample.aiff", format = "u8")
  }
}
