#ifndef MF_NODE_EXPORTER_H_
#define MF_NODE_EXPORTER_H_

#include <utility>
#include <tuple>
#include "../io/frame_importer.h"
#include "../flow/sink_node.h"

#include <iostream>

namespace mf { namespace node {

template<typename Exporter>
class exporter : public flow::sink_node {
private:
	Exporter exporter_;

public:
	input_type<Exporter::dimension, typename Exporter::elem_type> input;

	template<typename... Args>
	explicit exporter(flow::graph& gr, Args&&... args) :
		flow::sink_node(gr), exporter_(std::forward<Args>(args)...), input(*this) { }
	
	void setup() override {
		// TODO define frame shape after exporter construction
	}
	
	void process(flow::node_job& job) override {
		auto in = job.in(input);
		exporter_.write_frame(in);
	}
};

}}

#endif
