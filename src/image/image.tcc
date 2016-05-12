namespace mf {


template<typename Pixel_in, typename Pixel_out, typename Kernel, typename Function>
void apply_kernel(const image<Pixel_in>& in_img, image<Pixel_out>& out_img, const ndsize<2>& kernel_shp, Function func);
	if(input_img.shape() != output_img.shape())
		throw std::invalid_argument("input and output of apply_kernel must have same shapes");

	ndptrdiff<2> kernel_radius = kernel_shp / 2;
	for(auto out_it = output_img.view(); out_it != output_img.end(); ++out_it) {
		ndptrdiff<2> coord = it.coordinates();
		ndspan<2> span(coord - kernel_radius, coord + kernel_radius);
		MF_ASSERT(span.shape() == kernel_shp);
		
		ndspan<2> full_span(0, view_.shape());
		ndspan<2> truncated_span = span_intersection(full_span, span);
		
		ndarray_view<2, const Pixel> in_section = input_img.cview().section(truncated_span);
		
		MF_ASSERT(section.shape() == kernel_shp);
		
		ndptrdiff<2> center(
			kernel_radius + std::min(span.start_pos()[0], std::ptrdiff_t(0)),
			kernel_radius + std::min(span.start_pos()[1], std::ptrdiff_t(0))
		);

		MF_ASSERT(span.includes(center));
		MF_ASSERT(&(*it) == &section.at(center));
		
		Pixel new_value = func(in_section, center);
		*out_it = new_value;
	}
}

template<typename Pixel_in, typename Pixel_out, typename Kernel>
void convolve_kernel(const image<Pixel_in>& input_img, image<Pixel_out>& output_img, const Kernel& kernel) {
	apply_kernel(input_img, output_img, kernel,
	[&kernel](const ndarray_view<2, const Pixel_in>& section, const ndptrdiff<2>& center) -> Pixel_out {
		
	});
}
	
}

}
