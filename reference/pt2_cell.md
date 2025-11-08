# Select a cell from a ProTracker pattern table

A cell is an element at a specific row and column (channel). It holds
information aboute the note to be played, the instrument (sample) number
and the effect to be applied. For mor information about cells (class
`pt2cell`) consult
[`vignette("s3class")`](https://pepijn-devries.github.io/ProTrackR2/articles/s3class.md).
For more information about selecting elements from ProTrackR2 class
objects check out `vignette("select_opts")`.

## Usage

``` r
pt2_cell(pattern, i, j, ...)
```

## Arguments

- pattern:

  A `pt2pat` class object to extract a cell (`pt2cell`) from.

- i, j:

  Indices for extracting or replacing ProTrackR2 object elements. The
  indices starts at 0, for consistency with ProTracker!

- ...:

  Ignored

## Value

Returns a cell object from the table as class `pt2cell`.

## Author

Pepijn de Vries

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())

pt2_cell(mod$patterns[[1]], 0L, 0L)
#> C-3 01 A08
```
