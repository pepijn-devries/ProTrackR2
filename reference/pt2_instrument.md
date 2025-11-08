# Extract or replace a sample index from a ProTracker pattern

As explained in
[`vignette("s3class")`](https://pepijn-devries.github.io/ProTrackR2/articles/s3class.md),
the ProTracker pattern table consists of cells containing information
about the note and instrument to be played. This function extracts the
sample index (instrument) from such a cell.

## Usage

``` r
pt2_instrument(x, ...)

pt2_instrument(x, silent = TRUE, ...) <- value
```

## Arguments

- x:

  An object of class `pt2cell`, which can be extracted from a pattern
  table with
  [`pt2_cell()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_cell.md).
  A cell list (class `pt2celllist`) is also allowed. See
  [`vignette("sel_assign")`](https://pepijn-devries.github.io/ProTrackR2/articles/sel_assign.md)
  for more details about selecting cells and cell lists.

- ...:

  Ignored.

- silent:

  Don't warn about replacement values not being used or recycled.

- value:

  Replacement value for the instrument (sample id). An `integer` value
  ranging from `0` to `31`.

## Value

Returns the `integer` sample index in `x`. The index has a base of `1`.
An index of `0` means 'no sample'. In case of the assignment operator
(`<-`) an updated version of `x` is returned

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())

## select a specific cell from the first pattern
cell <- pt2_cell(mod$patterns[[1]], 0L, 0L)

## get the sample number used in this cell
pt2_instrument(cell)
#> [1] 1

## Replace the instrument in all cells of
## pattern 1 with sample number 3:
pt2_instrument(mod$patterns[[1]][]) <- 3
```
