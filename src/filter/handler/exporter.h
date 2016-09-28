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

#ifndef MF_EXPORTER_FILTER_H_
#define MF_EXPORTER_FILTER_H_

#include <utility>
#include "../filter_handler.h"
#include "../../io/frame_exporter.h"

namespace mf { namespace flow {

/// Exporter sink filter, writes frames to associated \ref frame_exporter.
template<typename Exporter>
class exporter_filter : public filter_handler {
private:
	Exporter exporter_;

public:
	input_type<Exporter::dimension, typename Exporter::elem_type> input;
	
	template<typename... Args>
	explicit exporter_filter(Args&&... args) :
		exporter_(std::forward<Args>(args)...),
		input(*this) { }
	
	void setup() override {
		exporter_.setup(input.frame_shape());
	}
	
	void process(job_type& job) override {
		auto in = job.in(input);
		exporter_.write_frame(in);
	}
};

}}

#endif
