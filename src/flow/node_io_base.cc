#include "node_io_base.h"
#include "node_base.h"
#include <stdexcept>

namespace mf { namespace flow {

node_output_base::node_output_base(node_base& nd) :
	node_(nd)
{
	nd.register_output_(*this);
}


node_input_base::node_input_base(node_base& nd, time_unit past_window, time_unit future_window) :
	past_window_(past_window), future_window_(future_window)
{
	nd.register_input_(*this);
}


}}
