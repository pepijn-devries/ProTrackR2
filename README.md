
# ProTrackR2

<!-- badges: start -->
<!-- badges: end -->

<img src="man/figures/logo.svg" align="right" height="139" copyright="cc-sa" alt="logo" class="pkgdown-hide" />

[ProTracker](https://en.wikipedia.org/wiki/Protracker) is music
sequencer software from the 1990s on the [Commodore
Amiga](https://en.wikipedia.org/wiki/Amiga) (see screenshot of version
2.3a on the right). The R package `ProTrackR2` is the successor of
[`ProTrackR`](https://pepijn-devries.github.io/ProTrackR/). It can be
used to play and manipulate Protracker module music.

## Installation

You can install the development version of ProTrackR2 from
[r-universe](https://pepijn-devries.r-universe.dev/ProTrackR2) with:

``` r
install.packages("ProTrackR2", repos = c("https://pepijn-devries.r-universe.dev", "https://cloud.r-project.org"))
```

## ProTracker2.3d clone

<img src="https://content.pouet.net/files/screenshots/00050/00050055.png" alt="ProTracker 2.3a" align="right" />

The ProTrackR2 package essentially provides an R entry point into the
[Protracker2.3d clone](https://16-bits.org/pt2.php) by [Olav Sørensen
(A.K.A. 8bitbubsy)](https://github.com/8bitbubsy) and other
[contributors](https://github.com/8bitbubsy/pt2-clone/graphs/contributors)
([source code](https://github.com/8bitbubsy/pt2-clone)). For this
purpose, I took the code by Sørensen and stripped away the graphical
user interface (GUI) and access to the audio device. Next I added R
bindings to access functionality to read, write, manipulate and render
Protracker modules. This allows you to process such modules with R
scripts and in batches. If you wish to use the the GUI I suggest to
visit one of the links above and check out the work of Olav Sørensen.

The work by Olav Sørensen is gratefully acknowledged here, without which
this package would have not been feasible.

## Example

The example below shows how to load a Protracker module file and play it
in R:

``` r
library(ProTrackR2)

mod <- pt2_read_mod("https://api.modarchive.org/downloads.php?moduleid=41529#elektric_funk.mod")

play(mod)
```

## Code of Conduct

Please note that the ProTrackR2 project is released with a [Contributor
Code of
Conduct](https://contributor-covenant.org/version/2/1/CODE_OF_CONDUCT.html).
By contributing to this project, you agree to abide by its terms.
