#include "node_io_base.h"
#include "node_base.h"
#include <stdexcept>
#include <cassert>

namespace mf { namespace flow {

node_output_base::node_output_base(node_base& nd) :
	node_(nd)
{
	nd.register_output_(*this);
}


void node_output_base::propagate_activation(bool input_activated) {
	input_activated_ = input_activated;
	node_.propagate_activation();
}


bool node_output_base::is_active() const {
	return input_activated_ && node_.is_active();
}


node_input_base::node_input_base(node_base& nd, time_unit past_window, time_unit future_window) :
	past_window_(past_window), future_window_(future_window)
{
	nd.register_input_(*this);
}


void node_input_base::activate() {
	if(! activated_) {
		connected_output().propagate_activation(true);
		activated_ = true;
	}
}


void node_input_base::desactivate() {
	if(activated_) {
		connected_output().propagate_activation(false);
		activated_ = false;
	}
}



}}
