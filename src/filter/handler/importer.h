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
#include "../filter_handler.h"
#include "../../io/frame_importer.h"
#include "../../io/seekable_frame_importer.h"

namespace mf { namespace flow {

/// Importer source filter, reads frames from associated \ref frame_importer.
template<typename Importer, typename = void>
class importer_filter : public filter_handler {
	static_assert(! Importer::is_seekable, "Importer must not be seekable");

private:
	Importer importer_;
	
	void skip_to_(time_unit t) {
		ndarray<Importer::dimension, typename Importer::elem_type> buffer;
		while(importer_.current_time() < t && ! importer_.reached_end())
			importer_.read_frame(buffer);
	}

public:
	output_type<Importer::dimension, typename Importer::elem_type> output;
	
	template<typename... Args>
	explicit importer_filter(filter& filt, Args&&... args) :
		filter_handler(filt),
		importer_(std::forward<Args>(args)...),
		output(filt)
	{
		stream_timing tm;
		tm.set_frame_clock_duration(std::chrono::milliseconds(100000));
		this_filter().set_own_timing(tm);	
	}
	
	void setup() override {
		output.define_frame_shape(importer_.frame_shape());
	}
	
	void pre_process(job_type& job) override {
		time_unit t = job.time();
		if(importer_.current_time() < t) skip_to_(t);
		else if(importer_.current_time() > t) throw std::runtime_error("cannot seek back (importer is not seekable)");
		
		if(importer_.reached_end()) job.mark_end();
	}
	
	void process(job_type& job) override {
		Assert(! importer_.reached_end());
		auto out = job.out(output);
		importer_.read_frame(out);
	}
};


/// Seekable importer source filter, reads frames from associated \ref seekable_frame_importer.
template<typename Importer>
class importer_filter<Importer, std::enable_if_t<Importer::is_seekable>> : public filter_handler {
	static_assert(Importer::is_seekable, "Importer must be seekable");
	
private:
	Importer importer_;

public:
	output_type<Importer::dimension, typename Importer::elem_type> output;
	
	template<typename... Args>
	explicit importer_filter(filter& filt, Args&&... args) :
		filter_handler(filt),
		importer_(std::forward<Args>(args)...),
		output(filt)
	{
		stream_timing tm;
		tm.set_frame_clock_duration(std::chrono::milliseconds(500));
		this_filter().set_own_timing(tm);		
	}
	
	
	void setup() override {
		output.define_frame_shape(importer_.frame_shape());
	}
	
	void pre_process(job_type& job) override {
		time_unit t = job.time();
		if(importer_.current_time() != t) importer_.seek(t);
		
		if(importer_.reached_end()) job.mark_end();
	}
	
	void process(job_type& job) override {
		Assert(! importer_.reached_end());
		Assert(importer_.current_time() == job.time());
		auto out = job.out(output);
		importer_.read_frame(out);
	}
};


}}

#endif
