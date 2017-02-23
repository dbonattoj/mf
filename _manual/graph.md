---
title: Graph
---

A flow graph is constructed and run using the `graph` class. First create an instance

    flow::graph gr;

## Adding filters
Filter nodes are added using

    my_filter& filt = gr.add_filter<my_filter>(...);

If the filter's constructor takes additional arguments (after the first `filter_node&`), they can be passed to
`graph::add_filter` and will be forwarded to it. The function returns a reference to the created `filter` object. The
filter object, and the underlying node, are owned by the `graph`, and will be destructed when the `graph` is destructed.

This creates a _synchronous_ node by default. To create an asynchronous node for the same filter, use

    my_filter& filt = gr.add_filter<my_filter, flow::async_node>(...);

To create the sink node, use

    my_sink_filter& sink = gr.add_sink<my_sink_filter>(...);

Exactly one sink must be created for the graph.


## Connections
Connections are made using the `connect()` function of filter inputs:

    sink.input.connect(filt.output);

Here, `sink.input` and `filt.output` need to have the same frame dimension and element type. Connections must be made
before the graph gets setup.

Currently, all inputs of all filters that were added into the graph need to be connected. Connections are one-by-one,
it is not possible to connect multiple inputs to one same output.


## Running graph
After the connections have been made, the graph needs to be setup using

    gr.setup();

This initialized the underlying nodes, allocates buffers and creates threads, and calls the `setup()` functions of the
filters.

After that the graph can be run using

    gr.run();

This runs until the end of the stream. Alternately `gr.run_for(time_unit duration)` and `gr.run_until(time_unit last)`
can be used to run only for a limited number of frames.

The graph is seekable if all its source filters are seekable. When this is the case, the time index can be changed using
`gr.seek(time_unit pos)`.
