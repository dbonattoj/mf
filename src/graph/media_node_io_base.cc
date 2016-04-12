#include "media_node_io_base.h"
#include "media_node_base.h"
#include <stdexcept>

namespace mf {

media_node_output_base::media_node_output_base(media_node_base& nd) :
	node_(nd)
{
	nd.register_output_(*this);
}


media_node_input_base::media_node_input_base(media_node_base& nd, time_unit past_window, time_unit future_window) :
	past_window_(past_window), future_window_(future_window)
{
	nd.register_input_(*this);
}


}
