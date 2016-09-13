/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MF_FLOW_NODE_FRAME_FORMAT_H_
#define MF_FLOW_NODE_FRAME_FORMAT_H_

#include "../../nd/opaque_format/opaque_multi_ndarray_format.h"
#include "node_frame_satellite.h"
#include "../../queue/frame.h"

namespace mf { namespace flow {
	
class node_frame_satellite;

/// Opaque frame format of frames passed between nodes.
/** Derives from \ref opaque_multi_ndarray_format and contains for each channel one \ref ndarray_format part.
 ** Additionally contains \ref node_frame_satellite object, which gets constructed and destructed with frame.
 ** Also allows direct access to its first array_format, so that from_opaque() can be used directly without prior
 ** use of extract_part(), on single-part frames. */
class node_frame_format : public opaque_multi_ndarray_format {
	using base = opaque_multi_ndarray_format;
	
	/*
	Memory layout of frame:
	---  <-- ndarray for with part, at offset 0
	XXX      offset 0 needed for direct access to array_frame by from_opaque
	XXX
	---  <-- ndarray for following parts,
	 :       with padding in-between as needed for
	---      alignment requirements
	XXX
	XXX
	XXX
	---
	XXX
	---
	 :       after ndarray parts: padding as needed, and
	SSS  <-- node_frame_satellite object
	 :   <-- at end: padding as needed for frame alignment
	*/
	
private:
	std::size_t satellite_offset_ = 0;
	
	///@{
	/// Get in-frame pointer to \ref node_frame_satellite object.
	node_frame_satellite* frame_satellite_ptr(frame_ptr) const;
	const node_frame_satellite* frame_satellite_ptr(const_frame_ptr) const;
	///@}

protected:
	void readjust_for_added_part_(const part& new_part, bool has_padding) override;
		
public:
	node_frame_format();
	
	///@{
	/// Get \ref node_frame_satellite object associated to the \a frame.
	node_frame_satellite& frame_satellite(frame_ptr frame) const;
	const node_frame_satellite& frame_satellite(const_frame_ptr frame) const;
	///@}
	
	void copy_frame(frame_ptr destination, const_frame_ptr origin) const override;
	bool compare_frame(const_frame_ptr a, const_frame_ptr b) const override;
	void construct_frame(frame_ptr) const override;
	void destruct_frame(frame_ptr) const override;
	void initialize_frame(frame_ptr) const override;
	
	bool has_array_format() const override;
	ndarray_format array_format() const override;
};


}}

#endif
