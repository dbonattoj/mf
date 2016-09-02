#include "processing_timeline_json.h"
#include "processing_timeline.h"
#include "../graph.h"
#include "../processing/processing_node.h"
#include "../processing/sync_node.h"
#include "../processing/async_node.h"
#include "../processing/sink_node.h"
#include <json.hpp>
#include <ostream>
#include <istream>

namespace mf { namespace flow {

processing_timeline_json_exporter::processing_timeline_json_exporter(const processing_timeline& tl, std::ostream& out) :
	timeline_(tl),
	output_(out) { }
	
	
void processing_timeline_json_exporter::generate() {
	
}

}}
