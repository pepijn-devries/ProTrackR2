if (requireNamespace(c("DiagrammeR", "DiagrammeRsvg", "xml2", "htmltools"))) {
  library(ProTrackR2)
  library(DiagrammeR)
  library(DiagrammeRsvg)
  library(xml2)
  svgdata <-
    grViz(
      "
digraph 'ProTracker modules' {
rankdir = 'LR';
layout = 'dot';
splines = ortho;
node [fontname = Helvetica shape = box style = 'filled, rounded'
  fillcolor = white class = 'csquare_nodes' fixedsize = true width = 1.8];

mod      [label = 'ProTracker module\n(pt2mod)' group = 'root' class = 'grvzclick'
          URL = '#protracker-module-objects-pt2mod'];
meta     [label = 'meta-data' group = 'root'];
filei    [label = 'file information' group = 'root'];
patord   [label = 'pattern sequence' group = 'pat'];
patlist  [label = 'pattern list\n(pt2patlist)' group = 'pat' class = 'grvzclick'
          URL = '#pattern-list-objects-pt2patlist'];
pat1     [label = 'pattern 1\n(pt2pat)' group = 'pat' class = 'grvzclick'
          URL = '#pattern-objects-pt2pat'];
patn     [label = 'pattern N\n(pt2pat)' class = 'grvzclick'
          URL = '#pattern-objects-pt2pat'];
samplist [label = 'sample list\n(pt2samplist)' group = 'samp' class = 'grvzclick'
          URL = '#sample-list-objects-pt2samplist'];
samp1    [label = 'sample 1\n(pt2samp)' group = 'samp' class = 'grvzclick'
          URL = '#sample-objects-pt2samp'];
sampn    [label = 'sample N\n(pt2samp)' class = 'grvzclick'
          URL = '#sample-objects-pt2samp'];
celllist [label = 'cell list\n(pt2celllist)' group = 'pat' class = 'grvzclick'
          URL = '#cell-list-objects-pt2celllist'];
cell1    [label = 'cell 1\n(pt2cell)' group = 'pat' class = 'grvzclick'
          URL = '#cell-objects-pt2cell'];
celln    [label = 'cell N\n(pt2cell)' class = 'grvzclick'
          URL = '#cell-objects-pt2cell'];

{rank = 'same'; filei; patord; pat1; patn; samp1; sampn}
{rank = 'same'; cell1; celln}

mod -> meta -> filei;
meta -> patord;
mod -> patlist -> pat1 -> celllist -> cell1;
mod -> samplist -> samp1;
pat1->patn [arrowhead = 'none' style = 'dashed'];
samp1->sampn [arrowhead = 'none' style = 'dashed'];
cell1->celln [arrowhead = 'none' style = 'dashed'];
patlist -> patn;
samplist -> sampn;
celllist -> celln
}
") |>
    export_svg() |>
    read_xml() |>
    as.character()
  svgdata <- gsub("scale(1 1)", "scale (0.65 0.65)", svgdata, fixed = TRUE)
  svgdata <- gsub("height=\"314pt\"", "height=\"230pt\"", svgdata, fixed = TRUE)
  svgdata <- gsub("viewBox=\"0.00 0.00 802.00 314.00\"", "viewBox=\"0.00 0.00 802.00 230.00\"",
                  svgdata, fixed = TRUE)
  
  svgdata |>
    htmltools::HTML() |>
    writeLines("man/figures/object-tree.svg")
} else {
  message("install required packages first and try again")
}
