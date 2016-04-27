#ifndef MF_RING_H_
#define MF_RING_H_

#include "../ndarray/ndarray_generic.h"

namespace mf {

class ring : public ndarray_generic<raw_ring_allocator> {
	using base = ndarray_generic<raw_ring_allocator>;
	
public:
	using section_view_type = ndarray_view_generic;

private:
	time_unit read_position_ = 0;
	time_unit write_position_ = 0;
	bool full_ = false;
		
	static std::size_t adjust_padding_(const frame_properties&, std::size_t duration); 
	section_view_type section_(time_unit start, time_unit duration);

public:
	ring(const frame_properties&, time_unit duration);
	
	void initialize();
	
	time_unit total_duration() const noexcept { return base::shape().front(); }
	time_unit frame_length() const noexcept { return base::shape().back(); }
	
	time_unit writable_duration() const;
	time_unit readable_duration() const;
	
	section_view_type begin_write(time_unit duration);
	void end_write(time_unit written_duration);
	
	section_view_type begin_read(time_unit duration);
	void end_read(time_unit read_duration);
		
	void skip(time_unit duration);
};


}

#endif

