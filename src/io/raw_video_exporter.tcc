#include <fstream>
#include <type_traits>
#include "../ndarray/ndarray_view_cast.h"

#include <iostream>

namespace mf {

template<typename Elem>
auto raw_video_exporter<Elem>::scaled_component_frame_
(std::ptrdiff_t component, const frame_view_type& vw) const -> image<component_type> {
	auto scale_x = format_.component_scale_x[component];
	auto scale_y = format_.component_scale_y[component];
	
	auto scaled_size = make_ndsize(vw.shape()[0] / scale_y, vw.shape()[1] / scale_x);
	
	using componentwise_view_type = ndarray_view<3, const component_type>;
	ndarray_view<2, const component_type> full_image = ndarray_view_cast<componentwise_view_type>(vw).slice(component, 2);
	
	cv::Mat_<component_type> full_image_mat;
	copy_to_opencv(full_image, full_image_mat);
	image<component_type> scaled_image(scaled_size);
	cv::resize(
		full_image_mat,
		scaled_image.cv_mat(),
		cv::Size(scaled_size[0], scaled_size[1]),
		0,
		0,
		CV_INTER_AREA
	);
	return scaled_image;
}


template<typename Elem> template<typename T>
void raw_video_exporter<Elem>::write_raw_(const ndarray_view<2, T>& vw) {
	if(vw.has_default_strides() && vw.default_strides_padding() == 0) {
		auto data = reinterpret_cast<const std::ostream::char_type*>(vw.start());
		std::streamsize length = vw.size() * sizeof(T);
		output_.write(data, length);
	} else {
		ndarray<2, std::remove_const_t<T>> copy(vw);
		write_raw_(copy.view());
	}
}


template<typename Elem>
void raw_video_exporter<Elem>::write_frame_planar_(const frame_view_type& vw) {
	for(std::ptrdiff_t component = 0; component < format_.num_components; ++component) {
		if(format_.component_scale_x[component] != 1 || format_.component_scale_y[component] != 1) {
			auto img = scaled_component_frame_(component, vw);
			write_raw_(img.view());
		} else {
			using componentwise_view_type = ndarray_view<3, const component_type>;
			auto component_view = ndarray_view_cast<componentwise_view_type>(vw).slice(component, 2);
			write_raw_(component_view);
		}
	}
}


template<typename Elem>
void raw_video_exporter<Elem>::write_frame_interleaved_(const frame_view_type& vw) {
	write_raw_(vw);
}


template<typename Elem>
raw_video_exporter<Elem>::raw_video_exporter(const std::string& filename, const format_type& format) :
	format_(format),
	output_(filename, std::ios_base::out | std::ios_base::binary) { }


template<typename Elem>
void raw_video_exporter<Elem>::write_frame(const frame_view_type& vw) {
	if(format_.interleaved) write_frame_interleaved_(vw);
	else write_frame_planar_(vw);
}


template<typename Elem>
void raw_video_exporter<Elem>::close() {
	output_.close();
}


}