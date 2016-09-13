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

#include "node_parameter_valuation.h"
#include <iterator>

namespace mf { namespace flow {

bool node_parameter_valuation::has(node_parameter_id id) const {
	return (values_.find(id) != values_.end());
}


const node_parameter_value& node_parameter_valuation::operator()(node_parameter_id id) const {
	return values_.at(id);
}


node_parameter_value& node_parameter_valuation::operator()(node_parameter_id id) {
	return values_.at(id);
}


void node_parameter_valuation::set(node_parameter_id id, const node_parameter_value& val) {
	if(has(id)) values_.at(id) = val;
	else values_.emplace(id, val);
}


void node_parameter_valuation::set_all(const node_parameter_valuation& val) {
	for(const auto& iv : val.values_) set(iv.first, iv.second);
}


void node_parameter_valuation::set_all(node_parameter_valuation&& val) {
	for(const auto& iv : val.values_) set(iv.first, std::move(iv.second));
}


void node_parameter_valuation::erase(node_parameter_id id) {
	values_.erase(id);
}


void node_parameter_valuation::clear() {
	values_.clear();
}


node_parameter_valuation combine(const node_parameter_valuation& a, const node_parameter_valuation& b) {
	node_parameter_valuation ab = a;
	ab.values_.insert(b.values_.cbegin(), b.values_.cend());
	return ab;
}

}}
