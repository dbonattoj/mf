namespace mf {

template<std::size_t Dim, typename T>
node<Dim, T>::node(ndsize<Dim> shape, std::size_t time_window) :
	//output_(ndcoord_cat(ndsize<1>(time_window), shape)),
	time_window_(time_window) { }


template<std::size_t Dim, typename T>
void node<Dim, T>::pull() {
	for(auto&& nd : input_nodes_) nd->pull();
	
	
	
	//this->process_frame_();
}

}