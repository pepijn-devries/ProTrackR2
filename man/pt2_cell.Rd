% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/cell.R
\name{pt2_cell}
\alias{pt2_cell}
\title{Select a cell from a ProTracker pattern table}
\usage{
pt2_cell(pattern, i, j, ...)
}
\arguments{
\item{pattern}{A \code{pt2pat} class object to extract a cell (\code{pt2cell}) from.}

\item{i, j}{Indices for extracting or replacing ProTrackR2 object elements. The indices
starts at 0, for consistency with ProTracker!}

\item{...}{Ignored}
}
\value{
Returns a cell object from the table as class \code{pt2cell}.
}
\description{
A cell is an element at a specific row and column (channel). It holds information
aboute the note to be played, the instrument (sample) number and the effect to be applied.
For mor information about cells (class \code{pt2cell}) consult \code{vignette("s3class")}.
For more information about selecting elements from ProTrackR2 class objects check out
\code{vignette("select_opts")}.
}
\examples{
mod <- pt2_read_mod(pt2_demo())

pt2_cell(mod$patterns[[1]], 0L, 0L)
}
\author{
Pepijn de Vries
}
