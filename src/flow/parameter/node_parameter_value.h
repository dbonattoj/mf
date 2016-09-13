#ifndef MF_FLOW_NODE_PARAMETER_VALUE_H_
#define MF_FLOW_NODE_PARAMETER_VALUE_H_

#include <utility>
#include <memory>
#include <type_traits>

namespace mf { namespace flow {

/// Type-erased container for node parameter value.
/** Can hold value of any type. Copy or move assignment reallocates the held value and may change its type.
 ** Value is retrieved (for reading and writing) using `get<T>()`, where `T` must be the correct type. */
class node_parameter_value {
private:
	class holder_base;
	template<typename T> class holder;
	
	std::unique_ptr<holder_base> holder_;

public:
	node_parameter_value(const node_parameter_value&);
	node_parameter_value(node_parameter_value&&);
	template<typename T> node_parameter_value(T&&);
	
	node_parameter_value& operator=(const node_parameter_value&);
	node_parameter_value& operator=(node_parameter_value&&);
	template<typename T> node_parameter_value& operator=(T&&);
	
	template<typename T> bool is_type() const;
	template<typename T> const T& get() const;
	template<typename T> T& get();
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
	explicit holder(const T& t) : value_(t) { }
	explicit holder(T&& t) : value_(std::move(t)) { }
	~holder() override = default;
	
	std::unique_ptr<holder_base> clone() const override
		{ return std::make_unique<holder>(value_); } 
	
	const T& value() const { return value_; }
	T& value() { return value_; }
};

}}

#include "node_parameter_value.tcc"

#endif

