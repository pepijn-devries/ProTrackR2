# Create a new (empty) ProTracker module

Creates an empty ProTracker module, it is returned as a `pt2mod` class
object.

## Usage

``` r
pt2_new_mod(name, ...)
```

## Arguments

- name:

  Name for the new module. It will be truncated if longer than 20
  characters.

- ...:

  Ignored

## Value

A `pt2mod` class module, with no samples and one empty pattern.

## Author

Pepijn de Vries

## Examples

``` r
mod <- pt2_new_mod("my_song")
```
