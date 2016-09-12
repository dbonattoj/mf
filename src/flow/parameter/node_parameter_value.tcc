namespace mf { namespace flow {


template<typename T>
node_parameter_value::node_parameter_value(const T& t) :
	holder_(std::make_unique<holder<T>>(t)) { }


template<typename T>
node_parameter_value::node_parameter_value(T&& t) :
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


template<typename T>
bool node_parameter_value::is_type() const {
	using holder_type = holder<T>;
	return (dynamic_cast<holder_type>(holder_.get()) != nullptr);
}


template<typename T>
const T& node_parameter_value::get() const {
	return dynamic_cast<const holder_type&>(*holder_).value();
}


template<typename T>
T& node_parameter_value::get() {
	return dynamic_cast<holder_type&>(*holder_).value();
}



}}
