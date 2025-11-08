# Obtain ProTracker module information

Obtain information about a ProTracker module or embedded samples.

## Usage

``` r
pt2_length(mod, ...)

pt2_length(mod, ...) <- value

pt2_n_pattern(mod, ...)

pt2_pattern_table(mod, ...)

pt2_pattern_table(mod, ...) <- value

pt2_name(x, ...)

pt2_name(x, ...) <- value

# S3 method for class 'pt2mod'
pt2_name(x, ...)

# S3 method for class 'pt2mod'
pt2_name(x, ...) <- value

# S3 method for class 'pt2samp'
pt2_name(x, ...)

# S3 method for class 'pt2samp'
pt2_name(x, ...) <- value

# S3 method for class 'pt2samplist'
pt2_name(x, ...)

# S3 method for class 'pt2samplist'
pt2_name(x, ...) <- value

pt2_n_sample(mod, ...)
```

## Arguments

- ...:

  Ignored

- value:

  Replacement value. In case of:

  - `pt2_length<-`: new length of a module in number of patterns in the
    pattern

  - `pt2_pattern_table<-`: a new patter table. A 128 long vector of
    integers between 0 and 99.

  - `pt2_name<-`: a new name (or names) for x

- x, mod:

  A `pt2mod` class object for which to obtain information. For `x` also
  samples of class `pt2samp` are allowed as input.

## Value

Returns information about the specified ProTracker module

## Details

You can use the following functions to get or set information on a
ProTracker modules (represented by `pt2mod` class objects):

- `pt2_length()`: get or set the length the pattern table.

- `pt2_n_pattern()`: number of distinct patterns. Same as
  `length(mod$patterns)`.

- `pt2_pattern_table()`: get or set table of pattern indexes. Patterns
  will be played in this order. Normally, only the first `pt2_length()`
  number of listed patterns are played. Patterns beyond this length are
  only played if you explicitly start the module at that position, or
  the module contains jump commands.

- `pt2_name()`: get or set the name of a module. Can also be used on
  samples in a module. Names in a ProTracker module are limited. Module
  names are automatically truncated to 20 UTF8 characters. For samples,
  the names are truncated to 22 characters.

## Author

Pepijn de Vries

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())

pt2_length(mod)
#> [1] 9
pt2_n_pattern(mod)
#> [1] 4
pt2_n_sample(mod)
#> [1] 4
pt2_pattern_table(mod)
#>   [1] 0 0 1 1 2 2 3 3 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
#>  [38] 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
#>  [75] 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
#> [112] 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
pt2_name(mod)
#> [1] "intro"
pt2_name(pt2_sample(mod, 4L))
#> [1] "commodore amiga 500"

mod2 <- pt2_new_mod("new")
pt2_length(mod2) <- 3L
pt2_pattern_table(mod2)[1L:3L] <- c(0L, 2L, 1L)
pt2_name(mod2) <- "foobar"
```
