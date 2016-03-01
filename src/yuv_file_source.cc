#include "yuv_file_source.h"

namespace mf {

void yuv_file_source::process_frame_(const ndarray_view<2, ycbcr_color>&) {
	
}


yuv_file_source::yuv_file_source(const std::string& filename, std::size_t width, std::size_t height, int sampling) :
base(ndsize<2>(height, width), 0) {

}

}
