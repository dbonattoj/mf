#include "multiplex_node_loader.h"
#include "graph.h"
#include <utility>
#include <functional>

namespace mf { namespace flow {

multiplex_node::loader::loader(multiplex_node& nd, thread_index tind) :
	node_(nd),
	thread_index_(tind) { }
	

///////////////


multiplex_node::sync_loader::sync_loader(multiplex_node& nd) :
	loader(nd, nd.output_at(0).reader_thread_index())
{
	Assert(! nd.outputs_on_different_threads_(),
		"multiplex_node with sync_loader must have all outputs on same reader thread");
}


void multiplex_node::sync_loader::stop() { }


void multiplex_node::sync_loader::launch() { }


node::pull_result multiplex_node::sync_loader::pull(time_span span) {
	// get expected input span, based on current time of the common successor node
	time_unit successor_time = this_node().capture_successor_time_();
	time_span expected_input_span = this_node().expected_input_span_(successor_time);
	if(! expected_input_span.includes(span)) {
		// tfail if pulled span not in this expected span
		return node::pull_result::transitory_failure;
	}
	
	successor_time = this_node().common_successor_node().current_time();
	if(successor_time != this_node().successor_time_of_input_view_()) {
		this_node().load_input_view_(successor_time);
	}
	
	time_span input_span = this_node().input_view_().span();
	if(input_span.includes(span)) return node::pull_result::success;
	else return node::pull_result::transitory_failure;
}


timed_frame_array_view multiplex_node::sync_loader::begin_read(time_span span) {	
	timed_frame_array_view input_view = this_node().input_view_();
		
	std::ptrdiff_t start_index = input_view.time_index(span.start_time());
	std::ptrdiff_t end_index = input_view.time_index(span.end_time());
	
	return input_view(start_index, end_index);
}


void multiplex_node::sync_loader::end_read(time_unit duration) { }


///////////////


multiplex_node::async_loader::async_loader(multiplex_node& nd) :
	loader(nd, nd.this_graph().new_thread_index()),
	stopped_(false) { }


multiplex_node::async_loader::~async_loader() {
	Assert(! thread_.joinable(), "multiplex_node::async_loader must be stopped before destruction");
}


void multiplex_node::async_loader::thread_main_() {
	for(;;) {
		time_unit successor_time = -1;
		
		// wait until current  successor_time no longer equal to the successor time for which input view was loaded 
		std::unique_lock<std::mutex> lock(successor_time_mutex_);
		successor_time_changed_cv_.wait(lock, [&] {
			if(stopped_) return true;
			successor_time = this_node().capture_successor_time_();
			return (successor_time != this_node().successor_time_of_input_view_());
		});
		lock.unlock();
		Assert(successor_time != -1);
		
		if(stopped_) break;
		
		// acquire exclusive lock on input view mutex --> waits until all readers end
		// and load the new input view
		{
			std::lock_guard<std::shared_timed_mutex> view_lock(input_view_mutex_);
			this_node().load_input_view_(successor_time);
		}
		
		// notify waiting readers that input view was changed
		input_view_updated_cv_.notify_all();
	}
	
	{
		std::lock_guard<std::shared_timed_mutex> view_lock(input_view_mutex_);
		this_node().unload_input_view_();
	}
}


void multiplex_node::async_loader::stop() {
	Assert(this_node().this_graph().was_stopped());
	Assert(thread_.joinable());
	
	stopped_ = true;

	successor_time_changed_cv_.notify_one();
	input_view_updated_cv_.notify_all();
	
	thread_.join();	
}


void multiplex_node::async_loader::launch() {
	stopped_ = false;
	thread_ = std::move(std::thread(
		std::bind(&multiplex_node::async_loader::thread_main_, this)
	));
}


node::pull_result multiplex_node::async_loader::pull(time_span span) {
	// TODO prove formally
		
	// get expected input span, based on current time of the common successor node
	time_unit successor_time = this_node().capture_successor_time_();
	time_span expected_input_span = this_node().expected_input_span_(successor_time);
	if(! expected_input_span.includes(span)) {
		// tfail if pulled span not in this expected span
		return node::pull_result::transitory_failure;
	}
	
	// acquire shared lock on input view
	// shared with other readers calling pull() on different threads
	// loader can modify the input view when all readers release the shared lock
	std::shared_lock<std::shared_timed_mutex> lock(input_view_mutex_);
	
	// wail until loader updates the input view for the current time of the common successor node
	while(this_node().current_time() != this_node().capture_successor_time_()) {
		if(stopped_) return node::pull_result::stopped;
		successor_time_changed_cv_.notify_one();
		input_view_updated_cv_.wait(lock);
	}
	if(stopped_) return node::pull_result::stopped;
		
	// time of the common successor node may have changed in the meantime so that the input view span
	// no longer includes the requested span. then return tfail 
	time_span input_span = this_node().input_view_().span();
	if(input_span.includes(span)) return node::pull_result::success;
	else return node::pull_result::transitory_failure;
}


timed_frame_array_view multiplex_node::async_loader::begin_read(time_span span) {
	input_view_mutex_.lock_shared();
	
	timed_frame_array_view input_view = this_node().input_view_();
		
	std::ptrdiff_t start_index = input_view.time_index(span.start_time());
	std::ptrdiff_t end_index = input_view.time_index(span.end_time());
	
	return input_view(start_index, end_index);
}


void multiplex_node::async_loader::end_read(time_unit duration) {
	input_view_mutex_.unlock_shared();
}



}}
