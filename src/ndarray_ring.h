#ifndef MF_NDARRAY_RING_H_
#define MF_NDARRAY_RING_H_

#include <functional>
#include "ndcoord.h"
#include "ndarray.h"
#include "ring_allocator.h"

namespace mf {

template<std::size_t Dim, typename T>
class ndarray_ring : public ndarray<Dim + 1, T, ring_allocator<T>> {
	using base = ndarray<Dim + 1, T, ring_allocator<T>>;

public:
	using section_view_type = ndarray_view<Dim + 1, T>;
	using const_section_view_type = ndarray_view<Dim + 1, const T>;

	using typename base::padding_type;

private:
	std::size_t read_position_ = 0;
	std::size_t write_position_ = 0;
	bool full_ = false;
	
	static padding_type adjust_padding_(const ndsize<Dim>& spatial_shape, std::size_t duration); 
	section_view_type section_(std::ptrdiff_t start, std::size_t duration);

public:
	ndarray_ring(const ndsize<Dim>& frames_shape, std::size_t duration);
	
	std::size_t readable_duration() const noexcept;
	std::size_t writable_duration() const noexcept;
	
	section_view_type write(std::size_t duration);
	void did_write(std::size_t written_duration);
	
	const_section_view_type read(std::size_t duration);
	void did_read(std::size_t read_duration);
	
	void skip(std::size_t duration);
	
	std::size_t total_duration() const noexcept { return base::shape().front(); }
};

}

#include "ndarray_ring.tcc"

#endif
