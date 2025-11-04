#' Select and assign operators for ProTrackR2 S3 class objects
#' 
#' Functions to select and assign elements to ProTracker modules.
#' See `vignette('s3class')` for an overview of ProTrackR2 S3 class objects. See `vignette('sel_assign')`
#' for practical guidance on selecting and assigning elements of ProTrackR2 class objects.
#' @param x Object to apply S3 method to. See 'usage' section for allowed object types.
#' @param i,j Indices for extracting or replacing ProTrackR2 object elements
#' @param value Replacement value for the selected element(s).
#' @param ... Passed on to other methods.
#' @returns Returns the selected object in case of a selection (`[`, `[[`, or `$`) operator. Returns the
#' updated object `x` in case of an assignment (`<-`) operator.
#' @rdname select_assign
#' @export
`$.pt2mod` <- function(x, i, ...) {
  x[[i]]
}

#' @rdname select_assign
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
  
  if (i == 1L || toupper(i) == "PATTERNS") {
    if (length(value) > 100L)
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
      seq_table[1L + idx_available[seq_along(new_ptns)]] <- new_ptns
      update_pattern_sequence_(x, seq_table)
    }
    for (j in seq_len(length(value))) {
      if (is.raw(value[[j]])) {
        set_new_pattern_(x, as.integer(j - 1L), unclass(value[[j]]))
      }
    }
  } else if (i == 2L || toupper(i) == "SAMPLES") {
    for (j in seq_len(length(value))) {
      if (is.raw(value[[j]])) {
        if (!validate_sample_raw_(value[[j]])) stop(sprintf("Not a valid sample at index %i", j))
        mod_set_sample_(x, as.integer(j - 1L), value[[j]])
      }
    }
    
  } else {
    stop("Index out of range")
  }
  x
}

#' @rdname select_assign
#' @export
`[[.pt2mod` <- function(x, i, ...) {
  if (i == 1 || toupper(i) == "PATTERNS") {
    result <-
      pt2_n_pattern(x) |>
      seq_len() |>
      lapply(\(y) pt2_pattern(x, y - 1) |> as.raw())
    class(result) <- "pt2patlist"
    result
  } else if (i == 2 || toupper(i) == "SAMPLES") {
    result <-
      lapply(1:31, \(y) pt2_sample(x, y - 1) |> as.raw())
    class(result) <- "pt2samplist"
    result
  } else {
    stop("Index out of range")
  }
}

#' @rdname select_assign
#' @export
`[.pt2patlist` <- function(x, i, ...) {
  x <- unclass(x)
  x <- NextMethod()
  class(x) <- "pt2patlist"
  x
}

#' @rdname select_assign
#' @export
`[[.pt2patlist` <- function(x, i, ...) {
  x <- unclass(x)
  x <- NextMethod()
  if (is.null(x)) return(x)
  class(x) <- "pt2pat"
  x
}

#' @rdname select_assign
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

#' @rdname select_assign
#' @export
`[.pt2samplist` <- function(x, i, ...) {
  x <- unclass(x)
  x <- NextMethod()
  class(x) <- "pt2samplist"
  x
}

#' @rdname select_assign
#' @export
`[[.pt2samplist` <- function(x, i, ...) {
  x <- unclass(x)
  x <- NextMethod()
  class(x) <- "pt2samp"
  x
}

#' @rdname select_assign
#' @export
`[.pt2pat` <- function(x, i, j, ...) {
  if (missing(i)) i <- 1L:64L
  if (missing(j)) j <- 1L:4L
  if (typeof(x) == "raw") {
    cur_notation <- attributes(x)$compact_notation
    width <- ifelse(cur_notation, 4L, pt_cell_bytesize())
    idx <- outer(i - 1L, j - 1L, \(y, z) y*width * 4L + z*width) |> c()
    idx <- outer(1:width, idx, `+`) |> c()
    x <- unclass(x)
    x <- x[idx]
    attributes(x)$compact_notation <- cur_notation
  } else {
    idx <- expand.grid(i = i - 1L, j = j - 1L)
    x <- mapply(\(y, i, j) pt2_cell(x, i, j), i = idx[,"i"], j = idx[,"j"], SIMPLIFY = FALSE)
  }
  class(x) <- "pt2celllist"
  attr(x, "celldim") <- c(length(i), length(j))
  as.raw.pt2celllist(x, compact = TRUE)
}

#' @rdname select_assign
#' @export
`[<-.pt2pat` <- function(x, i, j, ..., value) {
  if (is.character(value)) value <- as_pt2celllist(value)
  if (!inherits(value, "pt2celllist"))
    stop("`values` should be of class `pt2celllist`")
  
  if (missing(i)) i <- 1L:64L
  if (missing(j)) j <- 1L:4L
  target_idx <-
    expand.grid(as.integer(i) - 1L, as.integer(j) - 1L)
  
  if (typeof(x) == "raw") {
    values <- as.raw.pt2celllist(value, compact = attributes(x)$compact_notation)
    compact <- attributes(x)$compact_notation
    size <- ifelse(compact, 4L, pt_cell_bytesize())
    x <- x |> unclass()
    target_idx <-
      target_idx |>
      apply(1, \(z) size * (z[1] * 4L + z[2])) |>
      lapply(`+`, seq_len(size)) |>
      unlist()
    x[target_idx] <- unlist(values)
    class(x) <- "pt2pat"
    attributes(x)$compact_notation <- compact
    x
  } else {
    values <- as.raw.pt2celllist(value, compact = FALSE)
    m <- replace_cells_(x, as.matrix(target_idx), values)
    if (m != "") warning(m)
    x
  }
}

