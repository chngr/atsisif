var KEYCODE = {'h': 72, 'c': 67, 't': 84, 'n': 78, 'b': 66, 'd': 68, 'a': 65},
    TOGGLES = {'handle': false, 'tooth': false},
    GRAPH = {};

function contract_edges(graph) {
  var c_edges = [], f_edges = [], node_flags = [], nodes = [],
      in_path = false,
      e, p, i, j;
  for(e of graph.edges) {
    in_path = false;
    if(e.value == 1) {
      for(p of c_edges) {
        if(e.source == p.source) {
          p.source = e.target;
          in_path = true;
        } else if(e.source == p.target) {
          p.target = e.target;
          in_path = true;
        } else if(e.target == p.source) {
          p.source = e.source;
          in_path = true;
        } else if(e.target == p.target) {
          p.target = e.source;
          in_path = true;
        }
        if(in_path) { break; }
      }
      if(!in_path) {
        c_edges.push({ source: e.source, target: e.target, value: 1, weight: 1});
      }
    } else {
      f_edges.push({ source: e.source, target: e.target, value: e.value, weight: e.weight});
    }
  }
  var edges = c_edges.concat(f_edges);
  for(i = 0; i < graph.nodes.length; i++) { node_flags.push(0); }
  for(e of edges) {
    node_flags[e.source] = 1;
    node_flags[e.target] = 1;
  }
  for(i = 0, j = 0; i < graph.nodes.length; i++) {
    if(node_flags[i]) {
      nodes.push({name: j});
      node_flags[i] = j;
      j += 1;
    }
  }
  for(e of edges) {
    e.source = node_flags[e.source];
    e.target = node_flags[e.target];
  }
  return {nodes: nodes, edges: edges};
}

function toggle(t) {
  var toggle_keys = Object.keys(TOGGLES), key, i;
  for(i in toggle_keys) {
    key = toggle_keys[i];
    d3.select("#" + key + "_toggle").style("background", "white");
    TOGGLES[key] = key != t ? false : !TOGGLES[key];
  }
  if(TOGGLES[t]) {
    d3.select("#" + t + "_toggle").style("background", "green");
  }
}

function update_display(comb) {
  for(var i = 0; i < comb.teeth.length; i++) {
    d3.select("#tooth_" + i).text(comb.teeth[i]);
  }
  d3.select("#handle_nodes").text(comb.handle);
  var comb_sum, rhs = 3 * comb.teeth.filter(function(d) {return d.length > 0;}).length + 1;
  comb_sum = get_delta_weight(comb.handle) +
    comb.teeth.reduce(function(acc, x) {
      return acc + get_delta_weight(x);
    }, 0);
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
  comb.teeth = [];
  comb.handle = [];
  update_display(comb);
  vis.selectAll('.handle').style("fill", "red").classed('handle', false);
  vis.selectAll('.tooth').style("stroke", "black").classed("tooth", false);
  vis.selectAll('.node').call(function(d) { d.fixed = false; });
}

function add_tooth(comb, t_cols) {
  comb.teeth.push([]);
  var new_idx = comb.teeth.length - 1,
      new_name = "tooth_" + new_idx + ": ",
      new_elt = d3.select("#teeth")
                  .append("span")
                  .attr("id", "tooth_" + new_idx + "_container");
  new_elt.append("span")
    .attr("id", "tooth_" + new_idx + "_label")
    .style("color", t_cols(new_idx))
    .text(new_name);
  new_elt.append('span').attr('id', "tooth_" + new_idx);

  if(comb.teeth.length - comb.cur_t == 2) {
    comb.prev_t = comb.cur_t;
    comb.cur_t += 1;
    tooth_colours(comb);
  }
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
  tooth_colours(comb);
  console.log("Current tooth: " + comb.cur_t);
}
function get_delta_weight(V) {
  var val = GRAPH.edges.reduce(function(a, e) {
    if((V.indexOf(e.source.index) != -1) + (V.indexOf(e.target.index) != -1) == 1) {
      return a + e.value;
    } else {
      return a;
    }
  }, 0);
  return val;
}

function draw_graph(data) {
  var width = 1000, height = 600,
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

  d3.json("data/test.json", function(err, g) {
    var n = g.nodes, nodes = [];
    for(var i = 0; i < n; i++) { nodes.push({name: i}); }
    GRAPH = contract_edges({nodes: nodes, edges: g.edges});
    GRAPH.a_edges = GRAPH.edges;
    GRAPH.upper_value = 100;
    GRAPH.lower_value = 0;

    force.nodes(GRAPH.nodes).links(GRAPH.a_edges).start();

    link = link.data(GRAPH.a_edges)
               .enter().append("line")
               .attr("class", "link")
               .style("stroke", function (d) { return colour(d.value); });
    node = node.data(GRAPH.nodes)
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


  function update_value() {
    GRAPH.a_edges = GRAPH.edges.filter(function(e) {
      var ewt = 100 * e.value
      return GRAPH.lower_value <= ewt && ewt <= GRAPH.upper_value ? true : false;
    });
    link = link.data(GRAPH.a_edges);
    link.exit().remove();
    link.enter().insert("line")
      .attr("class",  "link")
      .style("stroke", function(d) { return colour(d.value); });
    node = node.data(GRAPH.nodes);
    node.exit().remove();
    node.enter().insert("circle")
        .attr("class", "node")
        .attr("r", 2)
        .call(force.drag)
        .on("click", node_click);
    force.nodes(GRAPH.nodes).links(GRAPH.a_edges).start();
  }

  d3.select("#upper_value").on("input", function() {
    GRAPH.upper_value = this.value;
    d3.select("#upper_value").property("value", this.value);
    d3.select("#upper_value_value").text(this.value / 100);
    update_value();
  });
  d3.select("#lower_value").on("input", function() {
    GRAPH.lower_value = this.value;
    d3.select("#lower_value").property("value", this.value);
    d3.select("#lower_value_value").text(this.value / 100);
    update_value();
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
    update_display(comb);
  }

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
