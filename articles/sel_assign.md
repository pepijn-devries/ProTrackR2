# Selecting and assigning ProTrackR2 objects

## Introduction

This vignette describes how to select elements from ProTrackR2 module
objects and how to replace them. In order to best understand this, it is
necessary to understand how objects are organised in the ProTrackR2
package. Please read
[`vignette("s3class")`](https://pepijn-devries.github.io/ProTrackR2/articles/s3class.md)
first, or use it for reference when reading the vignette at hand. Before
we continue, we start the session by loading a module file.

``` r
library(ProTrackR2)

mod <- pt2_read_mod(pt2_demo())
```

## Selecting lists from modules

There are two important lists of objects contained in ProTracker
modules. The first is a list of patterns (`pt2patlist`), which can be
obtained as follows.

``` r
mod$patterns
#> pattern list [n=4]
## which is identical to
mod[["patterns"]]
#> pattern list [n=4]
## which is identical to
mod[[1L]]
#> pattern list [n=4]
```

It returns a list of patterns and inherits the class `pt2patlist`, from
it you can select one ore more specific patterns (see below).

The second list you can get from a module is a list of samples
(`pt2samplist`), which can be obtained as follows.

``` r
mod$samples
#> sample list [n=31]
## which is identical to
mod[["samples"]]
#> sample list [n=31]
## which is identical to
mod[[2L]]
#> sample list [n=31]
```

## Index base: one or zero?

For vectors and lists, `R` uses indices starting at 1. ProTracker
modules often use arrays of elements that are starting with indices at 0
(but alas not consistently for everything). And in some case ProTracker
effect commands refer to such indices (see
[`vignette("effect_commands")`](https://pepijn-devries.github.io/ProTrackR2/articles/effect_commands.md)).
Unfortunately, this is a source of confusion, for which there exists no
ideal solution.

In this package, I’ve tried to implement it as consistent as possible.
Named functions and methods use zero-based indices, whereas bracket
operators (`[` and `[[`) use one-based indices conform `R` syntax. This
is summarised in the table below.

| Zero-based index                                                                                                                                                                                                                                                  | One-based index |
|:------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|:----------------|
| [`pt2_cell()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_cell.md), [`pt2_pattern()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_pattern.md), [`pt2_sample()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_sample.md) | `[`, `[[`       |

## Selecting and replacing list elements

### Samples

Selecting a specific sample from a sample list can be done with the
`[[`-operator. Note that sample number 5 is empty (consists of zero
samples):

``` r
mod$samples[[5L]]
#> [1] "PT2 Sample 'commodore amiga 500' [0]"
## which is identical to (see the different base of the index)
pt2_sample(mod, 4L)
#> [1] "PT2 Sample 'commodore amiga 500' [0]"
```

In order to replace this sample with sample number 2, you can simply
assign it like this:

``` r
mod$samples[[5L]] <- mod$samples[[2L]]
```

After this assignment operation samples 2 and 5 are identical. If you
want to replace multiple samples at once you need to use the
`[`-operator. The example below shows how to replace samples 5 up to 8
with sample 2:

``` r
mod$samples[5L:8L] <- mod$samples[2L]
```

Single brackets `[` returns a list (`pt2samplist`) with one or more
elements. Double brackets `[[` return a single element (`pt2samp`). In
essence, this works the same as with normal `list` objects.

### Patterns

For a pattern lists (`pt2patlist`) selection and assignment operators
are similar to those of sample lists (see above). Single brackets `[`
returns a list (`pt2patlist`) with one or more elements. Double brackets
`[[` return a single element (`pt2pat`). In essence, this works the same
as with normal `list` objects.

The range of pattern indices you can use depends on the maximum index
(+1) registered in the pattern sequence table
(`pt2_pattern_table(mod)`). In the currently loaded module there are 4
patterns registered. So to get the fourth pattern table in the module,
you use:

``` r
my_pattern <- mod$patterns[[4L]]
## which is identical to (see the different base of the index)
my_pattern <- pt2_pattern(mod, 3L)
```

If you attempt to get the fifth table (`mod$patterns[[5]]`), you will
get an error, since it doesn’t exist. However, you can append a new
pattern table at this position using the assign operator:

``` r
mod$patterns[[5L]] <- pt2_new_pattern()
```

This new pattern is also automatically added to the pattern sequence
table:

``` r
pt2_pattern_table(mod)
#>   [1] 0 0 1 1 2 2 3 3 2 4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
#>  [38] 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
#>  [75] 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
#> [112] 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
```

You can use the same strategy to copy existing patterns to a new index:

``` r
mod$patterns[[6L]] <- mod$patterns[[1L]]
```

In theory ProTracker modules can hold a maximum of 100 pattern tables.
If this limit can be reached depends on available slots in the pattern
sequence table (`pt2_pattern_table(mod)`). If you try to create a
pattern beyond the maximum of 100 or there are no more room for
additional patterns in the sequence table, you will get an error.

### Cells

As explained in
[`vignette("s3class")`](https://pepijn-devries.github.io/ProTrackR2/articles/s3class.md),
a cell (class `pt2cell`) is the elementary unit of a pattern table and
specifies which instrument (sample) to play at which note and with which
effect. You can select a range of cells with a `[`-operator by referring
to a pattern’s row index and its channel (column) index. The example
below selects the first 8 rows from the first channel.

``` r
my_cells <- mod$patterns[[1L]][1L:8L, 1L]
```

The result of this is an object of class `pt2celllist`. To select a
particular cell from this range you need the double bracket operator
`[[`. To select the first cell of the eight selected cells you do the
following.

``` r
my_cells[[1L]]
#> C-3 01 A08
## which is identical to (see the different base of the indices)
pt2_cell(pt2_pattern(mod, 0L), i = 0L, j = 0L)
#> C-3 01 A08
```

Remember that we appended an empty pattern at position 5? We can replace
the empty cells in that pattern with the selected cells. The example
below shows how the entire first column is filled with the 8 selected
cells (note that the cells are recycled).

``` r
mod$patterns[[5L]][,1L] <- my_cells
```
