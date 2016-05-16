#ifndef MF_IMPORTER_FILTER_H_
#define MF_IMPORTER_FILTER_H_

#include <utility>
#include "filter.h"
#include "../io/frame_importer.h"

namespace mf {

template<typename Importer>
class importer_filter : public source_filter {
private:
	Importer importer_;

public:
	output_type<Importer::dimension, typename Importer::elem_type> output;
	
	template<typename... Args>
	explicit importer(flow::node& nd, Args&&... args) :
		source_filter(nd, false),
		importer_(std::forward<Args>(args)...),
		output(*this) { }
	
	void setup() override {
		output.define_frame_shape(importer_.frame_shape());
	}
	
	void progress(flow::node_job& job) override {
		auto out = job.out(output);
		return importer_.read_frame(out);
		if(importer_.reached_end()) job.mark_end();
	}
};

}

#endif
