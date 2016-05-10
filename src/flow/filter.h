#ifndef MF_FLOW_FILTER_H_
#define MF_FLOW_FILTER_H_

#include "node_job.h"
#include "node.h"
#include <string>

namespace mf { namespace flow {

/// Filter which performs concrete processing media step, base class.
/** Associated to and called by node object. Concrete filter is implemented as subclass of this class.
 ** Inputs and outputs of the filter are defined by creating `input_type` and `output_type` instances with specified
 ** dimension and element type.  */
class filter {
private:
	template<std::size_t Dim, typename Elem> class input_port;
	template<std::size_t Dim, typename Elem> class output_port;

	node_job* job_ = nullptr;
	
	template<std::size_t Dim, typename Elem> void register_input();

protected:
	/// Time of currently processed frame.
	/** Can be called by derived concrete node only from within process() or pre_process(), i.e. when job exists. */
	time_unit time() const noexcept {
		MF_EXPECTS(job_ != nullptr);
		return job_->time();
	}
	
	/// Mark current frame as the last frame in stream.
	/** Can be only called from within process(). */
	void mark_end() {
		MF_EXPECTS(job_ != nullptr);
		job_->mark_end();
	}

	/// Set up the filter, called prior to any frame being processed.
	/** Implemented by concrete filter. Must define frame shapes of output(s). */
	virtual void setup() = 0;
	
	/// Prepare for processing a frame.
	/** Implemented by concrete filter. Called immidiatly prior to processing a frame. Job exists and current time can
	 ** be accessed using time(), but no input/output views are available yet. At this stage (only) the filter can
	 ** activate/desactivate inputs. */
	virtual void pre_process() = 0;
	
	/// Process a frame.
	/** Implemented by concrete filter. Job exists, input and output views are available. Concrete filter can read frame
	 ** from input(s) (with the time window it specified), and must write one frame to output. For non-seekable source
	 ** node filters, mark_end() must be called from here when this was the last frame. */
	virtual void process() = 0;

public:
	template<std::size_t Dim, typename Elem> using input_type = input_port<Dim, Elem>;
	template<std::size_t Dim, typename Elem> using output_type = output_port<Dim, Elem>;

	filter() = default;
	filter(const filter&) = delete;
	filter& operator=(const filter&) = delete;
	
	virtual ~filter() { }
	
	void call_setup() {
		this->setup();
	}
	
	void pre_process_job(node_job& job) {
		job_ = &job;
		this->pre_process();
		job_ = nullptr;		
	}
	
	void process_job(node_job& job) {
		job_ = &job;
		this->process();
		job_ = nullptr;
	}
};




template<std::size_t Dim, typename Elem>
class filter::input_port {
public:
	using node_type = typename Output::node_type;
	using elem_type = Elem;
	using frame_shape_type = ndsize<Dim>;
	constexpr static std::size_t dimension = Dim;

private:
	filter& filter_;
	node_input* node_input_ = nullptr;

public:
	filter_input(filter& filt, const std::string& id) : filter_(filt) { }
	
	void set_node_input(node_input* input) {
		node_input_ = input;
	}
	
	const frame_shape_type& frame_shape() const {  }
	
	void set_activated(bool activated) {
		MF_EXPECTS(node_input_ != nullptr);
		node_input_->set_activated(activated);
	}
	
	bool is_activated() {
		MF_EXPECTS(node_input_ != nullptr);
		return node_input_->is_activated();
	}
	
	timed_frame_array_view full_view() const {
		MF_EXPECTS(node_input_ != nullptr);
		MF_EXPECTS(filter_.job_ != nullptr);
		return filter_.job_.in_full(*node_output_);
	}
	
	frame_view view() const {
		MF_EXPECTS(node_input_ != nullptr);
		MF_EXPECTS(filter_.job_ != nullptr);
		return filter_.job_.in(*node_output_);	
	}
};



template<std::size_t Dim, typename Elem>
class filter::output_port {
public:
	using node_type = typename Output::node_type;
	using elem_type = Elem;
	using frame_shape_type = ndsize<Dim>;
	constexpr static std::size_t dimension = Dim;

private:
	filter& filter_;
	node_output* node_output_ = nullptr;
	frame_shape_type frame_shape_;

public:
	explicit filter_output(filter& filt, const std::string& id) : filter_(filt) { }

	void set_node_output(node_output* output) {
		node_output_ = output;
	}

	void define_frame_shape(const frame_shape_type& shp) {
		MF_EXPECTS(node_output_ != nullptr);
		node_output_->define_frame_length(shp.product());
	}
	
	const frame_shape_type& frame_shape() const {
		MF_EXPECTS(node_output_ != nullptr);
		return frame_shape_;
	}

	frame_view view() const {
		MF_EXPECTS(node_output_ != nullptr);
		MF_EXPECTS(filter_.job_ != nullptr);
		return filter_.job_.out(*node_output_);
	}

};


}}

#include "filter.tcc"

#endif
