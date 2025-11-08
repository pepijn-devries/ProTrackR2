# Obtain sample data and info from a ProTracker module

Obtain sample data and info from a ProTracker module at a specific
index. ProTracker modules can hold up to 31 samples. The index should
range from 0 to 30.

## Usage

``` r
pt2_sample(mod, i, ...)
```

## Arguments

- mod:

  An object of class `pt2mod` from which to obtain sample data and
  information

- i:

  The index of the requested sample (between 0 and 30).

- ...:

  Ignored.

## Value

Returns a sample object of class `pt2samp`.

## Author

Pepijn de Vries

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())

smp <- pt2_sample(mod, 0L)
```
