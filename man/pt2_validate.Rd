% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/validate.R
\name{pt2_validate}
\alias{pt2_validate}
\alias{pt2_validate.pt2samp}
\title{Validate ProTrackR2 S3 class objects}
\usage{
pt2_validate(x, ...)

\method{pt2_validate}{pt2samp}(x, ...)
}
\arguments{
\item{x}{object to be validated}

\item{...}{Ignored}
}
\value{
A \code{logical} value indicating whether the object is valid or not
}
\description{
Check aspects of S3 class objects for validity. For samples for instance it
is checked if all parameters (volume, finetune, etc.) are within ProTracker specifications.
}
\examples{
mod <- pt2_read_mod(pt2_demo())

pt2_validate(mod$samples[[1]])
}
\author{
Pepijn de Vries
}
