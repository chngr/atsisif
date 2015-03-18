# All the Same, I Saw it First

Currently contained here is a piece to visualize x-vectors. Edges between nodes
are coloured based on their weight, where darker edges have weight close to 1,
redder edges have weight close to 0.5 and bluer edges have weight close to 0.

There are several interactions that are currently possible:
  - A slider allows one to hide edges of weights higher than some value;
  - The '''h''' key toggles a selection mode which allows one to add nodes into
    a "handle" set;
  - The '''a''' key adds a "tooth";
  - The '''t''' key toggles a selection mode which allows one to add nodes into
    the current "tooth";
  - The '''n''' key shifts the tooth selection up one;
  - The '''b''' key shifts the tooth selection down one;
  - The '''c''' key clears all the selections so far.
No restrictions are currently in place making sure that the handle sets and the
teeth are valid. In particular, nothing checks that a selection is actually a
comb.

## Misc Notes
1. Cycles consisting entirely of fractional nodes seem to be a good sign that
   there is a violated comb inequality nearby. Specifically, if one takes a
   subgraph which is "bounded" by a fractional cycle as a handle and then all
   the extrusions out of this subgraph are taken as teeth, then this might be a
   candidate for a comb inequality.
