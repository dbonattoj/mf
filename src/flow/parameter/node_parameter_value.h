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

#ifndef MF_FLOW_NODE_PARAMETER_VALUE_H_
#define MF_FLOW_NODE_PARAMETER_VALUE_H_

namespace mf { namespace flow {

/// Holder for node parameter value, abstract base class.
/** Derived by \ref filter_parameter_holder, in which filter stores concrete type. */
class node_parameter_holder {
private:
	node_parameter_holder(const node_parameter_holder&) = delete;
	node_parameter_holder& operator=(node_parameter_holder&) = delete;
	
public:
	virtual ~node_parameter_holder() = 0;
	virtual node_parameter_holder* clone() const = 0;
};


/// Container of node parameter value.
class node_parameter_value {
private:	
	std::unique_ptr<node_parameter_holder> holder_;

public:
	node_parameter_value();
	node_parameter_value(const node_parameter_value&);
	node_parameter_value(node_parameter_value&&);
	
	node_parameter_value& operator=(const node_parameter_value&);
	node_parameter_value& operator=(node_parameter_value&&);
};

}}

#endif
