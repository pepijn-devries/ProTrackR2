#' Extract effect commands from a ProTracker module
#' 
#' As explained in `vignette("s3class")`, the ProTracker pattern table
#' consists of cells, containing information about the note and instrument
#' to be played. This function can be used to retrieve or replace the
#' effect commands in a module.
#' @param x An object of class `pt2_cell`, which can be extracted
#' from a pattern table with [`pt2_cell()`].
#' @param silent Don't warn about replacement values not being used or recycled.
#' @param ... Ignored
#' @param value A replacement value. It should be an object that can be converted into
#' an effect command. It can be a `character` string as shown in the example below.
#' @returns Returns a `pt2command` object containing the raw command code.
#' In case of the assign operator (`<-`) an update version of `x` is returned.
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' 
#' ## select a specific cell from the module
#' cell <- pt2_cell(mod$patterns[[1]], 0L, 0L)
#' 
#' ## show the command used for this cell
#' pt2_command(cell)
#' 
#' ## convert character strings into ProTracker commands
#' pt2_command(c("C30", "F06"))
#' 
#' ## Set the command for all cells in the first pattern
#' ## to `C20` (volume at 50%):
#' pt2_command(mod$patterns[[1]][]) <- "C20"
#' @export
pt2_command <- function(x, ...) {
  if (inherits(x, "pt2command")) return(x)
  if (typeof(x) == "raw") {
    raw_fun <- .get_raw_fun(x)
    
    x <-
      raw_fun(x, compact = FALSE) |>
      matrix(ncol = 6, byrow = TRUE)
    x <- x[,c(3,1)] |> t() |> c()
    class(x) <- "pt2command"
    return(x)
  }
  if (inherits(x, "pt2celllist")) {
    x <- lapply(x, \(y) {class(y) <- union("pt2command", class(y)); y})
    class(x) <- "pt2celllist"
  }
  if (inherits(x, c("pt2celllist", "pt2cell"))) {
    class(x) <- union("pt2command", class(x))
    return(x)
  }
  if (is.character(x)) {
    x <-
      cbind(
        command = paste0("0x", substr(x, 0, 1)),
        param   = paste0("0x", substr(x, 2, 3))
      ) |>
      apply(2, as.raw, simplify = FALSE) |>
      (\(y) {do.call(cbind, y)}) () |>
      apply(1, c, simplify = FALSE) |>
      unlist()
    class(x) <- "pt2command"
    return(x)
  }
  stop("`pt2_command` not implemented for '%s'",
       paste0(union(class(x), typeof(x)), collapse = "'/'"))
}

#' @rdname pt2_command
#' @export
`pt2_command<-` <- function(x, silent = TRUE, ..., value) {
  if (!inherits(x, c("pt2cell", "pt2celllist")))
    stop("`x` should inherit `pt2cell` or `pt2celllist`.")
  value <-
    pt2_command(value) |>
    as.raw() |>
    matrix(ncol = 2, byrow = TRUE)
  
  if (typeof(x) == "raw") {
    cur_notation <- attributes(x)$compact_notation
    cur_class <- class(x)
    raw_fun <- .get_raw_fun(x)
    x <- raw_fun(x, compact = FALSE)
    x <- matrix(x, ncol = 6, byrow = TRUE)
    x[,3] <- value[,1]
    x[,1] <- value[,2]
    x <- c(t(x))
    class(x) <- cur_class
    attributes(x)$compact_notation <- FALSE
    raw_fun <- .get_raw_fun(x)
    x <- raw_fun(x, compact = cur_notation)
    return(x)
  } else {
    if (inherits(x, "pt2cell")) {
      pt_set_eff_command_(list(x$mod), x$i, x$k, x$j, value[1:2], !silent)
    } else {
      mod <- lapply(x, `[[`, "mod")
      i <- lapply(x, `[[`, "i") |> unlist()
      j <- lapply(x, `[[`, "j") |> unlist()
      k <- lapply(x, `[[`, "k") |> unlist()
      pt_set_eff_command_(mod, i, k, j, value, !silent)
    }
  }

  x
}

.get_raw_fun <- function(x) {
  fun <- as.raw.pt2cell
  if (inherits(x, "pt2celllist")) fun <- as.raw.pt2celllist
  return (fun)
}