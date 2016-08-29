#include <utility>

namespace mf { namespace flow {

class filter_parameter_value::holder_base {
public:
	virtual ~holder_base() = 0;
	virtual holder_base* clone() = 0;
};


template<typename Value>
class filter_parameter_value::holder : public holder_base {
private:
	Value value_;
	
public:
	explicit holder(const Value& val) : value_(val) { }
	~holder() override = default;
	
	holder* clone() override {
		return new holder(value_);
	}
	
	const Value& value() const { return value_; }
	Value& value() { return value_; }
};


///////////////


template<typename Value>
filter_parameter_value::filter_parameter_value(const Value& val) :
	holder_(new holder<Value>(value)) { }


filter_parameter_value::filter_parameter_value(const filter_parameter_value& value) :
	holder_(value.holder_.clone()) { }


filter_parameter_value::filter_parameter_value(filter_parameter_value&& value) :
	holder_(std::move(value.holder_)) { }


filter_parameter_value::~filter_parameter_value() { }


filter_parameter_value& filter_parameter_value::operator=(const filter_parameter_value& value) {
	holder_.reset(value.holder_.clone()); // TODO exception safety
	return *this;
}


filter_parameter_value& filter_parameter_value::operator=(filter_parameter_value&& value) {
	holder_.reset(std::move(value.holder_.clone()));
	return *this;
}


template<typename Value>
bool filter_parameter_value::has_type() const {
	using expected_holder_type = holder<Value>;
	return (dynamic_cast<const expected_holder_type*>(&holder_) != nullptr);
}


template<typename Value>
const Value& filter_parameter_value::value() const {
	using expected_holder_type = holder<Value>;
	const auto& hold = dynamic_cast<const expected_holder_type&>(holder_);
	return hold.value();
}


template<typename Value>
Value& filter_parameter_value::value() {
	using expected_holder_type = holder<Value>;
	const auto& hold = dynamic_cast<const expected_holder_type&>(holder_);
	return hold.value();
}

}}
