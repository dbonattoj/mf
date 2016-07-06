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
