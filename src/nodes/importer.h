#ifndef MF_NODE_IMPORTER_H_
#define MF_NODE_IMPORTER_H_

#include <utility>
#include <type_traits>
#include "../io/frame_importer.h"
#include "../io/seekable_frame_importer.h"
#include "../flow/sync_node.h"

#include <iostream>

namespace mf { namespace node {

/// Source node which reads frames from frame importer object.
template<typename Importer>
class importer : public flow::sync_source_node {
private:
	Importer importer_;

public:
	output_type<Importer::dimension, typename Importer::elem_type> output;
	
	template<typename... Args>
	explicit importer(flow::graph& gr, Args&&... args) :
		flow::sync_source_node(gr, false), importer_(std::forward<Args>(args)...), output(*this) { }
	
	void setup() override {
		output.define_frame_shape(importer_.frame_shape());
	}
	
	void process(flow::node_job& job) override {
		auto out = job.out(output);
		return importer_.read_frame(out);
		if(importer_.reached_end()) job.mark_end();
	}
};


/// Source node which reads frames from seekable frame importer object.
/** Leverages the seek ability of the frame importer to the node. */
template<typename Seekable_importer>
class seekable_importer : public flow::sync_source_node {
private:
	Seekable_importer importer_;

public:
	output_type<Seekable_importer::dimension, typename Seekable_importer::elem_type> output;
	
	template<typename... Args>
	explicit seekable_importer(flow::graph& gr, Args&&... args) :
		flow::sync_source_node(gr), importer_(std::forward<Args>(args)...), output(*this)
	{
		define_source_stream_properties(true, importer_.total_duration());
	}
	
	void setup() override {
		output.define_frame_shape(importer_.frame_shape());
	}
	
	void pre_process(flow::node_job& job) override {		
		time_unit t = job.time();
		if(importer_.current_time() != t) importer_.seek(t);
	}
	
	void process(flow::node_job& job) override {
		MF_ASSERT(importer_.current_time() == job.time());
		auto out = job.out(output);
		return importer_.read_frame(out);
	}
};


}}

#endif
