# Effect commands

## What are effect commands

A pattern table is composed of cells telling the tracker which notes to
play in which order. It also has a field for effect commands. It is
often represented as a three digit hexadecimal value. The first hex
digit represents the effect command, the latter represent parameter
values used by the command. The table below (based on
`data(effect_commands)`) summarises all available effects in ProTracker.

More details on the tracker pattern structure can be found in
[`vignette("s3class")`](https://pepijn-devries.github.io/ProTrackR2/articles/s3class.md).
Some examples of the effects that can be applied are [provided
below](#examples). Note that this vignette does not intend to be a
complete tutorial. It merely addresses how you can manipulate ProTracker
modules programmatically in R. For more in depth information please
check the [references](#references).

| Code | Effect                       | Description                                                                                                                                                                                                                                                                                                                                            |
|:-----|:-----------------------------|:-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 0xy  | Arpeggio                     | This effect alternates the pitch each tick to simulate a chord. xy needs to be greater than 00. First the specified note is played, then the pitch is increased with x semitones, then with y semitones.                                                                                                                                               |
| 1xy  | Porta up                     | Decrease the period value with xy every tick but the first.                                                                                                                                                                                                                                                                                            |
| 2xy  | Porta down                   | Increase the period value with xy every tick but the first.                                                                                                                                                                                                                                                                                            |
| 3xy  | Porta to note                | Change the period value with xy every tick but the first, untill the specified target note is reached.                                                                                                                                                                                                                                                 |
| 4xy  | Vibrato                      | Oscillate the pitch with magnitude x. Where y relates to the oscillation frequency.                                                                                                                                                                                                                                                                    |
| 5xy  | Porta to note + Volume slide | A combination of porta to note (3xy) and volume slide (Axy).                                                                                                                                                                                                                                                                                           |
| 6xy  | Vibrato + Volume slide       | A combination of vibrato (4xy) and volume slide (Axy).                                                                                                                                                                                                                                                                                                 |
| 7xy  | Tremolo                      | Oscillate the volume with magnitude x. Where y relates to the oscillation frequency.                                                                                                                                                                                                                                                                   |
| 8xy  | Not implemented              | This effect command is not implemented in ProTracker, nor will it be in this package.                                                                                                                                                                                                                                                                  |
| 9xy  | Set sample offset            | This effect causes the note to start playing at an offset (of 256 times xy samples) into the sample, instead of just from the start.                                                                                                                                                                                                                   |
| Axy  | Volume slide                 | Change the volume every but the first tick: increase with x, decrease with y.                                                                                                                                                                                                                                                                          |
| Bxy  | Position jump                | Jump to position xy of the pattern sequence table.                                                                                                                                                                                                                                                                                                     |
| Cxy  | Set volume                   | Set the volume with xy.                                                                                                                                                                                                                                                                                                                                |
| Dxy  | Pattern break                | Break to row xy in the next pattern. Note: xy is (even though it is a hexadecimal) interpreted as a decimal.                                                                                                                                                                                                                                           |
| E0x  | Turn filter on/off           | If x is even, the (emulated) hardware filter is turned on (for all tracks). It is turned off if x is odd.                                                                                                                                                                                                                                              |
| E1x  | Porta up (fine)              | The period value is decreased with x, at the first tick.                                                                                                                                                                                                                                                                                               |
| E2x  | Porta down (fine)            | The period value is increased with x, at the first tick.                                                                                                                                                                                                                                                                                               |
| E3x  | Glissando Control            | This effect causes a change in the effect 3xy (porta to note). It toggles whether to do a smooth slide or whether to slide in jumps of semitones. When x is 0 it uses a smooth slide, non-zero values will result in jumps.                                                                                                                            |
| E4x  | Vibrato Waveform             | This effect sets the waveform for the vibrato command to follow. With x modulo 4 equals 0, a sine wave is used, with 1 ramp down, with 2 or 3 a square wave. Values greater than 4 causes the ossicating waveform not to retrigger it when a new note is played.                                                                                       |
| E5x  | Set finetune                 | Set the finetune with x, where x is interpreted as a signed nybble.                                                                                                                                                                                                                                                                                    |
| E6x  | Pattern loop                 | Set pattern loop start with E60, and loop x times when x is non-zero.                                                                                                                                                                                                                                                                                  |
| E7x  | Tremolo waveform             | Same as E4x, but this controls the wave form for the tremolo effect (7xy) rather than the vibrato effect.                                                                                                                                                                                                                                              |
| E8x  | Not implemented              | According to official documentation this command is not implemented in ProTracker, but it is. Applies a filter on a looped sample, therewith destroying the original sample data. In many cases this effect is not used like this, rather programmers used this to trigger certain (visual) effects and synchronise those with the music being played. |
| E9x  | Retrigger note               | Retrigger the note every x-th tick.                                                                                                                                                                                                                                                                                                                    |
| EAx  | Volume slide up (fine)       | Increase the volume with x at the first tick.                                                                                                                                                                                                                                                                                                          |
| EBx  | Volume slide down (fine)     | Decrease the volume with x at the first tick.                                                                                                                                                                                                                                                                                                          |
| ECx  | Cut note                     | Cut the volume of the note to zero after x ticks.                                                                                                                                                                                                                                                                                                      |
| EDx  | Delay note                   | The note is triggered with a delay of x ticks.                                                                                                                                                                                                                                                                                                         |
| EEx  | Pattern delay                | The duration of the row in ticks is multiplied by (x + 1).                                                                                                                                                                                                                                                                                             |
| EFx  | Not implemented              | According to official documentation this command is not implemented in ProTracker, but it is. It flips sample data in a looped sample, therewith destroying the original sample data.                                                                                                                                                                  |
| Fxy  | Set speed or tempo           | When xy is smaller then 32, it sets the speed in ticks per row. When xy is greater than 31, it will set the tempo, wich is inversely related to the duration of each tick. Speed and tempo can be defined in combination.                                                                                                                              |

## Extracting and updating effect commands

To get an effect command from a ProTracker module, you can simply call
[`pt2_command()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_command.md)
to a `pt2cell` or `pt2celllist` class object (see
[`vignette("s3class")`](https://pepijn-devries.github.io/ProTrackR2/articles/s3class.md)
for a ProTrackR2 object tree). There, are several ways of extracting
cells as documented in
[`vignette("sel_assign")`](https://pepijn-devries.github.io/ProTrackR2/articles/sel_assign.md).
In the example below there is one way shown of getting the effect
commands of the first 8 rows of the second column in the third pattern.

``` r
## load the demo-mod included with the package
mod <- pt2_read_mod(pt2_demo())

pt2_command(
  mod$patterns[[ 3 ]] [ 1:8, 2 ])
#> C40
#> C30
#> 000
#> 000
#> 000
#> 000
#> 000
#> 000
```

You can also assign new effect commands to your selection like this:

``` r
pt2_command(
  mod$patterns[[ 3 ]] [ 1:8, 2 ]) <- "C10"
```

## Examples of some common effects

Using the description in the [table above](#what), and the assignment
procedure [explained above](#updating), some examples are implemented
below:

``` r

## create a new mod te demonstrate commands
mod_command <- pt2_new_mod("commands")
## copy the samples from the mod above
mod_command$samples <- mod$samples

if (interactive()) {
  mod_command$patterns[[1]][1,1] <- "C-3 01 000"
  ## Reduce sample volume
  pt2_command(mod_command$patterns[[1]][,1]) <- "C10"
  play(mod_command)
  
  mod_command$patterns[[1]][1,1] <- "C-3 03 000"
  ## Simulate major chord:
  pt2_command(mod_command$patterns[[1]][,1]) <- "047"
  play(mod_command)
  
  ## Simulate minor chord
  pt2_command(mod_command$patterns[[1]][,1]) <- "037"
  play(mod_command)

  ## Porta up:
  pt2_command(mod_command$patterns[[1]][,1]) <- "101"
  play(mod_command)

  ## Porta down:
  pt2_command(mod_command$patterns[[1]][,1]) <- "201"
  play(mod_command)
  
  ## Porta to note:
  mod_command$patterns[[1]][1,1] <- "C-3 03 000"
  mod_command$patterns[[1]][2:64,1] <- "E-3 03 000"
  pt2_command(mod_command$patterns[[1]][2:64,1]) <- "301"
  play(mod_command)
  
  mod_command$patterns[[1]] <- pt2_new_pattern() # reset pattern
  mod_command$patterns[[1]][1,1] <- "C-3 03 000"
  ## Vibrato:
  pt2_command(mod_command$patterns[[1]][,1]) <- "464"
  play(mod_command)
  
  ## Tremolo:
  pt2_command(mod_command$patterns[[1]][,1]) <- "743"
  play(mod_command)

  ## Fade out:
  pt2_command(mod_command$patterns[[1]][,1]) <- "A01"
  play(mod_command)

  mod_command$patterns[[1]][1,1] <- "C-3 03 C00"
  ## Fade in:
  pt2_command(mod_command$patterns[[1]][2:63,1]) <- "A10"
  play(mod_command)
}
```

## Effect of instrument number on effects

The cells in a ProTracker pattern also hold information on the
instrument number. The function
[`pt2_instrument()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_instrument.md)
can be used to get or set an instrument number. The reason for
mentioning it here is that setting (or omitting) the instrument number
can affect how a
[`pt2_command()`](https://pepijn-devries.github.io/ProTrackR2/reference/pt2_command.md)
is interpreted.

By setting the instrument number in combination with an effect command,
can cause the player to reset the sample’s previous state. I’m not going
to show all features as there are many. But just as an example see what
happens when you apply a volume slide in combination with setting the
instrument number. The volume is reset each row of the pattern:

``` r
mod$patterns[[1]] <- pt2_new_pattern()
mod$patterns[[1]][1,1] <- "C-3 03 000"
## Slide the volume down in the entire track:
pt2_command(mod$patterns[[1]][,1]) <- "A08"
## Reset the volume each row by setting the sample
## number for the entire track
pt2_instrument(mod$patterns[[1]][,1]) <- 3L
if (interactive()) {
  play(mod)
}
```

## References

- [YouTube
  tutorial](https://www.youtube.com/playlist?list=PLVoRT-Mqwas9gvmCRtOusCQSKNQNf6lTc)
- [OpenMPT documentation on
  effects](https://wiki.openmpt.org/Manual:_Effect_Reference)
