namespace mf {

template<std::size_t Dim, typename T>
node<Dim, T>::node(ndsize<Dim> shape, std::size_t time_window) :
	output_(shape, time_window),
	time_window_(time_window) { }


template<std::size_t Dim, typename T>
void node<Dim, T>::pull() {
}

}