namespace mf {

template<typename Pixel>
camera_image<Pixel>::camera_image(const base& img, const projection_camera& cam) :
	base(img),
	camera_(cam, img.shape()) { }


template<typename Pixel>
camera_image<Pixel>::camera_image(base&& img, const projection_camera& cam) :
	base(std::move(img)),
	camera_(cam, img.shape()) { }

}
