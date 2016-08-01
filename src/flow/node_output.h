#ifndef MF_FLOW_NODE_OUTPUT_H_
#define MF_FLOW_NODE_OUTPUT_H_

#include "node.h"
#include "node_remote_output.h"

namespace mf { namespace flow {


/// Output port of node in node graph.
/** One output port have multiple *channels*. The output is *pulled* as a whole, but data is read from individual
 ** channels. The channels may have different formats. */
class node_output : public node_remote_output {
private:
	node& node_;
	
	node_input* connected_input_ = nullptr;

protected:
	explicit node_output(node& nd);
	node_output(const node_output&) = delete;
	node_output& operator=(const node_output&) = delete;
	virtual ~node_output() = default;

public:
	node& this_node() const noexcept { return node_; }

	bool is_connected() const noexcept { return (connected_input_ != nullptr); }
	node_input& connected_input() const noexcept { MF_EXPECTS(is_connected()); return *connected_input_; }
	node& connected_node() const noexcept;
	void input_has_connected(node_input&) final override;
	void input_has_disconnected() final override;
	
	time_unit end_time() const noexcept final override;

	bool is_online() const;
};


/*
/// Output port of node in flow graph.
class node_output : public node_remote_output {
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
*/

}}

#endif
