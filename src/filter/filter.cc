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

#include "filter.h"
#include "filter_parameter.h"
#include "filter_job.h"
#include "filter_handler.h"
#include "../flow/node_graph.h"
#include "../flow/node.h"
#include "../flow/multiplex/multiplex_node.h"
#include "../flow/timing/realtime_gate_node.h"
#include "../flow/processing/sink_node.h"
#include "../flow/processing/sync_node.h"
#include "../flow/processing/async_node.h"
#include "../flow/parameter/parameter_node.h"
#include <set>
#include <algorithm>

namespace mf { namespace flow {


bool filter::local_installation_guide::has_filter(const filter& filt) const {
	return (local_filter_nodes.find(&filt) != local_filter_nodes.end());
}


bool filter::local_installation_guide::has_filter_successors(const filter& filt) const {
	auto direct_suc = filt.direct_successors_();
	return std::all_of(direct_suc.begin(), direct_suc.end(), [this](const filter* suc_filt) {
		return has_filter(*suc_filt);
	});
}


bool filter::local_installation_guide::has_filter_predecessors(const filter& filt) const {
	auto direct_pre = filt.direct_predecessors_();
	return std::all_of(direct_pre.begin(), direct_pre.end(), [this](const filter* pre_filt) {
		return has_filter(*pre_filt);
	});
}


///////////////


const std::string filter::default_filter_name = "filter";
const std::string filter::default_filter_input_name = "in";
const std::string filter::default_filter_output_name = "out";

filter::filter(std::unique_ptr<filter_handler> hnd) :
	handler_(std::move(hnd)) { }

auto filter::direct_successors_() -> filter_reference_set {
	filter_reference_set direct_suc;	
	for(std::ptrdiff_t i = 0; i < outputs_count(); ++i) {
		const filter_output_base& out = output_at(i);
		for(std::ptrdiff_t edge = 0; edge < out.edges_count(); ++edge)
			direct_suc.emplace(&out.connected_filter_at_edge(edge));
	}
	return direct_suc;
}
auto filter::direct_successors_() const -> const_filter_reference_set {
	const_filter_reference_set direct_suc;	
	for(std::ptrdiff_t i = 0; i < outputs_count(); ++i) {
		const filter_output_base& out = output_at(i);
		for(std::ptrdiff_t edge = 0; edge < out.edges_count(); ++edge)
			direct_suc.emplace(&out.connected_filter_at_edge(edge));
	}
	return direct_suc;
}


auto filter::direct_predecessors_() -> filter_reference_set {
	filter_reference_set direct_pre;	
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		const filter_input_base& in = input_at(i);
		if(! in.is_connected()) continue;
		direct_pre.emplace(&in.connected_filter());
	}
	return direct_pre;
}
auto filter::direct_predecessors_() const -> const_filter_reference_set {
	const_filter_reference_set direct_pre;	
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		const filter_input_base& in = input_at(i);
		if(! in.is_connected()) continue;
		direct_pre.emplace(&in.connected_filter());
	}
	return direct_pre;
}




bool filter::precedes(const filter& filt) const {
	if(&filt == this) return true;
	
	auto direct_suc = direct_successors_();
	return std::any_of(direct_suc.begin(), direct_suc.end(), [&filt](const filter* suc_filt) {
		return suc_filt->precedes(filt);
	});
}



bool filter::precedes_strict(const filter& filt) const {
	if(&filt == this) return false;
	else return precedes(filt);
}


void filter::handler_pre_process(processing_node& nd, processing_node_job& job) {
	filter_job fjob(job);
	handler_->pre_process(fjob);
}


void filter::handler_process(processing_node& nd, processing_node_job& job) {
	filter_job fjob(job);
	handler_->process(fjob);
}


void filter::register_input(filter_input_base& in) {
	inputs_.push_back(&in);
}


void filter::register_output(filter_output_base& out) {
	outputs_.push_back(&out);
}


void filter::register_parameter(filter_parameter_base& param) {
	parameters_.push_back(&param);
}


void filter::set_asynchonous(bool async) {
	asynchronous_ = async;
}


bool filter::is_asynchonous() const {
	return asynchronous_;
}


void filter::set_prefetch_duration(time_unit dur) {
	prefetch_duration_ = dur;
}


time_unit filter::prefetch_duration() const {
	return prefetch_duration_;
}

