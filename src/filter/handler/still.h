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

#ifndef MF_STILL_FILTER_H_
#define MF_STILL_FILTER_H_

#include <utility>
#include "../filter_handler.h"

namespace mf { namespace flow {

template<std::size_t Dim, typename Elem>
class still_source : public filter_handler {
private:
	ndarray<Dim, Elem> frame_;

public:
	output_type<Dim, typename Elem> output;
	
	explicit still_source(filter& filt, const ndarray_view<Dim, Elem>& vw) :
		filter_handler(filt),
		frame_(vw) { }
	
	void setup() override {
		output.define_frame_shape(frame_.shape());
	}
		
	void process(job_type& job) override {
		job.out(output) = frame_;
	}
};



}}

#endif
