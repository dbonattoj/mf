#ifndef MF_FRAME_EXPORTER_H_
#define MF_FRAME_EXPORTER_H_

#include "../common.h"
#include "../ndarray/ndcoord.h"
#include "../ndarray/ndarray_view.h"

namespace mf {

template<std::size_t Dim, typename Elem>
class frame_exporter {
private:
	ndsize<Dim> frame_shape_;

protected:
	frame_exporter(const ndsize<Dim>& frame_shape) :
		frame_shape_(frame_shape) { }

public:
	using frame_view_type = ndarray_view<Dim, Elem>;
	using elem_type = Elem;
	constexpr static std::size_t dimension = Dim;

	virtual ~frame_exporter() { }
	
	const ndsize<Dim>& frame_shape() { return frame_shape_; }
	
	virtual void write_frame(const frame_view_type&) = 0;
	virtual void close() { }
};


}

#endif

