#ifndef MF_NODE_EXPORTER_H_
#define MF_NODE_EXPORTER_H_

#include <utility>
#include <tuple>
#include "../io/frame_importer.h"
#include "../flow/node.h"
#include "../flow/node_input.h"
#include "../flow/node_output.h"

namespace mf { namespace node {

template<typename Exporter>
class exporter : public flow::sink_node {
private:
	Exporter exporter_;

public:
	input_type<Exporter::dimension, typename Exporter::elem_type> input;

	template<typename... Args>
	explicit exporter(Args&&... args) :
		exporter_(std::forward<Args>(args)...), input(*this) { }
	
	void setup_() override {
		// TODO define frame shape after exporter construction
	}
	
	void process_() override {
		exporter_.write_frame(input.view());
	}
};

}}

#endif
