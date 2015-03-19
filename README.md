# All the Same, I Saw it First

Currently contained here is a piece to visualize x-vectors. Edges between nodes
are coloured based on their weight, where darker edges have weight close to 1,
redder edges have weight close to 0.5 and bluer edges have weight close to 0.

There are several interactions that are currently possible:
  - A slider allows one to hide edges of weights higher than some value;
  - The ```h``` key toggles a selection mode which allows one to add nodes into
    a "handle" set;
  - The ```a``` key adds a "tooth";
  - The ```t``` key toggles a selection mode which allows one to add nodes into
    the current "tooth";
  - The ```n``` key shifts the tooth selection up one;
  - The ```b``` key shifts the tooth selection down one;
  - The ```c``` key clears all the selections so far.
No restrictions are currently in place making sure that the handle sets and the
teeth are valid. In particular, nothing checks that a selection is actually a
comb.

## Misc Notes
1. Cycles consisting entirely of fractional nodes seem to be a good sign that
   there is a violated comb inequality nearby. Specifically, if one takes a
   subgraph which is "bounded" by a fractional cycle as a handle and then all
   the extrusions out of this subgraph are taken as teeth, then this might be a
   candidate for a comb inequality.
2. When starting out with a handle, one can add to it all the nodes that are
   connected to at least two nodes in the handle so far. This will decrease the
   outward flow of a handle.
3. It appears that things nodes on the boundary of the handle should have
   out degree from the comb of at most 1. Moreover, this single thing coming
   outside of the handle should be included as a tooth.
4. Need the edge at the extremes of teeth to be of weight 1.
5. Things are complicated when the exterior of the handle is given by nodes
   connected to something with very small edge weight.
6. One thing that is useful when slowly constructing combs is to add paths that
   are joined to pairs of nodes that are already in the handle.

## Ideas for Heuristics
In order for a comb inequality to be violated:
  - For every node in the handle that is adjacent to things on the outside, the
    combined weight of all these edges is not much more than 1;
  - Every edge coming out of the handle with significant weight must be included
    as a tooth;
  - The final edge at the very end of a tooth should have weight close to 1 so
    that the weight leaking out of the tooth on the end not connected to the
    handle is close to 1.
In the ideal case when all the leaking edges from the teeth are 1 and the all
the delta edges from the handle have weight 1, we clearly have a violated comb
inequality.

The above ideas become messy when there are edges that have weight close to 0.
To try simplifying things, then, it may be useful to round up and down edge
weights: edge weights below some threshold are rounded down to 0 and edge
weights above the complementary threshold are taken to be 1. The resulting
simplified graph may not actually satisfy subtour inequalities, but might allow
one to find violated comb inequalities in the reduced graph that might extend up
to the bigger graph once things are readjusted.

In fact, one can probably prove that, if the rounding threshold is not too high,
then combs of certain sizes in the reduced graph will be combs in the bigger
graph.
One of the main problems, it seems, is that lots of low weight edges leak out of
the handle and this adds up.

### Observation From Running Heuristic by Hand

This is seemingly useful procedure so far:
  1. Begin by setting lower threshold to around 0.5 and upper threshold to 0.99;
  2. Look for nontrivial connected components in the resulting induced graph
     and let these be the initial candidates for handles.
  3. Up the upper threshold back to 1 and add teeth: just all the nodes that are
     adjacent to the comb so far with edge weight 1.
  4. Lower the upper threshold back to 0.99 and decrease the lower threshold
     ever so slightly. This will lead to expanding of the handle, typically.
  5. Now repeat 3 and 4 until the lower is all the way to 0 and upper is 1.
One thing that takes place while varying the lower threshold is that combs that
used to be separate come together. Moreover, it appears that things that used to
be teeth might also join the handle.
