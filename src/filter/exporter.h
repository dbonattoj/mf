#ifndef MF_EXPORTER_FILTER_H_
#define MF_EXPORTER_FILTER_H_

#include <utility>
#include "filter.h"
#include "../io/frame_exporter.h"

namespace mf { namespace flow {

/// Exporter sink filter, writes frames to associated \ref frame_exporter.
template<typename Exporter>
class exporter_filter : public sink_filter {
private:
	Exporter exporter_;

public:
	input_type<Exporter::dimension, typename Exporter::elem_type> input;
	
	template<typename... Args>
	explicit exporter_filter(filter_node& nd, Args&&... args) :
		sink_filter(nd),
		exporter_(std::forward<Args>(args)...),
		input(*this) { }
	
	void setup() override {
		//
	}
	
	void process(node_job& job) override {
		auto in = job.in(input);
		exporter_.write_frame(in);
	}
};

}}

#endif
