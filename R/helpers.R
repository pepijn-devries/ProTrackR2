.check_mod <- function(mod) {
  if (!inherits(mod, "pt2mod"))
    stop("object does not enherit class 'pt2mod'")
  if (typeof(mod) != "externalptr")
    stop("object is not of type 'externalptr'")
}