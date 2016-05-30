#include "image_write.h"
#include "../opencv.h"

namespace mf {

void image_write(const image<rgb_color>& img, const std::string& filename) {
	cv::Mat output_img;
	cv::cvtColor(img.cv_mat(), output_img, CV_RGB2BGR);
	cv::imwrite(filename, output_img);
}


void image_write(const masked_image<rgb_color>& img, const std::string& filename) {
	cv::Mat output_img, holes;
	cv::cvtColor(img.cv_mat(), output_img, CV_RGB2BGRA);
	cv::bitwise_not(img.cv_mask_mat(), holes);
	output_img.setTo(cv::Scalar::all(0), holes);
	cv::imwrite(filename, output_img);
}


void image_write(const masked_image<rgb_color>& img, const std::string& filename, const rgb_color& background) {
	cv::Scalar cv_background(background.b, background.g, background.r);
	cv::Mat output_img, holes;
	cv::cvtColor(img.cv_mat(), output_img, CV_RGB2BGR);
	cv::bitwise_not(img.cv_mask_mat(), holes);
	output_img.setTo(cv_background, holes);
	cv::imwrite(filename, output_img);
}

}
