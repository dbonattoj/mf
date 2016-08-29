#ifndef MF_FLOW_DIAGNOSTIC_HANDLER_H_
#define MF_FLOW_DIAGNOSTIC_HANDLER_H_

#include "../../common.h"

namespace mf { namespace flow {

class processing_node;
class graph;

class diagnostic_handler {
public:
	virtual void processing_node_job_started(const processing_node&, time_unit t) = 0;
	virtual void processing_node_job_finished(const processing_node&, time_unit t) = 0;
	virtual void launched(const graph&) = 0;
	virtual void stopped(const graph&) = 0;	
};

}}

#endif
