#ifndef MF_HOMOGRAPHY_DEPTH_WARP_NODE_H_
#define MF_HOMOGRAPHY_DEPTH_WARP_NODE_H_

#include "../flow/async_node.h"
#include "../camera/projection_image_camera.h"
#include "../color.h"

namespace mf { namespace node {
	
template<typename Depth>
class homography_depth_warp : public flow::async_node {
public:
	using camera_type = projection_image_camera<Depth>;

private:
	Depth null_depth_;

public:
	input_type<2, Depth> source_depth_input;
	output_type<2, Depth> destination_depth_output;
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> destination_camera;
		
	homography_depth_warp(flow::graph& gr, Depth null_depth) :
		flow::async_node(gr), null_depth_(null_depth),
		source_depth_input(*this), destination_depth_output(*this) { }

protected:
	void setup() override;
	void process(flow::node_job&) override;
};

}}

#include "homography_depth_warp.tcc"

#endif
