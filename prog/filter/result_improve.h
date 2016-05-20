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

#ifndef PROG_RESULT_IMPROVE_FILTER_H_
#define PROG_RESULT_IMPROVE_FILTER_H_

#include <mf/filter/filter.h>
#include <mf/masked_elem.h>
#include <mf/color.h>
#include "../support/common.h"


class result_improve_filter : public mf::flow::filter {
public:
	input_type<2, mf::masked_elem<mf::rgb_color>> input;
	output_type<2, mf::masked_elem<mf::rgb_color>> output;
		
	result_improve_filter(mf::flow::filter_node& nd) :
		mf::flow::filter(nd),
		input(*this, 2, 2), output(*this) { }

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
