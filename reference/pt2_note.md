# Extract a note from a ProTracker module

Gets note information from a cell in a pattern table in a ProTracker
Module.

## Usage

``` r
pt2_note(x, ...)

pt2_note(x, silent = TRUE, ...) <- value
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

  A `character` string to replace the selected notes from `x`.

## Value

Returns a string representing the note's key.

## Details

A string representing the note's key is returned by the function. The
first letter indicates the position of the note in the [diatonic
scale](https://en.wikipedia.org/wiki/Diatonic_scale). The second
character indicates if it is a sharp key (with a hash symbol, and a dash
if it is not). The third character indicates the octave of the note. In
ProTracker allowed notes range from `"C-1"` to `"B-3"`.

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())

## select a specific cell from the first pattern
cell <- pt2_cell(mod$patterns[[1]], 0L, 0L)

## get the note played by this particular cell
pt2_note(cell)
#> [1] "C-3"

## Replace the notes in the first pattern
## with those of the first bar of
## 'Frère Jacques'
pt2_note(mod$patterns[[1]][]) <-
  c("C-2", "---", "---", "---",
    "D-2", "---", "---", "---",
    "E-2", "---", "---", "---",
    "C-2", "---", "---", "---")
```
