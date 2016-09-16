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
#include <memory>
#include <string>

namespace mf { namespace flow {

class filter;
class filter_graph;
class node;


/// Parameter attached to a filter, abstract base class.
class filter_parameter_base {
public:
	enum kind_type { undefined, deterministic, dynamic, reference };
	
	virtual ~filter_parameter_base() = default;
	virtual const filter& this_filter() const = 0;
	virtual const std::string& name() const = 0;
	virtual kind_type kind() const = 0;
	virtual const filter_parameter_base& referenced_parameter() const = 0;
	virtual bool is_input_reference() const = 0;
	virtual bool is_sent_reference() const = 0;

	virtual bool was_installed() const = 0;
	virtual void install(filter_graph&, node&) = 0;
};


/// Parameter attached to a filter.
/** The parameter carries a value of type \a Value, which may change during the execution of the graph.
 ** The filter can retrieve the value of its parameters (and set/send the value if applicable) during processing,
 ** through the \ref filter_job object.
 ** Must be set (before filter graph is set up) to one of three possible parameter kinds:
 ** - __Deterministic__: Carries either a constant value set via set_constant_value(), or a value computed in function
 **                      of the current time `t`, set be passing a function to set_value_function().
 ** - __Dynamic__: Its value is determined at runtime. The filter can set the values of its dynamic parameters.
 **                Set using set_dynamic(), with optional initial value.
 ** - __Reference__: Allows access to another filter's parameter. The referenced filter must _precede_ this filter in
 **                  the filter graph.
 **                  If the referenced parameter is deterministic, it simply computes the value the same way as for an
 **                  own deterministic parameter.
 **                  If the referenced parameter is dynamic, _input_ and/or _sent_ references are possible.
 **                  With an input reference, the referenced parameter's value is attached to each frame and propagated
 **                  down the internal node graph. The filter can access, for each frame, the value which the
 **                  referenced parameter had at the moment when the frame got processed by its owning filter.
 **                  With a sent reference, the filter can _send_ a new value to the referenced parameter. The owning
 **                  filter will then process its next frames with the last value it received for the parameter.
 **                  It is not specified when this filter starts to receive frames with that sent parameter value on
 **                  its inputs. */
template<typename Value>
class filter_parameter : public filter_parameter_base {
public:
	using value_type = Value;
	using deterministic_value_function = std::function<Value(time_unit)>;
	
private:
	filter& filter_;
	kind_type kind_ = undefined;
	bool input_reference_;
	bool sent_reference_;
	deterministic_value_function deterministic_value_function_;
	std::unique_ptr<Value> dynamic_initial_value_;
	const filter_parameter* referenced_parameter_ = nullptr;
	std::string name_;

	bool was_installed_ = false;
	node_parameter_id id_ = undefined_node_parameter_id;

	filter_parameter(const filter_parameter&) = delete;
	filter_parameter& operator=(const filter_parameter&) = delete;

	void set_undefined_();

public:
	explicit filter_parameter(filter&, const std::string& name = "");
	
	const filter& this_filter() const override { return filter_; }
	
	void set_name(const std::string& nm) { name_ = nm; }
	const std::string& name() const override { return name_; }
	
	kind_type kind() const override { return kind_; }
	void set_constant_value(const Value&);
	void set_value_function(deterministic_value_function);
	void set_dynamic(const Value& initial_value = Value());
	void set_reference(const filter_parameter&, bool input = true, bool sent = false);
		
	const Value& dynamic_initial_value() const;
	Value deterministic_value(time_unit t) const;
	const filter_parameter& referenced_parameter() const override;
	bool is_input_reference() const override;
	bool is_sent_reference() const override;
	
	node_parameter_id id() const { return id_; }
	
	bool was_installed() const override { return was_installed_; }
	void install(filter_graph&, node&) override;
};

}}

#include "filter_parameter.tcc"

#endif
