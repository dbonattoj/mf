#ifndef MF_NDARRAY_OPAQUE_FRAME_FORMAT_H_
#define MF_NDARRAY_OPAQUE_FRAME_FORMAT_H_

#include "../ndarray_format.h"
#include <vector>

namespace mf {

class ndarray_opaque_frame_format {
public:
	struct part {
		std::ptrdiff_t offset;
		ndarray_format format;
	};

private:
	std::vector<part> parts_;
	std::size_t frame_size_ = 0;
	std::size_t frame_alignment_requirement_ = 1;

public:
	ndarray_opaque_frame_format() = default;
	explicit ndarray_opaque_frame_format(std::size_t frame_size, std::size_t frame_alignment_requirement = 1) :
		frame_size_(frame_size), frame_alignment_requirement_(frame_alignment_requirement) { }
	explicit ndarray_opaque_frame_format(const ndarray_format&);
	
	ndarray_opaque_frame_format(const ndarray_opaque_frame_format&) = default;
	ndarray_opaque_frame_format& operator=(const ndarray_opaque_frame_format&) = default;
	
	const part& add_part(const ndarray_format& format);
	
	std::size_t frame_size() const noexcept { return frame_size_; }
	std::size_t frame_alignment_requirement() const noexcept { return frame_alignment_requirement_; }
	
	bool is_defined() const noexcept { return (frame_size_ > 0); }
	bool is_raw() const noexcept { return is_defined() && (parts_.size() == 0); }
		
	std::size_t parts_count() const { Assert(is_defined()); return parts_.size(); }
	bool is_multi_part() const { return (parts_count() > 1); }
	const part& part_at(std::ptrdiff_t part_index) const { Assert(is_defined()); return parts_.at(part_index); }

	bool is_single_part() const { return (parts_count() == 1); }
	const ndarray_format& array_format() const { Assert(is_single_part()); return parts_.front(); }
	
	friend bool operator==(const ndarray_opaque_frame_format&, const ndarray_opaque_frame_format&);
	friend bool operator!=(const ndarray_opaque_frame_format&, const ndarray_opaque_frame_format&);
};

}

#endif

