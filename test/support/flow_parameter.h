#ifndef MF_TEST_FLOW_PARAMETER_H_
#define MF_TEST_FLOW_PARAMETER_H_

#include "flow.h"
#include <mf/filter/filter.h>
#include <mf/filter/filter_parameter.h>
#include <string>
#include <vector>

namespace mf { namespace test {

class parameter_passthrough_filter : public passthrough_filter {
	using base = passthrough_filter;
	
public:
	using param_type = parameter_type<std::string>;
	using extern_param_type = extern_parameter_type<std::string>;

private:
	std::vector<param_type> params_;
	std::vector<extern_param_type> extern_params_;

	std::vector<param_type*> set_to_t_params_;
	std::vector<extern_param_type*> verify_is_t_params_;
	
	std::map<const extern_param_type*, std::string> expected_values_;

public:
	parameter_passthrough_filter() : passthrough_filter() { }

	param_type& add_param(bool set_to_t) {
		params_.emplace_back(*this);
		param_type& param = params.back();
		extern_params_.push_back(&param);
		return param;
	}
	
	extern_param_type& add_extern_param(bool verify_is_t) {
		extern_params_.emplace_back(*this);
		extern_param& param = extern_params_.back();
		if(verify_is_t) verify_is_t_params_.push_back(&param);
		return param;
	}
	
	void set_expected_value(const extern_param_type& par, const std::string& val) {
		expected_values_[par] = val;
	}
	
	void reset_expected_values() {
		expected_values_.clear();
	}
	
	void process(flow::filter_job& job) override {
		base::process(job);
		
		time_unit t = job.time();
		std::string t_str = std::to_string(t);
		
		for(param_type* par : set_to_t_params_)
			job.set_param(*par, t_str);

		for(extern_param_type* par : verify_is_t_params_)
			REQUIRE(job.param(*par) == t_str);
			
		for(auto&& pv : expected_values_)
			REQUIRE(job.param(pv.first) == pv.second);
	}
};


}}

#endif
