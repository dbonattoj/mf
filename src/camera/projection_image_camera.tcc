namespace mf {

template<typename Depth>
projection_image_camera<Depth>::projection_image_camera(const projection_camera& cam, ndsize<2> image_size) :
	projection_camera(cam),
	depth_image_camera<Depth>(
		image_size,
		cam.depth_parameters().depth_min(),
		cam.depth_parameters().depth_max() - cam.depth_parameters().depth_min()
	) { }

}
