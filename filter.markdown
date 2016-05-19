---
title: mf: Filters
---

# Writing Filters
Filters are implemented as derived classes of `filter`, `source_filter` or `sink_filter`. For example:

    class my_filter : public filter {
    public:
        input_type<2, rgb_color> image_input;
        input_type<2, depth_type> depth_input;
        output_type<2, rgb_color> output;
                
        explicit my_filter(flow::filter_node& nd) :
	        filter(nd),
	        image_input(*this),
	        depth_input(*this, 2, 1),
	        output(*this) { }
    
        void setup() override;
        void pre_process(const flow::node_job& job) override;
        void process(const flow::node_job& job) override;
    };


## Construction
The constructor must take a non-const reference `flow::filter_node&` as first argument, and pass it to the constructor
of the base class.

### Inputs and outputs
This defines a filter `my_filter` which takes two inputs and one output. The inputs are created by instantiating
objects of type `filter::input_type<Dim, Elem>`. The dimension and element type of the frames are statically defined
using the template arguments `Dim` and `Elem`. The frame shapes are set at runtime, in the setup phase. Outputs are
created the same way. Currently, each filter, must create exactly one output, except for the sink filters which have
none.

The constructor of both the inputs and outputs needs to be passed a reference to the `filter`, i.e. `*this`. This step
creates and registers the underlying node inputs/output. For inputs only, optionally the past and future time window
can be passed as constructor arguments:

    input_type(filter_node&, time_unit past_window = 0, time_unit future_window = 0)
   
In this example, `my_filter::depth_input` will receive not only the current frame, but also 2 previous frames and 1
future frame.

Creation of inputs and outputs is typically done at construction time, but can also be done at runtime, with the inputs
and/or output stored in a container. However it must be done before the graph is setup.

The filter's input and output object are meant to be publicly accessible. The graph is constructed using
`filt.input.connect(...)` calls from outside.


### Source stream properties
Source filters (derived from `source_filter`) must also define the _source stream properties_ at construction. The
`source_filter` constructor takes the arguments

    source_filter(flow::filter_node& nd, bool seekable = false, time_unit stream_duration = -1)

`seekable` defined whether the source node supports non-sequential pulling. If it is `false`, it is guaranteed that
its process functions will always be called in sequential order, with incrementing time index. This is for example
appropriate if the source filter reads from a real-time input. If any source in the graph is not seekable, `seek()`
cannot be called on the full graph, and it is always executed sequentially.

`stream_duration` sets the total number of frames from this source. The _stream duration_ of the full graph will always
be the minimal stream duration of its sources. It must be defined for seekable sources. E.g. for a file importer source,
it would be the number of frames in the file.

or non-seekable source, it can be left undefined. In that case the source filter mark the end after having written the
last frame, as will be elaborated below.

The source stream properties can also be changed after construction, but before setup, using

    source_filter::define_stream_properties(bool seekable, time_unit stream_duration)


## Setup
When the graph is setup with, the function `setup()` will be called on each filter in the graph. This always happens 
prior to any data flow through the node. The `filter` must define the frame shapes of its output in its implementation
of `setup()`:

    void my_filter::setup() {
        output.define_frame_shape(image_input.frame_shape());
    }

In this case, the `output` is set to the same frame shape as the `image_input`.

It is guaranteed that when `setup()` is called on a filter, it has already been called before on the filters connected
to its inputs. `input.frame_shape()` is always equal to the frame shape which the node connected to this input has set
for its output. That is, the frame shape is defined for the _edge_ formed by the connected input and output. Thus the
example code is valid, and the frame shape can be propagated through the graph from the sources to the sink.

`setup()` is always called before any frames are processed on the filter, and the filter can also implement any late
initialization code in this function.


## Processing frames
When the flow graph is running, frames are always processed one-by-one in each node. For each frame in each filter,
the functions `pre_process()` and `process()` are called.

First `pre_process()` is called, and the filter can activate or deactivate inputs. Then `process()` is called, from
where the filter can read inputs and must write one output frame.


### Node job
A `node_job` object is passed to `pre_process()` and `process()` functions. It represents the current job of the node.
In future versions, the functions may be called simultaneously on the same `filter` object, with different parallel jobs.

The time index of the frame being processed is accessed using `job.time()`. Input and output views are also accessed
through the job object.

### Preprocess
In the proprocessing stage, the node can activate or deactivate inputs. `pre_process()` does not need to be implemented
by the `filter`, as a default implementation exists which does nothing.

Inputs can be activated or deactivated using `input.set_activated(true/false)`. For example,

    void my_filter::pre_process(node_job& job) {
		time_unit t = job.time();
        depth_input.set_activated( (t / 100) % 2 != 0 );
    }

This activates and deactivates `depth_input` in intervals of 100 frames. The current activation status of an
input can be queried using `input.is_activated()`.

*Inputs must be activated only from within `pre_process()`.*

### Processing
Next the `process()` function is called. It is also called in case no inputs are activated. In it the node must always
write one frame to its output. It can read frames from its activated inputs.

    void my_filter::process(node_job& job) {
		auto& out = job.out(output);
        if(image_input.is_activated()) {
			auto& in = job.in(image_input);
			out = in;
        } else {
			std::fill(out.begin(), out.end(), rgb_color::black);
        }
    }

This example copies the current frame from `image_input` the output if the input is activated. Otherwise it fills the 
background with black pixels.

Input frame views are accessed using `job.in(image_input)`. The return value is a `ndarray_view<2, rgb_color>`. Using
template magic this always has the dimension and element type that was defined for the input. The filter must not write
into the input views. `job.in(image_input)` must not be called when `image_input` is deactivated.

The output frame view is accesseedd the same way using `job.out(output)`, which returns a `ndarray_view<2, rgb_color>`.
The filter must always a frame write into this view, even when no inputs are activated.

#### Time window
For inputs with time windows, the full view, with past and future frames, is accessed using `job.in_full(depth_input)`.
In this example the function returns a `ndarray_timed_view<3, depth_type>`. The coordinate is the relative time index.
The frame at the current time is accessed from this `ndarray_timed_view vw` using `vw.at_time(t)` with `t = job.time()`.
Alternately the index of this frame is `std::ptrdiff_t i = vw.time_index()`, the the current frame can be accessed like
`vw[i]`.

`vw` is an ordinary 3-dimensial `ndarray`, with added time-specific function for the first dimension. For example a
slice through time and one image axis can be taken like `vw.slice(320, 1)`. The returned `ndarray_view<2, depth_type>`
represents the rows 320 of the different frames.

`vw.duration()` is `depth_input.past_window_duration() + 1 + depth_input.future_window_duration()`, except when near the
first and last frame of the stream, where the time windows necessarily get truncated.

#### Source end
If the `filter` is a _non-seekable source node_, and its stream duration was not defined at construction, the system
relies on the source filter to mark the last frame of the stream.

If the frame that was written by `process()` was the last in the stream, the source filter must call `job.mark_end()`.
This marks the end of the stream, and the filter will not be called to process the next frame. It must always mark the
end after having written a frame: If `pre_process()` and `process()` are called, this implies that the frame to at time
`job.time()` exists. This in contrary to other I/O systems where the a end-of-file error condition instead occurs when
trying to read or write data beyond the end.
