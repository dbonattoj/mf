#ifndef MF_EXPORTER_FILTER_H_
#define MF_EXPORTER_FILTER_H_

#include <utility>
#include "filter.h"
#include "../io/frame_exporter.h"

namespace mf { namespace flow {

template<typename Exporter>
class exporter_filter : public sink_filter {
private:
	Exporter exporter_;

public:
	input_type<Importer::dimension, typename Importer::elem_type> input;
	
	template<typename... Args>
	explicit exporter(sink_node& nd, Args&&... args) :
		sink_filter(nd),
		exporter_(std::forward<Args>(args)...),
		input(*this) { }
	
	void setup() override {
		//
	}
	
	void progress(node_job& job) override {
		auto in = job.in(input);
		exporter_.write_frame(in);
	}
};

}}

#endif
