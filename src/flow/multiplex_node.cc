#include "multiplex_node.h"
#include "graph.h"
#include <utility>
#include <functional>

namespace mf { namespace flow {

multiplex_node::multiplex_node(graph& gr) : node(gr) {
	add_input_<node_input>(0, 0);
}


multiplex_node::~multiplex_node() {
	
}


void multiplex_node::load_input_view_(time_unit t) {
	set_current_time_(t);
	std::lock_guard<std::shared_timed_mutex> view_lock(input_view_mutex_);
	
	if(successor_time_of_input_view_ != -1)	input().end_read_frame();
	
	pull_result result = input().pull();
	successor_time_of_input_view_ = t;
	input_view_.reset(input().begin_read_frame());
}


void multiplex_node::thread_main_() {
	while(! this_graph().was_stopped()) {
		time_unit successor_time = successor_node_->current_time();
		std::unique_lock<std::mutex> lock(successor_time_mutex_);
		successor_time_changed_cv_.wait(lock, [&] {
			successor_time = successor_node_->current_time();
			return (successor_time_of_input_view_ != successor_time);
		});
		lock.unlock();
		
		load_input_view_(successor_time);
		input_view_updated_cv_.notify_all();
	}
}


time_unit multiplex_node::minimal_offset_to(const node& target_node) const {
	return 0; // TODO
}


time_unit multiplex_node::maximal_offset_to(const node& target_node) const {
	return 0; // TODO
}

	
void multiplex_node::launch() {
	thread_ = std::move(std::thread(
		std::bind(&multiplex_node::thread_main_, this)
	));
}


void multiplex_node::stop() {
	Assert(this_graph().was_stopped());
	Assert(thread_.joinable());
	thread_.join();
}


void multiplex_node::pre_setup() {
	successor_node_ = &first_successor();
	
	const frame_format& format = input().connected_output().this_output().format();
	for(auto&& out : outputs()) out->define_format(format);
}


void multiplex_node::setup() {
	std::size_t frame_length = input().connected_output().this_output().frame_length();
	for(auto&& out : outputs()) out->define_frame_length(frame_length);

	const frame_format& format = input().connected_output().this_output().format();
	for(auto&& out : outputs()) out->define_format(format);
}

	
node_input& multiplex_node::input() {
	Assert(inputs().size() == 1);
	return *inputs().front();
}


multiplex_node_output& multiplex_node::add_output() {
	return node::add_output_<multiplex_node_output>(frame_format::null());
}


multiplex_node_output::multiplex_node_output(node& nd, std::ptrdiff_t index, const frame_format& format) :
	node_output(nd, index, format),
	input_view_shared_lock_(this_node().input_view_mutex_, std::defer_lock) { }


node::pull_result multiplex_node_output::pull(time_span& span, bool reconnect) {
	input_view_shared_lock_.lock();
	while(this_node().successor_time_of_input_view_ != this_node().successor_node_->current_time()) {
		this_node().successor_time_changed_cv_.notify_one();
		this_node().input_view_updated_cv_.wait(input_view_shared_lock_);
	}
	time_span input_span = this_node().input_view_.span();
	input_view_shared_lock_.unlock();
	
	if(input_span.includes(span)) return node::pull_result::success;
	else return node::pull_result::transitory_failure;
}

timed_frame_array_view multiplex_node_output::begin_read(time_unit duration) {
	Assert(! input_view_shared_lock_);
	input_view_shared_lock_.lock();
	
	timed_frame_array_view& input_view = this_node().input_view_;
	const time_span& pulled_span = connected_input().pulled_span();
	
	Assert(input_view.span().includes(pulled_span)); // ???
	
	std::ptrdiff_t start_index = input_view.time_index(pulled_span.start_time());
	std::ptrdiff_t end_index = start_index + duration;
	return input_view(start_index, end_index);
}


void multiplex_node_output::end_read(time_unit duration) {
	Assert(input_view_shared_lock_);
	input_view_shared_lock_.unlock();
}



}}
