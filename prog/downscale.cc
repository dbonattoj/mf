#include <mf/io/yuv_importer.h>
#include <mf/io/video_exporter.h>
#include <mf/color.h>
#include <mf/flow/graph.h>
#include <mf/flow/sync_node.h>
#include <mf/flow/async_node.h>
#include <mf/filter/importer.h>
#include <mf/filter/exporter.h>
#include <mf/filter/color_converter.h>

#include "support/input_data.h"

#include "filter/downscale.h"

#include <string>

using namespace mf;

std::string out = "output/downscale_video.avi";


int main(int argc, const char* argv[]) {
	if(argc >= 2) out = argv[1];

	input_data data = poznan_blocks();
	ndsize<2> scaled_shape(data.image_height() / 3, data.image_width() / 3);
		
	flow::graph graph;
	
	auto& im_source = graph.add_filter<flow::importer_filter<yuv_importer>>(
		vis.image_yuv_file, shape, data.yuv_sampling
	);
		auto& im_converter = graph.add_filter<flow::color_converter_filter<ycbcr_color, rgb_color>>();
		im_converter.input.connect(im_source.output);
		

	// Result filter
	auto& result_filter = graph.add_filter<result_improve_filter>();
	result_filter.input.connect(blender.output);

	// Converter+sink for output image
	auto& sink = graph.add_sink_filter<flow::exporter_filter<video_exporter>>(out, shape);
	auto& sink_converter = graph.add_filter<flow::color_converter_filter<masked_elem<rgb_color>, rgb_color>>();
	sink_converter.input.connect(result_filter.output);
	sink.input.connect(sink_converter.output);

	graph.setup();
	graph.run();
}
