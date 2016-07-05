/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MF_IMPORTER_FILTER_H_
#define MF_IMPORTER_FILTER_H_

#include <utility>
#include "filter.h"
#include "../io/frame_importer.h"
#include "../io/seekable_frame_importer.h"

namespace mf { namespace flow {

/// Importer source filter, reads frames from associated \ref frame_importer.
template<typename Importer, typename = void>
class importer_filter : public source_filter {
	static_assert(! Importer::is_seekable, "Importer must not be seekable");

private:
	Importer importer_;

public:
	output_type<Importer::dimension, typename Importer::elem_type> output;
	
	template<typename... Args>
	explicit importer_filter(filter_node& nd, Args&&... args) :
		source_filter(nd, false),
		importer_(std::forward<Args>(args)...),
		output(*this) { }
	
	void setup() override {
		output.define_frame_shape(importer_.frame_shape());
	}
	
	void process(node_job& job) override {
		auto out = job.out(output);
		return importer_.read_frame(out);
		if(importer_.reached_end()) job.mark_end();
	}
};


/// Seekable importer source filter, reads frames from associated \ref seekable_frame_importer.
template<typename Importer>
class importer_filter<Importer, std::enable_if_t<Importer::is_seekable>> : public source_filter {
	static_assert(Importer::is_seekable, "Importer must be seekable");
	
private:
	Importer importer_;

public:
	output_type<Importer::dimension, typename Importer::elem_type> output;
	
	template<typename... Args>
	explicit importer_filter(filter_node& nd, Args&&... args) :
		source_filter(nd, false),
		importer_(std::forward<Args>(args)...),
		output(*this)
	{
		set_seekable(true);
	}
	
	void set_seekable(bool seekable) {
		if(seekable) this_node().define_source_stream_properties(true, importer_.total_duration());
		else this_node().define_source_stream_properties(false, -1);
	}
	
	void setup() override {
		output.define_frame_shape(importer_.frame_shape());
	}
	
	void pre_process(node_job& job) override {
		time_unit t = job.time();
		if(importer_.current_time() != t) importer_.seek(t);
	}
	
	void process(node_job& job) override {
		MF_ASSERT(importer_.current_time() == job.time());
		auto out = job.out(output);
		return importer_.read_frame(out);
	}
};


}}

#endif
