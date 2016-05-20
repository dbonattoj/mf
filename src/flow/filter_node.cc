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

#include "filter_node.h"
#include "../filter/filter.h"

namespace mf { namespace flow {

filter_node::filter_node(graph& gr) : node(gr) { }

filter_node::~filter_node() { }

void filter_node::setup_filter() {
	MF_EXPECTS(filter_ != nullptr);
	filter_->setup();
}

void filter_node::pre_process_filter(node_job& job) {
	MF_EXPECTS(filter_ != nullptr);
	filter_->pre_process(job);
}

void filter_node::process_filter(node_job& job) {
	MF_EXPECTS(filter_ != nullptr);
	filter_->process(job);
}


}}