bool filter::needs_multiplex_node_() const {
	// multiplex node is needed if there are >1 output edges
	// (does not imply that there are >1 direct successor filters)
	
	bool one_output_edge = false;
	for(auto&& out : outputs_) {
		std::size_t edges_count = out->edges_count();
		if(one_output_edge && edges_count >= 1) return true;
		else if(!one_output_edge && edges_count == 1) one_output_edge = true;
		else if(edges_count > 1) return true;
	}
	return false;
}


/*
bool filter::is_parallelization_join_point_() const {
	if(is_source() && parallelization_factor_ > 1) {
		return true;
	} else {
		auto direct_suc = direct_successors_();
		return std::any_of(direct_suc.begin(), direct_suc.end(), [parallelization_factor_](const filter& suc_filt) {
			return (suc_filt.parallelization_factor() != parallelization_factor_);
		});
	}
}

bool filter::is_parallelization_split_point_() const {
	if(parallelization_factor_ == 1) {
		return false;
	} else if(is_sink()) {
		return true;
	} else {
		auto direct_suc = direct_successors_();
		return std::any_of(direct_suc.begin(), direct_suc.end(), [parallelization_factor_](const filter& suc_filt) {
			return (suc_filt.parallelization_factor() != parallelization_factor_);
		});
	}
}
*/


bool filter::install_gate_node_if_needed_(processing_node& installed_node, local_installation_guide& local_guide) {	
	filter_node_group& node_group = guide.local_filter_nodes.at(this);
	Assert(node_group.multiplex == nullptr, "gate must be installed before multiplex");
	
	const stream_timing& this_timing = timing();
	
	const stream_timing* successor_timing = nullptr;
	auto successors = direct_successors_();
	if(successors.size() == 0) return false;
	for(auto&& succ : successors) {
		const stream_timing& tm = succ->timing();
		if(successor_timing == nullptr) successor_timing = &tm;
		else if(tm != *successor_timing) throw std::logic_error("filter outputs having different timings unsupported");
	}
	
	if(this_timing == *successor_timing) {
		return false;
	} else if(!this_timing.is_real_time() && successor_timing->is_real_time()) {
		realtime_gate_node& gate = guide.node_gr.add_node<realtime_gate_node>();
		gate.input().set_name("in");
		gate.output().set_name("out");
		gate.input().connect(installed_node.output());
		gate.set_name("rt gate");
		node_group.gate = &gate;
		return true;
	} else {
		throw std::logic_error("no gate available");
	}
}


bool filter::has_own_timing() const {
	return has_own_timing_;
}


void filter::set_own_timing(const stream_timing& tm) {
	own_timing_ = tm;
	has_own_timing_ = true;
}


void filter::unset_own_timing() {
	has_own_timing_ = false;
}


const stream_timing& filter::timing() const {
	if(has_own_timing()) return own_timing_;
	else if(has_common_input_timing()) return common_input_timing();
	else throw std::logic_error("timing not defined");
}


bool filter::has_common_input_timing() const {
	const stream_timing* first_connected_input_timing = nullptr;
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		const filter_input_base& in = input_at(i);
		if(! in.is_connected()) continue;
		if(first_connected_input_timing == nullptr)
			first_connected_input_timing = &in.connected_filter().timing();
		else if(in.connected_filter().timing() != *first_connected_input_timing)
			return false;
	}
	return (first_connected_input_timing != nullptr);
}


const stream_timing& filter::common_input_timing() const {
	const stream_timing* first_connected_input_timing = nullptr;
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		const filter_input_base& in = input_at(i);
		if(! in.is_connected()) continue;
		if(first_connected_input_timing == nullptr)
			first_connected_input_timing = &in.connected_filter().timing();
		else if(in.connected_filter().timing() != *first_connected_input_timing)
			throw std::logic_error("no common input timing (different input timings)");
	}
	if(first_connected_input_timing != nullptr) return *first_connected_input_timing;
	else throw std::logic_error("no common input timing (no connected inputs)");
}



void filter::propagate_setup() {
	if(was_setup_) return;
	
	for(filter_input_base* in : inputs_) {
		if(! in->is_connected()) continue;
		filter& direct_predecessor = in->connected_filter();
		direct_predecessor.propagate_setup();
	}
	
	setup_();
	
	was_setup_ = true;
}


