var KEYCODE = {'h': 72, 'c': 67, 't': 84, 'n': 78, 'b': 66, 'd': 68, 'a': 65},
    TOGGLES = {'handle': false, 'tooth': false},
    GRAPH = {};

function toggle(t) {
  var toggle_keys = Object.keys(TOGGLES), key, i;
  for(i in toggle_keys) {
    key = toggle_keys[i];
    TOGGLES[key] = key != t ? false : !TOGGLES[key];
  }
}

function update_display(comb, edges) {
  for(var i = 0; i < comb.teeth.length; i++) {
    d3.select("#tooth_" + i).text(comb.teeth[i]);
  }
  d3.select("#handle_nodes").text(comb.handle);
  var comb_sum, rhs = 3 * comb.teeth.length + 1;
  if(edges) {
    comb_sum = get_delta_weight(comb.handle, edges) +
      comb.teeth.reduce(function(acc, x) {
        return acc + get_delta_weight(x, edges);
      }, 0);
  }
  d3.select("#comb_weight_sum").text(comb_sum);
  d3.select("#comb_difference").text(rhs - comb_sum)
}

function tooth_colours(comb) {
  d3.select("#tooth_" + comb.prev_t + "_label")
    .style("background", "white");
  d3.select("#tooth_" + comb.cur_t + "_label")
    .style("background", "yellow");
}

function reset_comb(comb, vis) {
  for(var i = 0; i < comb.teeth.length; i++) {
    d3.select("#tooth_" + i + "_container").remove();
  }
  comb.cur_t = comb.prev_t = -1;
  comb.teeth = comb.handle = [];
  update_display(comb);
  vis.selectAll('.handle').style("fill", "red").classed('handle', false);
  vis.selectAll('.tooth').style("stroke", "black").classed("tooth", false);
}

function add_tooth(comb, t_cols) {
  comb.prev_t = comb.cur_t;
  comb.cur_t += 1;
  comb.teeth.push([]);

  var new_name = "tooth_" + comb.cur_t + ": ",
      new_elt = d3.select("#teeth")
                  .append("p")
                  .attr("id", "tooth_" + comb.cur_t + "_container");
  new_elt.append("span")
    .attr("id", "tooth_" + comb.cur_t + "_label")
    .style("color", t_cols(comb.cur_t))
    .text(new_name);
  new_elt.append('span').attr('id', "tooth_" + comb.cur_t);
  tooth_colours(comb);
}
function next_tooth(comb) {
  comb.prev_t = comb.cur_t;
  comb.cur_t = comb.cur_t < comb.teeth.length ? comb.cur_t + 1 : comb.cur_t;
  tooth_colours(comb);
  console.log("Current tooth: " + comb.cur_t);
}
function back_tooth(comb) {
  comb.prev_t = comb.cur_t;
  comb.cur_t = comb.cur_t > 0 ? comb.cur_t - 1 : comb.cur_t;
  tooth_colours(comb.prev_t);
  console.log("Current tooth: " + comb.cur_t);
}
function get_delta_weight(V, edges) {
  return edges.reduce(function(a, e) {
    return (V.indexOf(e.a) > -1) + (V.indexOf(e.b) > -1) == 1 ? a + e.weight : a;
  }, 0);
}

