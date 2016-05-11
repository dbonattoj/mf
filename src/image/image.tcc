namespace mf {

template<typename Pixel> template<typename Kernel, typename Function>
void image<Pixel>::apply_kernel(const Kernel& kernel, Function func) {
	ndptrdiff<2> kernel_radius = kernel.shape() / 2;
	for(auto it : view_) {
		ndptrdiff<2> coord = it.coordinates();
		ndspan<2> span(coord - kernel_radius, coord + kernel_radius);
		MF_ASSERT(span.shape() == kernel.shape());
		
		ndspan<2> full_span(0, view_.shape());
		ndspan<2> truncated_span = span_intersection(full_span, span);
		
		ndarray_view<2, const Pixel> section = view_.section(truncated_span);
		
		MF_ASSERT(section.shape() == kernel.shape());
		
		ndptrdiff<2> center(
			kernel_radius[0] + std::min(span.start_pos()[0], std::ptrdiff_t(0)),
			kernel_radius[1] + std::min(span.start_pos()[1], std::ptrdiff_t(0))
		);

		MF_ASSERT(span.includes(center));
		MF_ASSERT(&(*it) == &section.at(center));
		
		Pixel new_value = func(section, kernel.cview(), center);
		*it = new_value;
	}
}


template<typename Pixel> template<typename Kernel>
void image<Pixel>::convolve_kernel(const Kernel& kernel) {
	
}

}
