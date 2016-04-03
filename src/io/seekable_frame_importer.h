#ifndef MF_SEEKABLE_FRAME_IMPORTER_H_
#define MF_SEEKABLE_FRAME_IMPORTER_H_

#include "frame_importer.h"
#include "../common.h"
#include "../ndarray/ndcoord.h"
#include "../ndarray/ndarray_view.h"

namespace mf {

template<std::size_t Dim, typename Elem>
class seekable_frame_importer : public frame_importer<Dim, Elem> {
	using base = frame_importer<Dim, Elem>;
	
protected:
	seekable_frame_importer(const ndsize<Dim>& frame_shape) :
		base(frame_shape) { }

public:
	using typename base::frame_view_type;
	
	virtual time_unit current_time() const = 0;
	virtual time_unit total_duration() const = 0;
	
	virtual void seek(time_unit) = 0;
	
	void read_frame_at(const frame_view_type& vw, time_unit t) {
		this->seek(t);
		this->read_frame(vw);
	}
};


}

#endif

