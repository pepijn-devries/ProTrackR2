#' @keywords internal
"_PACKAGE"
NULL

.onUnload <- function(libpath) {
  ## Cleans up memory allocated by pt_init_()
  pt_cleanup_()
  ## Unload the dynamic binding (particularly useful on Windows)
  library.dynam.unload("ProTrackR2", libpath)
}

.onLoad <- function(libname, pkgname) {
  ## Initializes ProTracker configuration and audio buffer
  pt_init_()
}

#' @useDynLib ProTrackR2, .registration = TRUE
NULL
