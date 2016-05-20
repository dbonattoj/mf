---
title: N-d arrays
---

# N-d arrays
The `ndarray` represents a multi-dimensional homogeneous array. Its items ("elements") must be of an _elem_ type, i.e.
a type for which `elem_traits` is specialized. See the page on [elem types]({{ site.baseurl }}/manual/elem.html) for
more on this. All data processed by the [flow graph]({{ site.baseurl }}/manual/graph.html) is of this type.

`ndarray_view<Dim, Elem>` is a _view_ type which provides access to a `Dim`-dimensional array of elements of type `Elem`
in an existing buffer. `ndarray<Dim, Elem>` is the equivalent _container_ type, which allocates and own the buffer
itself.

## View
An `ndarray_view<Dim, Elem>` is a mapping between _coordinates_, _indices_ and _memory addresses_. Coordinates are
vectors `ndptrdiff<Dim>`. Indices are positive `std::ptrdiff_t` integers. Memory addresses are pointers `Elem*` of the
actual elements in the memory buffer.

Coordinates map to memory addresses, but not the other way around. Indices map to coordinates in both ways. For this
the functions `coordinates_to_pointer()`, `index_to_coordinates()`, `coordinates_to_index()` exist.

The `ndarray_view<Dim, Elem>` is defined using its _shape_, _strides_, and _start_. The shape `ndsize<Dim>` defines the
length of each axis. For example on a two-dimensional `ndarray_view<2, rgb_color>`, it is the height and width of the
image. The _start_ is the pointer `Elem*` to the element at coordinates `(0,0,...,0)`.

### Strides
The strides define the multi-dimensional memory layout: `strides[i]` is the number of _bytes_ that a pointer must
advance when the `i`-th coordinate is incremented. The default layout is _row-major_: The last stride is `sizeof(Elem)`,
i.e. elements in the last dimension are sequential in memory. There can also be _padding_ between these elements,
then the last stride is `sizeof(Elem) + padding`. The preceding strides are `stride[i] = shape[i+1] * stride[i+1]`.
That is, the elements at coordinates `(k,shape[1]-1)` and `(k+1,0)`, are sequential in memory, with the same `padding`.
The `ndarray_view` has _default strides down to dimension `i` with padding `padding`_ if this remains true until and
including `stride[i]`. It has fully default strides if it has default strides down to dimension 0.

Strides can also be negative. In that case increasing the coordinate decreases the memory addresses. They can also be
set such that individual elements, or groups of elements, overlap. The user of the `ndarray_view` needs to make sure
that the constraints of the `Elem` are respected. For instance, the _strides_ and _start_ must respect the alignment
requirement `alignof(Elem)` of the element type.

Using the strides, _coordinates_ are mapped to _memory addresses_. Memory addresses cannot be mapped back to
coordinates. Depending on the strides setting, one memory address can correspond to none, or to multiple coordinates.

### Construction
An `ndarray_view` is constructed by explicitly specifying the start, shape and strides:

    ndarray_view<Dim, Elem>::ndarray_view(Elem* start, ndsize<Dim> shape, ndptrdiff<Dim> strides)
    ndarray_view<Dim, Elem>::ndarray_view(const ndarray_view<Dim, Elem>&)

Similarly to references in C++, copy-constructing a `ndarray_view` does not copy its elements. *However assigning and
comparing `ndarray_view`s does not change the `ndarray_view`, but instead assigns/compares its elements.* This behavior
renders the interface more practical in many cases.

To reassign the start, shape and strides, use the function `reset()`:

   void ndarray_view<Dim, Elem>::reset(Elem* start, ndsize<Dim> shape, ndptrdiff<Dim> strides)
   void ndarray_view<Dim, Elem>::reset(const ndarray_view<Dim, Elem>&)

To compare the start, shape and strides, use the function `same()`:

   bool same(const ndarray_view<Dim, Elem>&, const ndarray_view<Dim, Elem>&)

The copy-constructor allows `ndarray_view<Dim, const Elem>` to be constructed from `ndarray_view<Dim, Elem>`, but not
the opposite.

#### Null view
A special _null_ `ndarray_view<Dim, Elem>` is constructed with either

    ndarray_view<Dim, Elem>::ndarray_view()
    ndarray_view<Dim, Elem>::null()
    void ndarray_view<Dim, Elem>::reset()

All _null_ `ndarray_view<Dim, Elem>` instances compare as equal using `same()`. A non-null and a null
`ndarray_view<Dim, Elem>` compare as non-equal. `is_null()`, or the explicit bool conversion operator test if the
view is null. The _null_ `ndarray_view<Dim, Elem>` is equivalent to a view with _start_ `nullptr`. It is used to mark
error conditions.


### Element access
Elements are accessed by coordinates using `at()`, or by chaining the subscript `operator[]`. For example, the element
at coordinates `(1,3,0)` can be accessed using

    vw.at(make_ndsize(1, 3, 0))
    vw[1][3][0]

