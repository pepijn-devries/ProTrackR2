% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/io.R
\name{pt2_read_mod}
\alias{pt2_read_mod}
\alias{pt2_write_mod}
\title{Read and write ProTracker modules}
\usage{
pt2_read_mod(file, ...)

pt2_write_mod(mod, file, ...)
}
\arguments{
\item{file}{Filename of the file to read from or write to.}

\item{...}{Ignored}

\item{mod}{An object of class \code{pt2mod}.}
}
\value{
\code{pt2_read_mod()} returns a \code{pt2mod} class object when successful.
\code{pt_write_mod()} returns \code{NULL} invisibly.
}
\description{
Functions to read and write ProTracker module. The read function will
read a number of mod files that are compatible with ProTracker, this
includes files compressed with PowerPacker (PP). The write function
will only write modules conform ProTracker specifications.
}
\examples{
mod <- pt2_read_mod(pt2_demo())
}
\author{
Pepijn de Vries
}
