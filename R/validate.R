#' Validate ProTrackR2 S3 class objects
#' 
#' Check aspects of S3 class objects for validity. For samples for instance it
#' is checked if all parameters (volume, finetune, etc.) are within ProTracker specifications.
#' @param x object to be validated
#' @param ... Ignored
#' @returns A `logical` value indicating whether the object is valid or not
#' @examples
#' mod <- pt2_read_mod(pt2_demo())
#' 
#' pt2_validate(mod$samples[[1]])
#' @author Pepijn de Vries
#' @export
pt2_validate <- function(x, ...) {
  UseMethod("pt2_validate", x)
}

#' @rdname pt2_validate
#' @name pt2_validate
#' @export
pt2_validate.pt2samp <- function(x, ...) {
  validate_sample_raw_(as.raw(x))
}
