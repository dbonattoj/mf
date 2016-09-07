#ifndef MF_FLOW_NODE_PARAMETER_VALUE_H_
#define MF_FLOW_NODE_PARAMETER_VALUE_H_

#include <utility>
#include <memory>

namespace mf { namespace flow {

class node_parameter_value {
private:
	class holder_base;
	template<typename T> class holder;
	
	std::unique_ptr<holder_base> holder_;

public:
	node_parameter_value(const node_parameter_value&);
	node_parameter_value(node_parameter_value&&);
	template<typename T> node_parameter_value(const T&);
	template<typename T> node_parameter_value(T&&);
	
	node_parameter_value& operator=(const node_parameter_value&);
	node_parameter_value& operator=(node_parameter_value&&);
	template<typename T> node_parameter_value& operator=(const T&);
	template<typename T> node_parameter_value& operator=(T&&);
};


class node_parameter_value::holder_base {
public:
	virtual ~holder_base() { }
	virtual std::unique_ptr<holder_base> clone() const = 0;
};


template<typename T>
class node_parameter_value::holder : public node_parameter_value::holder_base {
private:
	T value_;

public:
	holder() : value_() { }
	holder(const T& t) : value_(t) { }
	holder(T&& t) : value_(std::move(t)) { }
	~holder() override = default;
	
	std::unique_ptr<holder_base> clone() const override
		{ return std::make_unique<holder>(value_); } 
};

}}

#include "node_parameter_value.tcc"

#endif

