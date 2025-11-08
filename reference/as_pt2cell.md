# Convert object into a pt2cell or pt2celllist class object

Protracker uses pattern tables to annotate music progression. Each cell
in the table contains information about the note, sample identifier and
sound effect. This function coerces objects to `pt2cell` or `pt2cell`
class objects, such that they can be inserted into patterns.

## Usage

``` r
as_pt2cell(x, ...)

as_pt2celllist(x, ...)
```

## Arguments

- x:

  A (`vector` of) `character` string(s), to be coerced to a `pt2cell`
  class object. The first two or three characters represent the note
  (see
  [`pt2_note()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_note.md)).
  this is followed by two numerical characters representing the sample
  number. And finally three hexadecimal characters representing an
  effect or trigger. These three parts can optionally be padded with
  spaces. A valid string would for instance be `"C#2 01 C1A`.

- ...:

  Ignored

## Value

An object of class `pt2cell`, `pt2celllist`.

## Examples

``` r
as_pt2cell("A-3 02 121")
#> A-3 02 121
as_pt2cell("--- 01 000")
#> --- 01 000
as_pt2celllist(c("A-3 02 121", "--- 01 000"))
#> cell list [n=2]
```
