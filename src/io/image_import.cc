#include "image_import.h"
#include "../opencv.h"

namespace mf {

image<rgb_color> image_import(const std::string& filename) {
	cv::Mat img = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
	cv::cvtColor(img, img, CV_BGR2RGB);
	return image<rgb_color>(img);
}

}
