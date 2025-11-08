# Effect commands (data.frame)

ProTracker uses codes to achieve certain effects or jump to a specific
position (see
[`vignette("effect_commands")`](https://pepijn-devries.github.io/ProTrackR2/articles/effect_commands.md)).

## Format

`effect_commands` is a `data.frame` with three columns:

- `Code`: a `character` of pseudo code used to achieve a certain effect
  in ProTracker

- `Effect`: Name of the effect.

- `Description`: Description of the effect.

It is used for documentation and reference purposes.

## Examples

``` r
data("effect_commands")
```
