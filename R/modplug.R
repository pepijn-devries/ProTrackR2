#' Format a ProTracker pattern conform OpenMPT specs
#' 
#' [OpenMpt](https://openmpt.org/) is a popular modern music tracker. This function allows
#' you to format a pattern such that it can be pasted directly into OpenMPT.
#' On Windows you can use `writeClipboard()` for this purpose.
#' @param pattern An object of class `pt2pat` to be formatted
#' @param ... Ignored
#' @returns Returns a `character` object formatted such that it can be copied into OpenMPT
#' @author Pepijn de Vries
#' @examples
#' mod    <- pt2_read_mod(pt2_demo())
#' as_modplug_pattern(pt2_pattern(mod, 0L))
#' @export
as_modplug_pattern <- function(pattern, ...) {
  if (!inherits(pattern, "pt2pat"))
    stop("'pattern' should be of class pt2pat")

  result <-
    as.character(pattern, padding = c("", "..."), empty_char = c(".", ".", "."),
                 fmt = list(note = "%s", padding = "%s", instrument = "%02X", effect = "%X%02X")) |>
    apply(1, paste0, collapse = "|")
  result <- c("ModPlug Tracker MOD", paste0("|", result))
  return(result)
}
