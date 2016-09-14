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

#ifndef MF_TEST_FLOW_PARAMETER_H_
#define MF_TEST_FLOW_PARAMETER_H_

#include "flow.h"
#include <mf/filter/filter.h>
#include <mf/filter/filter_parameter.h>
#include <string>
#include <vector>
#include <memory>

namespace mf { namespace test {

class parameter_passthrough_filter : public passthrough_filter {
	using base = passthrough_filter;
	
public:
	using param_type = parameter_type<std::string>;
	using extern_param_type = extern_parameter_type<std::string>;

private:
	std::vector<std::unique_ptr<param_type>> params_;
	std::vector<std::unique_ptr<extern_param_type>> extern_params_;

	std::vector<param_type*> set_to_t_params_;
	std::vector<extern_param_type*> verify_is_t_params_;
	
	std::map<const extern_param_type*, std::string> expected_values_;

public:
	parameter_passthrough_filter(time_unit past_window, time_unit future_window) :
		passthrough_filter(past_window, future_window) { }

	param_type& add_param(bool set_to_t) {
		params_.emplace_back(new param_type(*this));
		param_type& param = *params_.back();
		if(set_to_t) set_to_t_params_.push_back(&param);
		return param;
	}
	
	extern_param_type& add_extern_param(bool verify_is_t) {
		extern_params_.emplace_back(new extern_param_type(*this));
		extern_param_type& param = *extern_params_.back();
		if(verify_is_t) verify_is_t_params_.push_back(&param);
		return param;
	}
	
	void set_expected_value(const extern_param_type& par, const std::string& val) {
		expected_values_[&par] = val;
	}
	
	void reset_expected_values() {
		expected_values_.clear();
	}
	
	void process(flow::filter_job& job) override {
		base::process(job);
		
		time_unit start_t = job.in_full(input).start_time();
		time_unit end_t = job.in_full(input).end_time();
		
		time_unit t = job.time();
		std::string t_str = std::to_string(t);
		
		for(param_type* par : set_to_t_params_)
			job.set_param(*par, t_str);

		for(extern_param_type* par : verify_is_t_params_) {
			for(time_unit t = start_t; t < end_t; ++t)
				REQUIRE(job.in(*par, t) == std::to_string(t));
			REQUIRE(job.in(*par) == t_str);
		}
			
		for(auto&& pv : expected_values_)
			REQUIRE(job.in(*pv.first) == pv.second);
	}
};


}}

#endif
