#ifndef MF_FLOW_NODE_OUTPUT_H_
#define MF_FLOW_NODE_OUTPUT_H_

#include "node.h"
#include "node_remote_output.h"

namespace mf { namespace flow {


class node_output_base : public node_remote_output {
public:
	virtual ~node_output_base() = default;
};


/// Output port of node in flow graph.
class node_output : public node_output_base {
private:
	node& node_;
	std::ptrdiff_t index_ = -1;
	
	node_input* connected_input_ = nullptr;
	frame_format format_;
	std::size_t frame_length_;
	
public:
	node_output(node& nd, std::ptrdiff_t index);
	node_output(const node_output&) = delete;
	node_output& operator=(const node_output&) = delete;
	~node_output() override = default;

	std::ptrdiff_t index() const noexcept { return index_; }

	node& this_node() const noexcept { return node_; }
	node_output& this_output() noexcept final override { return *this; }
	time_unit end_time() const noexcept final override;
	
	void define_frame_length(std::size_t len) { frame_length_ = len; }
	void define_format(const frame_format& format) { format_ = format; }
	std::size_t frame_length() const noexcept { return frame_length_; }
	const frame_format& format() const noexcept { return format_; }

	bool is_connected() const noexcept { return (connected_input_ != nullptr); }
	node_input& connected_input() const noexcept { MF_EXPECTS(is_connected()); return *connected_input_; }
	node& connected_node() const noexcept;
	void input_has_connected(node_input&);
	void input_has_disconnected();
	
	bool is_online() const;
};


class node_output_bundle : public node_output {
private:
	std::vector<std::unique_ptr<node_output>> outputs_;
	
public:
	explicit node_output_bundle(node&);
	
	template<typename Output>
	Output& add_output() {
		Output* output = new Output(*this, outputs_.size());
		outputs_.emplace_back(output);
		return *output;
	}
};


}}

#endif
