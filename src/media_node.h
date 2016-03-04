#ifndef MF_MEDIA_NODE_H_
#define MF_MEDIA_NODE_H_

#include <vector>
#include <memory>
#include "ndcoord.h"
#include "ndarray_ring.h"

namespace mf {

class media_node_base {
protected:
	int time_;
};

class media_node_input_base {

};


struct media_node_time_window {
	std::size_t past_frames;
	std::size_t future_frames;
};

template<std::size_t Dim, typename T>
class media_node_input : public media_node_input_base {
private:
			
};


template<std::size_t Dim, typename T>
class media_node : public media_node_base {
private:
	std::vector<std::unique_ptr<media_node_input_base>> inputs_;
	ndarray_ring<Dim + 1, T> output_buffer_;
	int time_;
	
protected:
	template<std::size_t Dim2, typename T2>
	media_node_input<Dim2, T2>& add_input_(const ndsize<Dim2>& shape, media_node_time_window time_window = {0, 0});
	
	void initialize(std::size_t buffer_size);
	
	
};



}

#include "media_node.tcc"

#endif
