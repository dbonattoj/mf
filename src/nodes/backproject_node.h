#ifndef MF_BACKPROJECT_NODE_H_
#define MF_BACKPROJECT_NODE_H_

#include "../graph/media_node.h"
#include "../point.h"
#include "../elem.h"

namespace mf {

template<typename Pixel, typename Image_camera>
class backproject_node : public media_node {
private:
	Image_camera camera_;
	
	ndsize<2> shape_() const;
		
public:
	using point_type = elem_tuple<point_xyz, Pixel>;

	media_node_input<2, Pixel> image_input;
	media_node_input<2, float> depth_input;
	media_node_output<2, point_type> point_cloud_output;
	
	explicit backproject_node(const Image_camera& cam) :
		camera_(cam), image_input(*this), depth_input(*this), point_cloud_output(*this) { }

protected:
	void setup_() override;
	void process_() override;
};

}

#include "backproject_node.tcc"

#endif
