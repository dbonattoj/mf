#ifndef MF_REVERSE_HOMOGRAPHY_WARP_NODE_H_
#define MF_REVERSE_HOMOGRAPHY_WARP_NODE_H_

#include "../flow/async_node.h"
#include "../camera/projection_image_camera.h"
#include "../color.h"
#include "../masked_elem.h"

namespace mf { namespace node {
	
template<typename Color, typename Depth>
class reverse_homography_warp : public flow::async_node {
public:
	using camera_type = projection_image_camera<Depth>;
	using masked_depth_type = masked_elem<Depth>;
	using masked_image_type = masked_elem<Color>;

public:
	input_type<2, Color> source_image_input;
	input_type<2, masked_depth_type> destination_depth_input;
	output_type<2, masked_image_type> destination_image_output;
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> destination_camera;
		
	reverse_homography_warp(flow::graph& gr) :
		flow::async_node(gr),
		source_image_input(*this), destination_depth_input(*this), destination_image_output(*this) { }

protected:
	void setup() override;
	void process(flow::node_job&) override;
};

}}

#include "reverse_homography_warp.tcc"

#endif
