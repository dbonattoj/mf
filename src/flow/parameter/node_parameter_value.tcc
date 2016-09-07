namespace mf { namespace flow {


template<typename T>
node_parameter_value::node_parameter_value(const T& t) :
	holder_(std::make_unique<holder<T>>(t)) { }


template<typename T>
node_parameter_value::node_parameter_value(T&&);
	holder_(std::make_unique<holder<T>>(std::move(t))) { }


template<typename T>
node_parameter_value& node_parameter_value::operator=(const T& t) {
	holder_ = std::move(std::make_unique<holder<T>>(t));
	return *this;
}


template<typename T>
node_parameter_value& node_parameter_value::operator=(T&& t) {
	holder_ = std::move(std::make_unique<holder<T>>(std::move(t)));
	return *this;
}


}}
