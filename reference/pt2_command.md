# Extract effect commands from a ProTracker module

As explained in
[`vignette("s3class")`](https://pepijn-devries.github.io/ProTrackR2/articles/s3class.md),
the ProTracker pattern table consists of cells, containing information
about the note and instrument to be played. This function can be used to
retrieve or replace the effect commands in a module.

## Usage

``` r
pt2_command(x, ...)

pt2_command(x, silent = TRUE, ...) <- value
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

  Ignored

- silent:

  Don't warn about replacement values not being used or recycled.

- value:

  A replacement value. It should be an object that can be converted into
  an effect command. It can be a `character` string as shown in the
  example below.

## Value

Returns a `pt2command` object containing the raw command code. In case
of the assign operator (`<-`) an update version of `x` is returned.

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())

## select a specific cell from the module
cell <- pt2_cell(mod$patterns[[1]], 0L, 0L)

## show the command used for this cell
pt2_command(cell)
#> A08

## convert character strings into ProTracker commands
pt2_command(c("C30", "F06"))
#> C30
#> F06

## Set the command for all cells in the first pattern
## to `C20` (volume at 50%):
pt2_command(mod$patterns[[1]][]) <- "C20"
```
