#ifndef MF_HOMOGRAPHY_WARP_NODE_H_
#define MF_HOMOGRAPHY_WARP_NODE_H_

#include "../flow/async_node.h"
#include "../camera/projection_image_camera.h"
#include "../color.h"

namespace mf { namespace node {
	
template<typename Color, typename Depth>
class homography_warp : public flow::async_node {
public:
	using camera_type = projection_image_camera<Depth>;

private:
	Color background_color_;

public:
	input_type<2, Color> source_image_input;
	input_type<2, Depth> source_depth_input;
	output_type<2, Color> destination_image_output;
	parameter_type<camera_type> source_camera;
	parameter_type<camera_type> destination_camera;
		
	homography_warp(flow::graph& gr, Color background_color) :
		flow::async_node(gr), background_color_(background_color),
		source_image_input(*this), source_depth_input(*this), destination_image_output(*this) { }

protected:
	void setup() override;
	void process(flow::node_job&) override;
};

}}

#include "homography_warp.tcc"

#endif
