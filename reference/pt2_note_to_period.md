# Get a corresponding period value from a note string

Back in the days, ProTracker was hardware driven on a Commodore Amiga.
It made advantage of a custom chipset where each chip had specific
tasks. One of the chips (named Paula) could play 8 bit audio samples
stored in memory directly to one of the four audio channels. On that
chip you could set the integer 'period' value which is inversely related
to the sample rate at which the sample is played. Hence, it defines the
pitch of the sample. ProTracker used the period value to play different
notes. With this function you can convert a `character` string
representing a note to its corresponding period value used by Paula.

## Usage

``` r
pt2_note_to_period(note, empty_char = "-", finetune = 0, ...)
```

## Arguments

- note:

  A `character` string representing notes (see also
  [`pt2_note()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_note.md)).

- empty_char:

  A `character` that is used to represent empty values.

- finetune:

  ProTracker used `integer` `finetune` values to tweak the playback
  rate. it should be in the range of `-8`, up to `+7`.

- ...:

  Ignored.

## Value

Returns a `vector` of `integer` period values.

## Examples

``` r
pt2_note_to_period(c("A#2", "C-1"))
#> [1] 240 856
```
