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

#ifndef MF_NDARRAY_WRAPPER_H_
#define MF_NDARRAY_WRAPPER_H_

#include "../ndarray_view.h"
#include "../ndcoord.h"
#include "../../os/memory.h"
#include <memory>
#include <utility>

namespace mf { namespace detail {

#define WRAP_VIEW_FUNCTION(__function__) \
	template<typename... Args> decltype(auto) __function__(Args&&... args) { \
		return view_.__function__(std::forward<Args>(args)...); \
	} \
	template<typename... Args> auto __function__(Args&&... args) const { \
		return cview().__function__(std::forward<Args>(args)...); \
	}
// const verion returns copies, from cview() (itself a temporary copy of view_)
// non-const version may return references (i.e. at() -> ref to elem)


/// Container for ndarray type, base class and wrapper around view.
/** Allocates and owns memory, and gives const correct access to data: `const` access to the `ndarray_wrapper` gives
 ** only `const` access to the data. */
template<typename View, typename Const_view, typename Allocator>
class ndarray_wrapper {
public:
	using view_type = View;
	using const_view_type = Const_view;
	using allocator_type = Allocator;
	
	using shape_type = typename view_type::shape_type;
	using strides_type = typename view_type::strides_type;
	

private:
	void allocate_(std::size_t allocate_size, std::size_t alignment);
	void deallocate_();

	Allocator allocator_; ///< Raw allocator used to allocate memory.
	std::size_t allocated_size_ = 0; ///< Allocated memory size, in bytes.
	void* allocated_buffer_ = nullptr; ///< Allocated memory.
	view_type view_; ///< View to allocated memory.
	
protected:
	/// \name Construction, assignment base
	///@{
	explicit ndarray_wrapper(const allocator_type& = allocator_type());
	
	template<typename... Arg>
	ndarray_wrapper(
		const shape_type& shape,
		const strides_type& strides,
		std::size_t allocate_size,
		std::size_t allocate_alignment,
		const Allocator& allocator,
		const Arg&... view_arguments
	);
	
	ndarray_wrapper(ndarray_wrapper&&);
	
	ndarray_wrapper& operator=(ndarray_wrapper&&);
	
	template<typename... Arg>
	void reset_(
		const shape_type& shape,
		const strides_type& strides,
		std::size_t allocate_size,
		std::size_t allocate_alignment,
		const Arg&... view_arguments
	);
	///@}

public:
	/// \name Construction
	///@{
	~ndarray_wrapper();

	bool is_null() const noexcept { return view_.is_null(); }
	explicit operator bool () const noexcept { return ! is_null(); }
	
	std::size_t allocated_byte_size() const { return allocated_size_; }
	const allocator_type& get_allocator() const { return allocator_; }
	///@}
	
	
	
	/// \name View access
	///@{
	const view_type& view() { return view_; }
	const_view_type view() const { return cview(); }
	const_view_type cview() const { return const_view_type(view_);  }
	
	operator view_type () noexcept { return view(); }
	operator const_view_type () const noexcept { return cview(); }
	///@}
	
	

	/// \name Attributes
	///@{
	WRAP_VIEW_FUNCTION(size);
	WRAP_VIEW_FUNCTION(start);
	WRAP_VIEW_FUNCTION(shape);
	WRAP_VIEW_FUNCTION(strides);
	WRAP_VIEW_FUNCTION(full_span);
	
	std::size_t allocated_size() const noexcept { return allocated_size_; }
	///@}
	
	
	
	/// \name Deep comparison
	///@{	
	template<typename Other_view> bool compare(const Other_view& vw) const { return cview().compare(vw); }
	template<typename Other_view> bool operator==(const Other_view& vw) const { return cview().compare(vw); }
	template<typename Other_view> bool operator!=(const Other_view& vw) const { return ! cview().compare(vw); }
	///@}
		
	
	
	/// \name Iteration
	///@{
	auto begin() noexcept { return view().begin(); }
	auto begin() const noexcept { return cview().begin(); }	
	auto cbegin() const noexcept { return cview().begin(); }	
	auto end() noexcept { return view().end(); }
	auto end() const noexcept { return cview().end(); }	
	auto cend() const noexcept { return cview().end(); }	
	///@}
	
	
	/// \name Indexing
	///@{
	WRAP_VIEW_FUNCTION(section);
	WRAP_VIEW_FUNCTION(slice);
	WRAP_VIEW_FUNCTION(operator[]);
	WRAP_VIEW_FUNCTION(operator());
	WRAP_VIEW_FUNCTION(at);
	///@}
};

#undef WRAP_VIEW_FUNCTION

}}

#include "ndarray_wrapper.tcc"

#endif
