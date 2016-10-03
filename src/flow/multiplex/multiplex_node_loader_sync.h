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

#ifndef MF_FLOW_MULTIPLEX_NODE_LOADER_SYNC_H_
#define MF_FLOW_MULTIPLEX_NODE_LOADER_SYNC_H_

#include "multiplex_node.h"

namespace mf { namespace flow {

class multiplex_node::sync_loader : public multiplex_node::loader {
private:
	pull_result input_pull_result_ = pull_result::undefined;

public:
	explicit sync_loader(multiplex_node&);

	bool is_async() const override { return false; }

	void stop() override;
	void launch() override;
	void pre_pull(time_span) override;
	node::pull_result pull(time_span&) override;
	node_frame_window_view begin_read(time_span) override;
	void end_read(time_unit duration) override;
};

}}

#endif