#' @rdname select_assign
#' @export
`[[<-.pt2celllist` <- function(x, i, ..., value) {
  if (!inherits(value, c("pt2cell", "pt2celllist")))
    value <- as_pt2celllist(value)
  
  if (length(i) != 1 || length(value) != 1)
    stop("Index and replacement should both have a length of 1")
  x[i, ...] <- value
  x
}

#' @rdname select_assign
#' @export
`[<-.pt2celllist` <- function(x, i, ..., value) {
  if (!inherits(value, c("pt2cell", "pt2celllist")))
    value <- as_pt2celllist(value)
  
  if (length(list(...)) != 0) warning("Ignoring arguments passed to dots")
  
  if (typeof(x) == "raw") {
    cpt <- attr(x, "compact_notation")
    d <- attr(x, "celldim")
    value <- as.raw.pt2celllist(value, compact = cpt)
    sz <- ifelse(cpt, 4L, pt_cell_bytesize())
    idx <- rep((i - 1L)*sz, each = sz) + seq_len(sz)
    x <- unclass(x)
    x[idx] <- unclass(value)
    x <- structure(x, class = "pt2celllist", celldim = d, compact_notation = cpt)
  } else {
    replace_cells_(x, as.integer(i), as.raw.pt2celllist(value, compact = FALSE))
  }
  x
}

.raw_sel_celllist <- function(x, i) {
  cur_notation <- attributes(x)$compact_notation
  width        <- ifelse(cur_notation, 4L, pt_cell_bytesize())
  idx          <- outer(seq_len(width), (i - 1L)*width, `+`) |> c()
  x            <- unclass(x)[idx]
  attributes(x)$compact_notation <- cur_notation
  return(x)
}

.raw_sel_command <- function(x, i) {
  idx          <- outer(seq_len(2L), (i - 1L)*2L, `+`) |> c()
  x            <- unclass(x)[idx]
  return(x)
}

#' @rdname select_assign
#' @export
`[[.pt2celllist` <- function(x, i, ...) {
  if (typeof(x) == "raw") {
    cur_class    <- class(x)
    x <- .raw_sel_celllist(x, i)
    class(x) <- union("pt2cell", setdiff(cur_class, "pt2cellist"))
    x
  } else {
    NextMethod()
  }
}

#' @rdname select_assign
#' @export
`[.pt2celllist` <- function(x, i, ...) {
  cur_class <- class(x)
  if (typeof(x) == "raw") {
    x <- .raw_sel_celllist(x, i)
  } else {
    x <- NextMethod()
  }
  class(x) <- cur_class
  x
}

#' @rdname select_assign
#' @export
`[[.pt2command` <- function(x, i, ...) {
  if (typeof(x) == "raw") {
    x <- .raw_sel_command(x, i)
    class(x) <- "pt2command"
    x
  } else {
    NextMethod()
  }
}

#' @rdname select_assign
#' @export
`[.pt2command` <- function(x, i, ...) {
  cur_class <- class(x)
  if (typeof(x) == "raw") {
    x <- .raw_sel_command(x, i)
  } else {
    x <- NextMethod()
  }
  class(x) <- cur_class
  x
}

#' @rdname select_assign
#' @export
`[[<-.pt2command` <- function(x, i, ..., value) {
  if (inherits(x, c("pt2cell", "pt2celllist"))) {
    
    class(x) <- setdiff(class(x), "pt2command")
    pt2_command(x[[i]]) <- value
    class(x) <- union("pt2command", class(x))
    
  } else if (typeof(x) == "raw") {
    
    x <- .command_list(x)
    value <- pt2_command(value) |>
      as.raw() |>
      .command_list()
    x[[i]] <- value
    x <- unlist(x)
    class(x) <- "pt2command"
    
  } else {
    stop("Replacement method not implemented")
  }
  x
}

#' @rdname select_assign
#' @export
`[<-.pt2command` <- function(x, i, ..., value) {
  if (inherits(x, c("pt2cell", "pt2celllist"))) {
    
    class(x) <- setdiff(class(x), "pt2command")
    pt2_command(x[i]) <- value
    class(x) <- union("pt2command", class(x))
    
  } else if (typeof(x) == "raw") {
    
    x <- .command_list(x)
    value <- pt2_command(value) |>
      as.raw() |>
      .command_list()
    x[i] <- value
    x <- unlist(x)
    class(x) <- "pt2command"
    
  } else {
    stop("Replacement method not implemented")
  }
  x
}

.command_list <- function(x) {
  matrix(unclass(x), ncol = 2L, byrow = TRUE) |>
    apply(1, c, simplify = FALSE)
}