void filter::propagate_install(installation_guide& guide) {
	if(guide.has_filter(*this)) return;
	
	for(filter_input_base* in : inputs_) {
		if(! in->is_connected()) continue;
		filter& direct_predecessor = in->connected_filter();
		direct_predecessor.propagate_install(guide);
	}
	
	install_(guide);
}


void filter::setup_() {
	handler_->setup();
	for(filter_output_base* out : outputs_)
		Assert(out->frame_shape_is_defined());
}


void filter::install_input_
(filter_input_base& in, processing_node& installed_node, const local_installation_guide& local_guide) {
	if(! in.is_connected()) return;
	
	// Add input to installed node
	processing_node_input& node_in = installed_node.add_input();
	node_in.set_past_window(in.past_window_duration());
	node_in.set_future_window(in.future_window_duration());
	in.set_index(node_in.index());
	
	node_in.set_name(in.name().empty() ? default_filter_input_name : in.name());
	
	// Get connected filter output
	const filter_output_base& connected_output = in.connected_output();
	std::ptrdiff_t connected_output_channel_index  = connected_output.index();
	const filter& connected_filter = in.connected_filter();
	Assert(local_guide.has_filter(connected_filter));

	// Get local node group for connected filter
	const filter_node_group& connected_node_group = local_guide.filter_nodes.at(&connected_filter);
	
	// Connect to connected filter's node output (processing, gate or multiplex)
	// Install through edge
	node_output* connected_node_out = nullptr;
	if(connected_node_group.multiplex != nullptr) {
		connected_node_out = &connected_node_group.multiplex->add_output(connected_output_channel_index);
		in.install_edge(*connected_node_out, 0, node_in);
		connected_node_out->set_name(node_in.name());
	} else if(connected_node_group.gate != nullptr) {
		connected_node_out = &connected_node_group.gate->output();
		in.install_edge(*connected_node_out, connected_output_channel_index, node_in);
	} else {
		connected_node_out = &connected_node_group.processing->output();
		in.install_edge(*connected_node_out, connected_output_channel_index, node_in);
	}
}


void filter::install_(installation_guide& guide, local_installation_guide& local_guide) {
	// install is called in source-to-sink order
	// predecessors of this filter have already been installed
	
	filter_node_group& node_group = local_guide.filter_nodes[this];

	// Create processing node for this filter
	processing_node* installed_node = nullptr;
	if(is_sink()) {
		sink_node& nd = guide.node_gr.add_sink<sink_node>();
		installed_node = &nd;
	} else if(asynchronous_) {
		async_node& nd = guide.node_gr.add_node<async_node>();
		nd.set_prefetch_duration(prefetch_duration_);
		installed_node = &nd;
		nd.output().set_name("out");
	} else {
		sync_node& nd = guide.node_gr.add_node<sync_node>();
		installed_node = &nd;
		nd.output().set_name("out");
	}
	installed_node->define_output_timing(timing());
	installed_node->set_name(name_.empty() ? default_filter_name : name_);
	
	installed_node->set_handler(*this);
		
	node_group.processing = installed_node;
	
	// Add node inputs for each connected filter input
	// and connect them to predecessor outputs (which have already been installed)
	for(filter_input_base* in : inputs_)
		install_input_(*in, *installed_node, local_guide);
		
	// Add parameter node if needed
	// TODO .................
		
	
	// Add gate node if necessary
	// connects directly to the processing node output
	install_gate_node_if_needed_(*installed_node, local_guide);
	
	// Add multiplex node if necessary
	// connects either directly to processing node output, or to gate output
	if(needs_multiplex_node_()) {
		multiplex_node& multiplex = guide.node_gr.add_node<multiplex_node>();
		node_group.multiplex = &multiplex;

		if(node_group.gate != nullptr) multiplex.input().connect(node_group.gate->output());
		else multiplex.input().connect(installed_node->output());

		multiplex.set_name("multiplex");
		multiplex.input().set_name("in");
	}
	
	// Add channels for outputs
	for(filter_output_base* out : outputs_) {
		if(out->edges_count() == 0) continue;
		processing_node_output_channel& out_ch = installed_node->add_output_channel();
		out->set_index(out_ch.index());
		out_ch.define_frame_format(out->frame_format());
		out_ch.set_name(out->name().empty() ? default_filter_output_name : out->name());
	}
}


}}