function draw_graph() {
  var width = 1000, height = 600,
      edges = [],
      comb = {
        handle: [],
        teeth: [],
        cur_t: -1,
        prev_t: -1,
      },
      pan_enabled = true;

  var colour = d3.scale.linear()
                  .domain([0.0,0.5,1.0])
                  .range(["cornflowerblue", "red", "black"]),
      t_cols = d3.scale.linear()
                  .domain([0,5,10,15])
                  .range(["green", "magenta", "yellow", "orange"]),
      xScale = d3.scale.linear()
                  .domain([0,width])
                  .range([0,width]),
      yScale = d3.scale.linear()
                  .domain([0,height])
                  .range([0,height]);

  var force = d3.layout.force()
                .charge(-10)
                .linkDistance(3)
                .size([width,height]);

  var svg = d3.select("#force_graph")
              .attr("tabindex", 1)
              .on("keydown", keydown)
              .append("svg")
              .attr("width", width)
              .attr("height", height);

  var zoomer = d3.behavior.zoom()
                .scaleExtent([0.1,10])
                .x(xScale)
                .y(yScale)
                .on("zoom", zoom);

  var svg_graph = svg.append("svg:g").call(zoomer),
      rect = svg_graph.append("svg:rect")
              .attr("width", width)
              .attr("height", height)
              .attr("fill", "transparent")
              .attr("stroke", "transparent")
              .attr("stroke-width", 1)
              .attr("id", "zrect"),
      vis = svg_graph.append("svg:g")
              .attr("fill", "red")
              .attr("stroke", "black")
              .attr("stroke-width", 1)
              .attr("opacity", 0.5)
              .attr("id", "vis");
  var link = vis.append("g").attr("class", "link").selectAll("line"),
      node = vis.append("g").attr("class", "node").selectAll("circle");

  d3.json("/data/c.json", function(err, g) {
    var n = g.nodes, nodes = [];
    for(var i = 0; i < n; i++) { nodes.push({name: i}); }
    GRAPH = g;
    GRAPH.nodes = nodes;
    GRAPH.a_edges = GRAPH.edges;

    force.nodes(nodes).links(GRAPH.a_edges).start();
    edges = force.links().map(function(e) {
      return {a: e.source.index, b: e.target.index, weight: e.value };
    });

    link = link.data(GRAPH.edges)
               .enter().append("line")
               .attr("class", "link")
               .style("stroke", function (d) { return colour(d.value); });
    node = node.data(nodes)
               .enter().append("circle")
               .attr("class", "node")
               .attr("r", 2)
               .call(node_drag)
               .on("dblclick", releasenode)
               .on("click", node_click);

    force.on("tick", function() {
      link.attr("x1", function(d) { return d.source.x; })
          .attr("y1", function(d) { return d.source.y; })
          .attr("x2", function(d) { return d.target.x; })
          .attr("y2", function(d) { return d.target.y; });
      node.attr("cx", function(d) { return d.x; })
          .attr("cy", function(d) { return d.y; });
    });
  });


  d3.select("#weight").on("input", function() {
    var wt = this.value;
    GRAPH.a_edges = GRAPH.edges.filter(function (e) {return 100 * e.value <= wt;});
    d3.select("#weight").property("value", wt);
    link = link.data(GRAPH.a_edges);
    link.exit().remove();
    link.enter().insert("line").style("stroke", function(d) { return colour(d.value); })
    node = node.data(GRAPH.nodes);
    node.exit().remove();
    node.enter().insert("circle")
        .attr("class", "node")
        .attr("r", 2)
        .call(force.drag)
        .on("click", node_click);
    force.nodes(GRAPH.nodes).links(GRAPH.a_edges).start();
  });

  /* Event handlers */
  function keydown() {
    switch(d3.event.keyCode) {
      case KEYCODE.c:
        reset_comb(comb, vis);
        break;
      case KEYCODE.h:
        toggle('handle');
        break;
      case KEYCODE.t:
        toggle('tooth');
        break;
      case KEYCODE.n:
        next_tooth(comb);
        break;
      case KEYCODE.b:
        back_tooth(comb);
        break;
      case KEYCODE.a:
        add_tooth(comb, t_cols);
        break;
    }
  }

  function zoom() {
    if(pan_enabled) {
      svg.attr("transform", "translate("+d3.event.translate+")scale("+d3.event.scale+")");
    }
  }

  function node_click(d) {
    if(d3.event.defaultPrevented) return;
    if(TOGGLES.handle) {
      d3.select(this)
        .classed("handle", d.handle = !d.handle)
        .style("fill", function(p) { return (p.handle ? "blue" : "red"); });

      if(d.handle) {
        comb.handle.push(d.index);
      } else {
        comb.handle.splice(comb.handle.indexOf(d.index), 1);
      }
    } else if(TOGGLES.tooth) {
      d3.select(this).classed("tooth", d.tooth = !d.tooth)
        .style("stroke", function(p) { return (p.tooth ? t_cols(comb.cur_t) : "black"); });
      if(d.tooth) {
        comb.teeth[comb.cur_t].push(d.index);
      } else {
        for(var i = 0; i < comb.teeth.length; i++) {
          var idx = comb.teeth[i].indexOf(d.index);
          if(idx > -1) {
            comb.teeth[i].splice(idx, 1);
          }
        }
      }
    }
    update_display(comb, edges);
   };

  var node_drag = d3.behavior.drag()
                    .on("dragstart", dragstart)
                    .on("drag", dragmove)
                    .on("dragend", dragend);
  function dragstart(d, i) {
    force.stop();
    pan_enabled = false;
  }
  function dragmove(d, i) {
    d.px += d3.event.dx;
    d.py += d3.event.dy;
    d.x += d3.event.dx;
    d.y += d3.event.dy;
  }
  function dragend(d, i) {
    d.fixed = true;
    pan_enabled = true;
    force.resume();
  }
  function releasenode(d) {
    d.fixed = false;
  }
}

