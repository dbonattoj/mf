#ifndef MF_HOMOGRAPHY_DEPTH_WARP_NODE_H_
#define MF_HOMOGRAPHY_DEPTH_WARP_NODE_H_

#include "../flow/async_node.h"
#include "../camera/projection_image_camera.h"
#include "../color.h"
#include "../masked_elem.h"

namespace mf { namespace node {
	
template<typename Depth>
class homography_depth_warp : public flow::async_node {
public:
	using camera_type = projection_image_camera<Depth>;
	using masked_depth_type = masked_elem<Depth>;

public:

	input_type<2, masked_depth_type> source_depth_input;
	output_type<2, masked_depth_type> destination_depth_output;
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> destination_camera;
		
	homography_depth_warp(flow::graph& gr) :
		flow::async_node(gr),
		source_depth_input(*this), destination_depth_output(*this) { }

protected:
	void setup() override;
	void process(flow::node_job&) override;
};

}}

#include "homography_depth_warp.tcc"

#endif
