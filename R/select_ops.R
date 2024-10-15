#' @export
`$.pt2mod` <- function(x, i, ...) {
  x[[i]]
}

#' @export
`$<-.pt2mod` <- function(x, i, value) {
  value <-
    value |>
    length() |>
    seq_len() |>
    lapply(\(y) {
      if ("mod" %in% names(value[[y]]) && identical(value[[y]]$mod, x) && value[[y]]$i == y) {
        value[[y]]
      } else if (is.null(value[[y]])) {
        value[[y]]
      } else {
        if (inherits(value[[y]], "pt2pat"))
          as.raw.pt2pat(value[[y]], compact = FALSE) else
            as.raw.pt2samp(value[[y]])
      }
    })
  
  if (i == 1 || toupper(i) == "PATTERNS") {
    if (length(value) > 100)
      stop("A ProTracker module cannot hold more than 100 patterns.")
    n_pat <- pt2_n_pattern(x)
    empty_spots <- pt2_pattern_table(x) |> rev() |> cumsum()
    empty_spots <- sum(empty_spots == 0L)
    if (empty_spots >= 128L) empty_spots <- 128L - n_pat
    if (length(value) > (n_pat + empty_spots) || length(value) > 100L)
      stop("ProTracker cannot hold more than 100 patterns or insufficient room in sequene table.")
    
    # increase patterns in sequence table if necessary
    if (length(value) > n_pat) {
      seq_table <- pt2_pattern_table(x)
      new_ptns <- seq(n_pat, length(value) - 1L)
      idx_available <- 128L - seq(1L, empty_spots) |> rev()
      seq_table[1 + idx_available[seq_along(new_ptns)]] <- new_ptns
      update_pattern_sequence_(x, seq_table)
    }
    for (j in seq_len(length(value))) {
      if (is.raw(value[[j]])) {
        set_new_pattern_(x, as.integer(j - 1L), unclass(value[[j]]))
      }
    }
  } else if (i == 2 || toupper(i) == "SAMPLES") {
    for (j in seq_len(length(value))) {
      if (is.raw(value[[j]])) {
        mod_set_sample_(x, as.integer(j - 1L), value[[j]])
      }
    }
    
  } else {
    stop("Index out of range")
  }
  x
}

#' @export
`[[.pt2mod` <- function(x, i, ...) {
  if (i == 1 || toupper(i) == "PATTERNS") {
    result <-
      pt2_n_pattern(x) |>
      seq_len() |>
      lapply(\(y) pt2_pattern(x, y - 1) |> unclass())
    class(result) <- "pt2patlist"
    result
  } else if (i == 2 || toupper(i) == "SAMPLES") {
    result <-
      lapply(1:31, \(y) pt2_sample(x, y - 1) |> unclass())
    class(result) <- "pt2samplist"
    result
  } else {
    stop("Index out of range")
  }
}

#' @export
`[.pt2patlist` <- function(x, i, ...) {
  x <- unclass(x)
  x <- NextMethod()
  class(x) <- "pt2patlist"
  x
}

#' @export
`[[.pt2patlist` <- function(x, i, ...) {
  x <- unclass(x)
  x <- NextMethod()
  if (is.null(x)) return(x)
  class(x) <- "pt2pat"
  x
}

#' @export
`[[<-.pt2patlist` <- function(x, i, value) {
  if (!inherits(value, "pt2pat"))
    stop("Can only replace a pattern in a pattern list by an object of class `pt2pat`")
  value <- as.raw.pt2pat(value, compact = TRUE)
  x <- unclass(x)
  x[[i]] <- value
  class(x) <- "pt2patlist"
  x
}

#' @export
`[.pt2samplist` <- function(x, i, ...) {
  x <- unclass(x)
  x <- NextMethod()
  class(x) <- "pt2samplist"
  x
}

#' @export
`[[.pt2samplist` <- function(x, i, ...) {
  x <- unclass(x)
  x <- NextMethod()
  class(x) <- "pt2samp"
  x
}