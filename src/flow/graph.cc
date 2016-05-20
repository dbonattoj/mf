#include "graph.h"
#include "node.h"
#include "sink_node.h"
#include "../utility/stopwatch.h"

namespace mf { namespace flow {


void graph::pull_next_frame_() {
	stopwatch watch;
	watch.start();
	sink_->pull_next_frame();
	watch.stop();
	//std::cout << sink_->current_time() << " " << std::chrono::duration_cast<std::chrono::milliseconds>(watch.total_duration()).count() << std::endl;
}

graph::~graph() {
	if(running_) stop();
}


void graph::setup() {
	if(was_setup_) throw std::logic_error("graph was already set up");
	if(sink_ == nullptr) throw std::logic_error("no sink node was added to media graph");
	sink_->setup_graph();
	was_setup_ = true;
}


void graph::launch() {
	if(running_) throw std::logic_error("graph is already running");
	for(const auto& nd : nodes_) nd->launch();
	running_ = true;
}


void graph::stop() {
	if(! running_) throw std::logic_error("graph is not running");
	stop_event_.notify();
	for(const auto& nd : nodes_) nd->stop();
	stop_event_.reset();
	running_ = false;
}


time_unit graph::current_time() const {
	if(! was_setup_) throw std::logic_error("graph not set up");
	return sink_->current_time();
}

void graph::run_until(time_unit last_frame) {
	if(! was_setup_) throw std::logic_error("graph not set up");
	
	if(! running_) launch();
	
	while(sink_->current_time() < last_frame && !sink_->reached_end())
		pull_next_frame_();
}


void graph::run_for(time_unit duration) {
	run_until(current_time() + duration);
}


bool graph::run() {
	if(! was_setup_) throw std::logic_error("graph not set up");
	if(! sink_->is_bounded()) throw std::logic_error("sink is not bounded");
	if(! running_) launch();
	while(sink_->is_bounded() && !sink_->reached_end()) pull_next_frame_();
	return sink_->is_bounded();
}


void graph::seek(time_unit target_time) {
	if(! sink_->is_seekable()) throw std::logic_error("sink is not seekable");
	sink_->seek(target_time);
}



}}
