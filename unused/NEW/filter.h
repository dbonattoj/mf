#ifndef MF_FLOW_FILTER_H_
#define MF_FLOW_FILTER_H_

#include "node_job.h"
#include "filter_node.h"
#include <string>

namespace mf { namespace flow {

/// Filter which performs concrete processing media step, base class.
class filter {
private:
	template<std::size_t Dim, typename Elem> class port;
	template<std::size_t Dim, typename Elem> class input_port;
	template<std::size_t Dim, typename Elem> class output_port;
	
	filter_node& node_;
	
	const filter_node& this_node() const { return node_; }
	filter_node& this_node() { return node_; }
	
public:
	template<std::size_t Dim, typename Elem> using input_type = input_port<Dim, Elem>;
	template<std::size_t Dim, typename Elem> using output_type = output_port<Dim, Elem>;

	explicit filter(filter_node& nd) : node_(nd) { }
	
	filter(const filter&) = delete;
	filter& operator=(const filter&) = delete;
	
	virtual ~filter() { }

	/// Set up the filter, called prior to any frame being processed.
	virtual void setup() = 0;
	
	/// Prepare for processing a frame.
	virtual void pre_process(node_job&) = 0;
	
	/// Process a frame.
	virtual void process(node_job&) = 0;
};


template<std::size_t Dim, typename Elem>
class filter::port {
public:
	using elem_type = Elem;
	using frame_shape_type = ndsize<Dim>;
	constexpr static std::size_t dimension = Dim;

private:
	filter& filter_;
	frame_shape_type frame_shape_;

protected:
	static frame_format default_format() const { return frame_format::default_format<Elem>(); }

	explicit port(filter& filt) : filter_(filt) { }
	port(const port&) = delete;
	
	void set_frame_shape(const frame_shape_type& shp) { frame_shape_ = shp; }

public:
	const filter& this_filter() const { return filter_; } 
	filter& this_filter() { return filter_; }
	
	const frame_shape_type& frame_shape() const { return frame_shape_; }
};


template<std::size_t Dim, typename Elem>
class filter::output_port : public filter::port<Dim, Elem> {
	using base = filter::port<Dim, Elem>;
	
private:
	node_output node_output_;

public:
	explicit output_port(filter& filt) :
		base(filt),
		node_output_(filt.this_node(), base::default_format()) { }

	node_output& this_node_output() { return node_output_; }

	std::ptrdiff_t index() const { return node_output_.index(); }

	void define_frame_shape(const frame_shape_type& shp) {
		node_output_.define_frame_length(shp.product());
	}
	
	const frame_shape_type& frame_shape() const {
		return frame_shape_;
	}
};


template<std::size_t Dim, typename Elem>
class filter::input_port : public filter::port<Dim, Elem> {
	using base = filter::port<Dim, Elem>;
	
private:
	node_input node_input_;

public:
	explicit input_port(filter& filt, time_unit past_window = 0, time_unit future_window = 0) :
		base(filt),
		node_input_(filt.this_node(), past_window, future_window) { }

	node_input& this_node_input() { return node_input_; }

	std::ptrdiff_t index() const { return node_output_.index(); }

	void set_activated(bool activated) {
		node_input_.set_activated(activated);
	}
	
	bool is_activated() {
		return node_input_.is_activated();
	}
	
	void connect(output_port<Dim, Elem>& output) {
		// TODO check compatibility, implicit conversion --> ndarray_view_cast, color_convert, etc.
		node_input_.connect(output.this_node_output());
		base::set_frame_shape(output.frame_shape());
	}
};


}}

#include "filter.tcc"

#endif
