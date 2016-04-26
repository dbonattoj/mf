#ifndef MF_NODE_EXPORTER_H_
#define MF_NODE_EXPORTER_H_

#include <utility>
#include <tuple>
#include "../io/frame_importer.h"
#include "../flow/sink_node.h"

namespace mf { namespace node {

template<typename Exporter>
class exporter : public flow::sink_node {
private:
	Exporter exporter_;

public:
	input_type<Exporter::dimension, typename Exporter::elem_type> in;

	template<typename... Args>
	explicit exporter(Args&&... args) :
		exporter_(std::forward<Args>(args)...), in(*this) { }
	
	void setup() override {
		// TODO define frame shape after exporter construction
	}
	
	void process() override {
		exporter_.write_frame(in.view());
	}
};

}}

#endif
