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

#ifndef MF_FLOW_MULTIPLEX_NODE_H_
#define MF_FLOW_MULTIPLEX_NODE_H_

#include "../node_derived.h"
#include "../node_input.h"
#include "../node_output.h"

namespace mf { namespace flow {

class node_graph;
class multiplex_node;

class multiplex_node_output final : public node_output {
private:
	const std::ptrdiff_t input_channel_index_;
	
	multiplex_node& this_node() noexcept;
	const multiplex_node& this_node() const noexcept;
	
public:
	multiplex_node_output(node& nd, std::ptrdiff_t input_channel_index);
	
	std::size_t channels_count() const noexcept override;
	std::string channel_name_at(std::ptrdiff_t i) const override;
	node::pull_result pull(time_span& span, bool reconnect) override;
	node_frame_window_view begin_read(time_unit duration) override;
	void end_read(time_unit duration) override;
};


class multiplex_node_input final : public node_input {
public:
	using node_input::node_input;

	thread_index reader_thread_index() const final override;
};


/// Node which lets multiple connected inputs read from the same output.
class multiplex_node final : public node_derived<multiplex_node_input, multiplex_node_output> {
	using base = node_derived<multiplex_node_input, multiplex_node_output>;
	friend class multiplex_node_output;

private:
	class loader;
	class sync_loader;
	class async_loader;

	const node* common_successor_node_ = nullptr;
	time_unit input_past_window_ = -1;
	time_unit input_future_window_ = -1;
	
	std::unique_ptr<loader> loader_;
	node_frame_window_view loaded_input_view_;
	
	bool outputs_on_different_threads_() const;

	/// \name Loader interface
	/// Called by loader, possibly from different threads.
	///@{
	time_unit capture_successor_time_() const;

	time_span expected_input_span_(time_unit successor_time) const;

	void load_input_view_(time_unit successor_time);
	void unload_input_view_();
	
	time_unit successor_time_of_input_view_() const;
	const node_frame_window_view& input_view_() const { return loaded_input_view_; }
	node_frame_window_view& input_view_() { return loaded_input_view_; }
	///@}
	
public:
	explicit multiplex_node(node_graph&);
	~multiplex_node() override;

	time_unit minimal_offset_to(const node&) const override;
	time_unit maximal_offset_to(const node&) const override;
	
	bool is_async() const;
	thread_index loader_thread_index() const;
	
	const node& common_successor_node() const { return *common_successor_node_; }
	
	void launch() override;
	void stop() override;
	void pre_setup() override;
	void setup() override;
	
	multiplex_node_input& input() { return input_at(0); }
	const multiplex_node_input& input() const { return input_at(0); }
	multiplex_node_output& add_output(std::ptrdiff_t input_channel_index);
};


inline multiplex_node& multiplex_node_output::this_node() noexcept {
	return static_cast<multiplex_node&>(node_output::this_node());
}

inline const multiplex_node& multiplex_node_output::this_node() const noexcept {
	return static_cast<const multiplex_node&>(node_output::this_node());
}

inline thread_index multiplex_node_input::reader_thread_index() const {
	return static_cast<const multiplex_node&>(this_node()).loader_thread_index();
}


inline bool is_multiplex_node(const node& nd) {
	return (dynamic_cast<const multiplex_node*>(&nd) != nullptr);
}



}}

#endif

