#include "../opencv.h"

namespace mf {

template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const masked_image<T>& img, const std::string& filename, T min_value, T max_value, const rgb_color& background)
{
	cv::Mat output_img;
	double alpha = 255.0 / (max_value - min_value);
	double beta = alpha * min_value;
	img.cv_mat().convertTo(output_img, CV_8U, alpha, beta);
	
	cv::Scalar cv_background(background.b, background.g, background.r);
	cv::Mat output_rgb_img, holes;
	cv::cvtColor(output_img, output_rgb_img, CV_GRAY2RGB);
	cv::bitwise_not(img.cv_mask_mat(), holes);
	output_rgb_img.setTo(cv_background, holes);
	cv::imwrite(filename, output_rgb_img);

}


template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const masked_image<T>& img, const std::string& filename, const rgb_color& background)
{
	cv::Point min_pos, max_pos;
	cv::minMaxLoc(img.cv_mat(), NULL, NULL, &min_pos, &max_pos, img.cv_mask_mat());
	image_write(img, filename, img.cv_mat()(min_pos), img.cv_mat()(max_pos), background);
}


template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const masked_image<T>& img, const std::string& filename, T min_value, T max_value)
{
	cv::Mat output_img;
	double alpha = 255.0 / (max_value - min_value);
	double beta = alpha * min_value;
	img.cv_mat().convertTo(output_img, CV_8U, alpha, beta);
	
	cv::Mat output_rgba_img, holes;
	cv::cvtColor(output_img, output_rgba_img, CV_GRAY2RGBA);
	cv::bitwise_not(img.cv_mask_mat(), holes);
	output_rgba_img.setTo(cv::Scalar::all(0), holes);
	cv::imwrite(filename, output_rgba_img);
}


template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const masked_image<T>& img, const std::string& filename)
{
	cv::Point min_pos, max_pos;
	cv::minMaxLoc(img.cv_mat(), NULL, NULL, &min_pos, &max_pos, img.cv_mask_mat());
	image_write(img, filename, img.cv_mat()(min_pos), img.cv_mat()(max_pos));
}


template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const image<T>& img, const std::string& filename, T min_value, T max_value)
{
	cv::Mat output_img;
	double alpha = 255.0 / (max_value - min_value);
	double beta = alpha * min_value;
	img.cv_mat().convertTo(output_img, CV_8U, alpha, beta);
	cv::imwrite(filename, output_img);
}


template<typename T>
std::enable_if_t<std::is_arithmetic<T>::value> image_write
	(const image<T>& img, const std::string& filename)
{
	cv::Point min_pos, max_pos;
	cv::minMaxLoc(img.cv_mat(), NULL, NULL, &min_pos, &max_pos);
	image_write(img, filename, img.cv_mat()(min_pos), img.cv_mat()(max_pos));
}



}
