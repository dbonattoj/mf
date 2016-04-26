#ifndef MF_MULTIWARP_NODE_H_
#define MF_MULTIWARP_NODE_H_

#include "../flow/async_node.h"
#include "../camera/projection_image_camera.h"
#include "../color.h"
#include <memory>

namespace mf { namespace node {

class multiwarp : public flow::async_node {
public:
	using camera_type = projection_image_camera<std::uint8_t>;

	struct input_prop {
		input_prop(multiwarp& self, const camera_type& cm) : im(self), di(self), cam(cm) { }
		
		input_type<2, rgba_color> im;
		input_type<2, std::uint8_t> di;
		camera_type cam;
	};

private:
	camera_type output_camera_;

	void warp_(
		const ndarray_view<2, rgba_color> in_im,
		const ndarray_view<2, std::uint8_t> in_di,
		const ndarray_view<2, rgba_color> out_im,
		const camera_type& in_cam,
		const camera_type& out_cam
	) const;

public:
	output_type<2, rgba_color> out;
	std::vector<std::unique_ptr<input_prop>> ins;
		
	explicit multiwarp(const camera_type& cam_out) :
		output_camera_(cam_out), out(*this) { }

	input_prop& add_input_view(const camera_type& cam) {
		ins.emplace_back(new input_prop(*this, cam));
		return *ins.back();
	}

protected:
	void setup() override;
	void pre_process() override;
	void process() override;
};

}}

#endif
