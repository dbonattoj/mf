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
#include "../flow/parameter/node_parameter.h"

namespace mf { namespace flow {

class filter;

template<typename Value> class filter_extern_parameter;

/// Parameter of type \a Value belonging to a filter.
template<typename Value>
class filter_parameter {
public:
	using value_type = Value;
	using extern_parameter_type = filter_extern_parameter<Value>;

private:
	filter& filter_;
	parameter_id id_;
	std::string name_;

public:
	explicit filter_parameter(filter&);
	
	void set_name(const std::string& nm) { name_ = nm; }
	const std::string& name() const { return name_; }
};


/// Link to a parameter of type \a Value belonging to another filter.
template<typename Value>
class filter_extern_parameter {
public:
	using value_type = Value;
	using parameter_type = filter_parameter<Value>;

private:
	filter& filter_;
	parameter_id linked_id_;
	bool readable_;
	bool writable_;
	std::string name_;

public:
	explicit filter_extern_parameter(filter&);
	
	void link(parameter_type&);
	bool is_linked() const;
	
	void set_name(const std::string& nm) { name_ = nm; }
	const std::string& name() const { return name_; }	
};


#include "filter_parameter.tcc"

}}

#endif
