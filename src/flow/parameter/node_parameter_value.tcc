namespace mf { namespace flow {
	

template<typename T>
node_parameter_value::node_parameter_value(T&& t) :
	holder_(new holder<std::decay_t<T>>(std::forward<T>(t))) { }


template<typename T>
node_parameter_value& node_parameter_value::operator=(T&& t) {
	holder_.reset(new holder<std::decay_t<T>>(std::forward<T>(t)));
	return *this;
}


template<typename T>
bool node_parameter_value::is_type() const {
	using holder_type = holder<T>;
	return (dynamic_cast<holder_type>(holder_.get()) != nullptr);
}


template<typename T>
const T& node_parameter_value::get() const {
	using holder_type = holder<T>;
	return dynamic_cast<const holder_type&>(*holder_).value();
}


template<typename T>
T& node_parameter_value::get() {
	using holder_type = holder<T>;
	return dynamic_cast<holder_type&>(*holder_).value();
}



}}
