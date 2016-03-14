namespace mf {

template<std::size_t Dim, typename T>
void media_node_input<Dim, T>::pull() {
	if(connected_output_ == nullptr) throw std::logic_error("node input is not connected");

	time_unit t = node_.current_time();
	connected_output_->pull(t + future_window_);
}


template<std::size_t Dim, typename T>
void media_node_input<Dim, T>::begin_read() {
	if(connected_output_ == nullptr) throw std::logic_error("node input is not connected");
	
	time_unit t = node_.current_time();
		
	// time span to request from connected output
	// if t near beginning, size of past window is reduced
	time_span requested_span;
	if(t < past_window_) {
		requested_span = time_span(0, t + future_window_ + 1);
		view_center = t;
	} else {
		requested_span = time_span(t - past_window_, t + future_window_ + 1);
		view_center_ = past_window_;
	}

	// begin read span from output's ring buffer
	// waits until it becomes available		
	auto view = connected_output_->begin_read_span(requested_span);
	
	// if near end, view duration < requested span duration, and size of future window is reduced
	assert(view.shape().front() <= span.duration());
	view_.reset(view);
}


template<std::size_t Dim, typename T>
void media_node_input<Dim, T>::end_read() {
	if(connected_output_ == nullptr) throw std::logic_error("node input is not connected");
	
	// normal operation: 1 frame consumed from output = first from past window
	// if near beginning and past window still reduced: consume no frame
	time_unit t = node_.current_time();
	connected_output_->end_read(t >= past_window_ ? 1 : 0);
}

}