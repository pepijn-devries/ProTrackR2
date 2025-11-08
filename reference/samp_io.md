# Read and write ProTracker audio samples

Functions to read and write ProTracker audio samples. Reading is
supported for common types of WAV, IFF and AIFF files. Writing is
supported for WAV and IFF files.

## Usage

``` r
pt2_read_sample(file, ...)

pt2_write_sample(sample, file, ...)
```

## Arguments

- file:

  Filename of the file to read from or write to. For
  `pt2_write_sample()` the file extension will be used to determine
  which file format to write.

- ...:

  Ignored

- sample:

  An object of class `pt2samp`.

## Value

`pt2_read_sample()` returns a `pt2samp` class object when successful.
`pt_write_sample()` returns `NULL` invisibly.

## Author

Pepijn de Vries

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())
my_sample <- pt2_sample(mod, 1L)
my_sample_file <- tempfile(fileext = ".iff")
pt2_write_sample(my_sample, my_sample_file)
```
