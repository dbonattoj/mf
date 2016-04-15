#ifndef MF_NODE_IMPORTER_H_
#define MF_NODE_IMPORTER_H_

#include <utility>
#include "../io/frame_importer.h"
#include "../flow/node.h"
#include "../flow/node_input.h"
#include "../flow/node_output.h"

namespace mf { namespace node {

template<typename Importer>
class importer : public flow::source_node {
private:
	Importer importer_;

public:
	output_type<Importer::dimension, typename Importer::elem_type> output;

	template<typename... Args>
	explicit importer(Args&&... args) :
		importer_(std::forward<Args>(args)...), output(*this) { }
	
	void setup() override {
		output.define_frame_shape(importer_.frame_shape());
	}
	
	void process() override {
		return importer_.read_frame(output.view());
	}
	
	bool reached_end() override {
		return importer_.reached_end();
	}
};

}}

#endif
