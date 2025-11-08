# Read and write ProTracker modules

Functions to read and write ProTracker module. The read function will
read a number of mod files that are compatible with ProTracker, this
includes files compressed with PowerPacker (PP). The write function will
only write modules conform ProTracker specifications.

## Usage

``` r
pt2_read_mod(file, ...)

pt2_write_mod(mod, file, ...)
```

## Arguments

- file:

  Filename of the file to read from or write to.

- ...:

  Ignored

- mod:

  An object of class `pt2mod`.

## Value

`pt2_read_mod()` returns a `pt2mod` class object when successful.
`pt_write_mod()` returns `NULL` invisibly.

## Author

Pepijn de Vries

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())
```
