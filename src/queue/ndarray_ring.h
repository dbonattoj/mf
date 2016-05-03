#ifndef MF_NDARRAY_RING_H_
#define MF_NDARRAY_RING_H_

#include "ring.h"
#include "timed_ring.h"
#include "shared_ring.h"

namespace mf {

namespace detail {
	template<std::size_t Dim, typename Elem>
	class ndarray_ring_base {
	public:
		using frame_shape_type = ndsize<Dim>;
		using frame_view_type = ndarray_view<Dim, Elem>;
		using section_view_type = ndarray_view<Dim + 1, Elem>;

	private:
		frame_shape_type frame_shape_;

	protected:
		frame_array_properties properties_(std::size_t duration) const noexcept {
			return frame_array_properties(
				frame_format::default_format<Elem>(),
				frame_shape_.product(),
				duration
			);
		}
	
	public:
		explicit ndarray_ring_base(const frame_shape_type& shp) : frame_shape_(shp) { }
	
		const frame_shape_type& frame_shape() const noexcept { return frame_shape_; }
	};
}


template<std::size_t Dim, typename Elem>
class ndarray_ring : public detail::ndarray_ring_base<Dim, Elem>, public ring {
	using base = detail::ndarray_ring_base<Dim, Elem>;

public:
	using frame_shape_type = typename base::frame_shape_type;
	using frame_view_type = typename base::frame_view_type;
	using section_view_type = typename base::section_view_type;
	
	ndarray_ring(const frame_shape_type& frame_shape, time_unit dur) :
		base(frame_shape),
		ring(base::properties_(dur)) { }
	
	section_view_type begin_write(time_unit duration) {
		return from_generic<Dim, Elem>(ring::begin_write(duration), base::frame_shape());
	}
	
	section_view_type begin_read(time_unit duration) {
		return from_generic<Dim, Elem>(ring::begin_read(duration), base::frame_shape());
	}
};


template<std::size_t Dim, typename Elem>
class ndarray_timed_ring : public detail::ndarray_ring_base<Dim, Elem>, public timed_ring {
	using base = detail::ndarray_ring_base<Dim, Elem>;

public:
	using frame_shape_type = typename base::frame_shape_type;
	using frame_view_type = typename base::frame_view_type;
	using section_view_type = ndarray_timed_view<Dim + 1, Elem>;
	
	ndarray_timed_ring(const frame_shape_type& frame_shape, time_unit dur) :
		base(frame_shape),
		timed_ring(base::properties_(dur)) { }
	
	section_view_type begin_write(time_unit duration) {
		return from_generic_timed<Dim, Elem>(timed_ring::begin_write(duration), base::frame_shape());
	}
	
	section_view_type begin_write_span(time_span span) {
		return from_generic_timed<Dim, Elem>(timed_ring::begin_write_span(span), base::frame_shape());
	}
	
	section_view_type begin_read(time_unit duration) {
		return from_generic_timed<Dim, Elem>(timed_ring::begin_read(duration), base::frame_shape());
	}
	
	section_view_type begin_read_span(time_span span) {
		return from_generic_timed<Dim, Elem>(timed_ring::begin_read_span(span), base::frame_shape());
	}
};


template<std::size_t Dim, typename Elem>
class ndarray_shared_ring : public detail::ndarray_ring_base<Dim, Elem>, public shared_ring {
	using base = detail::ndarray_ring_base<Dim, Elem>;

public:
	using frame_shape_type = typename base::frame_shape_type;
	using frame_view_type = typename base::frame_view_type;
	using section_view_type = ndarray_timed_view<Dim + 1, Elem>;
	
	ndarray_shared_ring(const frame_shape_type& frame_shape, std::size_t dur, bool seekable, time_unit end_time = -1) :
		base(frame_shape),
		shared_ring(base::properties_(dur), seekable, end_time) { }
	
	section_view_type begin_write(time_unit write_duration) {
		return from_generic_timed<Dim, Elem>(shared_ring::begin_write(write_duration), base::frame_shape());
	}
	
	bool try_begin_write(time_unit write_duration, section_view_type& section) {
		shared_ring::section_view_type generic_section;
		if(shared_ring::try_begin_write(write_duration, generic_section)) {
			section.reset(from_generic_timed<Dim, Elem>(generic_section, base::frame_shape()));
			return true;
		} else {
			return false;
		}
	}
	
	section_view_type begin_read_span(time_span span) {
		return from_generic_timed<Dim, Elem>(shared_ring::begin_read_span(span), base::frame_shape());
	}
	
	section_view_type begin_read(time_unit read_duration) {
		return from_generic_timed<Dim, Elem>(shared_ring::begin_read(read_duration), base::frame_shape());
	}
	
	bool try_begin_read(time_unit read_duration, section_view_type& section) {
		shared_ring::section_view_type generic_section;
		if(shared_ring::try_begin_read(read_duration, generic_section)) {
			section.reset(from_generic_timed<Dim, Elem>(generic_section, base::frame_shape()));
			return true;
		} else {
			return false;
		}
	}
};

}

#endif
