# Format a ProTracker pattern conform OpenMPT specs

[OpenMpt](https://openmpt.org/) is a popular modern music tracker. This
function allows you to format a pattern such that it can be pasted
directly into OpenMPT. On Windows you can use `writeClipboard()` for
this purpose.

## Usage

``` r
as_modplug_pattern(pattern, ...)
```

## Arguments

- pattern:

  An object of class `pt2pat` to be formatted

- ...:

  Ignored

## Value

Returns a `character` object formatted such that it can be copied into
OpenMPT

## Author

Pepijn de Vries

## Examples

``` r
mod    <- pt2_read_mod(pt2_demo())
mp_pat <- as_modplug_pattern(pt2_pattern(mod, 0L))
```
