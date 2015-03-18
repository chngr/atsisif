var KEYCODE = {'h': 72, 'c': 67, 't': 84, 'n': 78, 'b': 66, 'd': 68, 'a': 65};

function draw_graph() {
  var width = 1000,
      height = 600,
      edges = [],
      toggles = {'handle': false, 'tooth': false},
      handle = [],
      previous_tooth = -1,
      current_tooth = -1,
      teeth = [];

  var colour = d3.scale.linear().domain([0.0,0.5,1.0]).range(["cornflowerblue", "red", "black"]),
      teeth_colours = d3.scale.linear().domain([0,5,10,15]).range(["green", "magenta", "yellow", "orange"]),
      xScale = d3.scale.linear().domain([0,width]).range([0,width]),
      yScale = d3.scale.linear().domain([0,height]).range([0,height]);

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

  function zoom() {
    svg.attr("transform", "translate("+ d3.event.translate +")scale("+ d3.event.scale +")");
  }

  var svg_graph = svg.append("svg:g").call(zoomer);
  var rect = svg_graph.append("svg:rect")
    .attr("width", width)
    .attr("height", height)
    .attr("fill", "transparent")
    .attr("stroke", "transparent")
    .attr("stroke-width", 1)
    .attr("id", "zrect");
  var vis = svg_graph.append("svg:g");
  vis.attr("fill", "red")
     .attr("stroke", "black")
     .attr("stroke-width", 1)
     .attr("opacity", 0.5)
     .attr("id", "vis");

  var link = vis.append("g").attr("class", "link").selectAll("line");
  var node = vis.append("g").attr("class", "node").selectAll("circle");

  d3.json("/data/c.json", function(err, g) {
    var n = g.nodes;
    var nodes = [];
    for(var i = 0; i < n; i++) {
      nodes.push({name: i});
    }
    force.nodes(nodes).links(g.edges).start();
    edges = force.links().map(function(e) {
      return {a: e.source.index, b: e.target.index, weight: e.value };
    });

    link = link.data(g.edges)
               .enter().append("line")
               .attr("class", "link")
               .style("stroke", function (d) { return colour(d.value); });
    node = node.data(nodes)
               .enter().append("circle")
               .attr("class", "node")
               .attr("r", 2)
               .call(force.drag)
               .on("click", function(d) {
                if(d3.event.defaultPrevented) return;
                if(toggles.handle) {
                  d3.select(this).classed("handle", d.handle = !d.handle)
                    .style("fill", function(p) {
                      if(p.handle) {
                        return 'blue';
                      } else {
                        return 'red';
                      }
                    });
                  if(d.handle) {
                    handle.push(d.index);
                  } else {
                    handle.splice(handle.indexOf(d.index), 1);
                  }
                } else if(toggles.tooth) {
                  d3.select(this).classed("tooth", d.tooth = !d.tooth)
                    .style("stroke", function(p) {
                      if(p.tooth) {
                        return teeth_colours(current_tooth);
                      } else {
                        return "black";
                      }
                    });
                  if(d.tooth) {
                    teeth[current_tooth].push(d.index);
                  } else {
                    for(var i = 0; i < teeth.length; i++) {
                      var idx = teeth[i].indexOf(d.index);
                      if(idx > -1) {
                        teeth[i].splice(idx, 1);
                      }
                    }
                  }
                }
                update_display();
               });

    force.on("tick", function() {
      link.attr("x1", function(d) { return d.source.x; })
          .attr("y1", function(d) { return d.source.y; })
          .attr("x2", function(d) { return d.target.x; })
          .attr("y2", function(d) { return d.target.y; });
      node.attr("cx", function(d) { return d.x; })
          .attr("cy", function(d) { return d.y; });
    });
  });

  function update_display() {
    for(var i = 0; i < teeth.length; i++) {
      d3.select("#tooth_" + i).text(teeth[i]);
    }
    d3.select("#handle_nodes").text(handle);
    var comb_sum = get_delta_weight(handle) +
      teeth.reduce(function(acc, x) {
        return acc + get_delta_weight(x);
      }, 0),
      rhs = 3 * teeth.length + 1;
    d3.select("#comb_weight_sum").text(comb_sum);
    d3.select("#comb_difference").text(rhs - comb_sum)
  }
  d3.select("#weight").on("input", function() { update(this.value); });
  update(100);
  function update(wt) {
    d3.select("#weight").property("value", wt);
    link.style("stroke-opacity", function(d) {
      if(100 * d.value <= wt) {
        return 1;
      } else {
        return 0;
      }
    });
  };

  function keydown() {
    switch(d3.event.keyCode) {
      case KEYCODE.c:
        clear();
        break;
      case KEYCODE.h:
        toggle('handle');
        break;
      case KEYCODE.t:
        toggle('tooth');
        break;
      case KEYCODE.n:
        next_tooth();
        break;
      case KEYCODE.b:
        back_tooth();
        break;
      case KEYCODE.a:
        add_tooth();
        break;
    }
  }
  function clear() {
    for(var i = 0; i < teeth.length; i++) {
      d3.select("#tooth_" + i + "_container").remove();
    }
    current_tooth = -1;
    previous_tooth = -1;
    teeth = [];
    handle = [];
    update_display();
    vis.selectAll('.handle').style("fill", "red").classed('handle', false);
    vis.selectAll('.tooth').style("stroke", "black").classed("tooth", false);
  }
  function toggle(t) {
    var toggle_keys = Object.keys(toggles);
    for(var i in toggle_keys) {
      var key = toggle_keys[i];
      if(key != t) {
        toggles[key] = false;
      }
    }
    toggles[t] = !toggles[t];
  }

  function add_tooth() {
    previous_tooth = current_tooth;
    current_tooth += 1;
    teeth.push([]);

    var new_tooth_name = "tooth_" + current_tooth + ": ",
        new_tooth_elem = d3.select("#teeth").append("p").attr("id", "tooth_" + current_tooth + "_container");
    new_tooth_elem.append("span")
      .attr("id", "tooth_" + current_tooth + "_label")
      .style("color", teeth_colours(current_tooth))
      .text(new_tooth_name);
    new_tooth_elem.append('span').attr('id', "tooth_" + current_tooth);
    tooth_colours(previous_tooth, current_tooth);
  }
  function next_tooth() {
    previous_tooth = current_tooth;
    if(current_tooth < teeth.length) {
      current_tooth += 1;
    }
    tooth_colours(previous_tooth, current_tooth);
    console.log("Current tooth: " + current_tooth);
  }
  function back_tooth() {
    previous_tooth = current_tooth;
    if(current_tooth > 0) {
      current_tooth -= 1;
    }
    tooth_colours(previous_tooth, current_tooth);
    console.log("Current tooth: " + current_tooth);
  }
  function tooth_colours(prev_t, cur_t) {
    d3.select("#tooth_" + prev_t + "_label")
      .style("background", "white");
    d3.select("#tooth_" + cur_t + "_label")
      .style("background", "yellow");
  }
  function get_delta_weight(V) {
    return edges.reduce(function(acc, e) {
      if((V.indexOf(e.a) > -1) + (V.indexOf(e.b) > -1) == 1) {
        return acc + e.weight;
      } else {
        return acc;
      }
    }, 0);
  }
}
