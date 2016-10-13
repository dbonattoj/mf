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
/** Derives from \ref opaque_multi_ndarray_format. Frame contains for each channel one \ref ndarray_format part.
 ** Additionally frame contains \ref node_frame_satellite object, which gets constructed and destructed with the frame.
 ** Also allows direct access to its first array_format, so that from_opaque() can be used directly without prior
 ** use of extract_part(), on single-part frames. */
class node_frame_format : public opaque_multi_ndarray_format {
	using base = opaque_multi_ndarray_format;
	
	/*
	Memory layout of frame:
	---  <-- ndarray for with part, at offset 0
	XXX      
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
	
	bool compare(const opaque_format& frm) const override;
		
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
	std::ptrdiff_t array_offset() const override;
};


/// Same frame format as \ref node_frame_format, with one selected channel.
/** Describes the same frame format, but one of the multiple channels is _selected_, and becomes the ndarray associated
 ** with the frame. The ndarray for this channel is accessed using from_opaque().
 ** Format is not used for storage in buffers, but is used for temporary views, e.g. those read from the multiplex node
 ** output. */
class node_selected_channel_frame_format : public node_frame_format {
	using base = node_frame_format;

private:
	std::ptrdiff_t selected_channel_;

public:
	node_selected_channel_frame_format() : base(), selected_channel_(-1) { }
	node_selected_channel_frame_format(const base& frm, std::ptrdiff_t sel_channel);
	
	bool compare(const opaque_format& frm) const override;

	bool has_parts() const override { return false; }
	std::size_t parts_count() const override { throw std::logic_error("not implemented");; }
	extracted_part extract_part(std::ptrdiff_t index) const override { throw std::logic_error("not implemented"); }

	bool has_array_format() const override;
	ndarray_format array_format() const override;
	std::ptrdiff_t array_offset() const override;
};


}}

#endif
