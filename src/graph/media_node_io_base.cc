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


void media_node_output_base::define_required_buffer_duration(time_unit dur) {
	buffer_duration_ = dur;
}


time_unit media_node_output_base::required_buffer_duration() const {
	if(! required_buffer_duration_is_defined()) throw std::logic_error("required buffer duration is not defined");
	return buffer_duration_;
}


bool media_node_output_base::required_buffer_duration_is_defined() const {
	return (buffer_duration_ != -1);
}



}
