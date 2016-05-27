#ifndef MF_IO_RAW_VIDEO_EXPORTER_H_
#define MF_IO_RAW_VIDEO_EXPORTER_H_

#include "frame_exporter.h"
#include "raw_video_frame_format.h"
#include "../elem.h"
#include "../image/image.h"
#include <string>
#include <iosfwd>

namespace mf {

template<typename Elem>
class raw_video_exporter : public frame_exporter<2, Elem> {
	using base = frame_exporter<2, Elem>;

public:
	using format_type = raw_video_frame_format<Elem>;
	using component_type = typename format_type::component_type;
	using typename base::frame_view_type;
	using typename base::frame_shape_type;

private:
	raw_video_frame_format<Elem> format_;
	std::ofstream output_;
	
	template<typename T> void write_raw_(const ndarray_view<2, T>&);
	
	image<component_type> scaled_component_frame_(std::ptrdiff_t component, const frame_view_type&) const;

	void write_frame_planar_(const frame_view_type&);
	void write_frame_interleaved_(const frame_view_type&);

public:
	raw_video_exporter(const std::string& filename, const format_type& format);

	void write_frame(const frame_view_type&) override;
	void close() override;
};

}

#include "raw_video_exporter.tcc"

#endif
