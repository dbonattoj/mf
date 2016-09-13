/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MF_FLOW_FILTER_PARAMETER_H_
#define MF_FLOW_FILTER_PARAMETER_H_

#include <functional>
#include "../flow/types.h"

namespace mf { namespace flow {

class filter;
class filter_graph;
class node;

template<typename Value> class filter_extern_parameter;

class filter_parameter_base {
public:
	virtual ~filter_parameter_base() = default;
	virtual void install(filter_graph&, node&) = 0;
};


class filter_extern_parameter_base {
public:
	virtual ~filter_extern_parameter_base() = default;
	virtual void install(filter_graph&, node&) = 0;
};


/// Parameter of type \a Value belonging to a filter.
/** Can be either _deterministic_ or _dynamic_. Deterministic parameters have either a constant value, or a value
 ** computed in function of the frame index (animation). Dynamic parameters are updated at will by the node(s) during
 ** execution. */
template<typename Value>
class filter_parameter : public filter_parameter_base {
public:
	using value_type = Value;
	using extern_parameter_type = filter_extern_parameter<Value>;
	using deterministic_value_function = Value(time_unit);

private:
	filter& filter_;
	node_parameter_id id_ = undefined_node_parameter_id;
	std::function<deterministic_value_function> value_function_;
	std::string name_;

public:
	explicit filter_parameter(filter&);
	
	node_parameter_id id() const { return id_; }
	bool is_deterministic() const;
	bool is_dynamic() const;

	template<typename Function> void set_value_function(Function&& func);
	void set_constant_value(const Value&);
	void set_dynamic();
	
	Value deterministic_value(time_unit t) const;
	
	void set_name(const std::string& nm) { name_ = nm; }
	const std::string& name() const { return name_; }
	
	bool was_installed() const;
	void install(filter_graph&, node&);
};


/// Link to a parameter of type \a Value belonging to another filter.
/** ... */
template<typename Value>
class filter_extern_parameter : public filter_extern_parameter_base {
public:
	using value_type = Value;
	using parameter_type = filter_parameter<Value>;

private:
	filter& filter_;
	parameter_type* linked_parameter_ = nullptr;
	bool readable_;
	bool writable_;
	std::string name_;

public:
	explicit filter_extern_parameter(filter&);
	
	void link(parameter_type&);
	bool is_linked() const;
	const parameter_type& linked_parameter() const;
	
	void set_name(const std::string& nm) { name_ = nm; }
	const std::string& name() const { return name_; }
	
	void install(filter_graph&, node&);
};

}}

#include "filter_parameter.tcc"

#endif
