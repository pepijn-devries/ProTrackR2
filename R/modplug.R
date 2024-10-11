#' Format a Protracker pattern conform OpenMPT specs
#' 
#' [OpenMpt](https://openmpt.org/) is a popular modern music tracker. This function allows
#' you to format a pattern such that it can be pasted directly into OpenMPT.
#' On windows you can use `writeClipboard()` for this purpose.
#' @param pattern An object of class `pt2pat` to be formatted
#' @param ... Ignored
#' @returns Returns a `character` object formatted such that it can be copied into OpenMPT
#' @author Pepijn de Vries
#' @examples
#' mod    <- pt2_read_mod(system.file("mod.intro", package = "ProTrackR2"))
#' mp_pat <- as_modplug_pattern(pt2_pattern(mod, 0L))
#' @export
as_modplug_pattern <- function(pattern, ...) {
  if (!inherits(pattern, "pt2pat"))
    stop("'pattern' should be of class pt2pat")
  result <-
    as.character(pattern, padding = c("", "..."), empty_char = c(".", ".", ".")) |>
    apply(1, paste0, collapse = "|")
  result <- c("ModPlug Tracker MOD", paste0("|", result))
  return(result)
}