# Validate ProTrackR2 S3 class objects

Check aspects of S3 class objects for validity. For samples for instance
it is checked if all parameters (volume, finetune, etc.) are within
ProTracker specifications.

## Usage

``` r
pt2_validate(x, ...)

# S3 method for class 'pt2samp'
pt2_validate(x, ...)
```

## Arguments

- x:

  object to be validated

- ...:

  Ignored

## Value

A `logical` value indicating whether the object is valid or not

## Author

Pepijn de Vries

## Examples

``` r
mod <- pt2_read_mod(pt2_demo())

pt2_validate(mod$samples[[1]])
#> [1] TRUE
```
