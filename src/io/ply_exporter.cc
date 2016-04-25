#include "ply_exporter.h"
#include <iomanip>
#include <stdexcept>
#include <cassert>


namespace mf {
	
namespace {
	constexpr std::size_t vertex_count_string_length_ = 15;
}

ply_exporter::ply_exporter(const std::string& filename, bool full, bool ascii, line_delimitor ld) :
	file_(filename, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary),
	line_delimitor_(ld),
	full_(full),
	ascii_(ascii)
{
	write_line_("ply");
	if(ascii_) write_line_("format ascii 1.0");
	else if(host_is_little_endian) write_line_("format binary_little_endian 1.0");
	else write_line_("format binary_big_endian 1.0");
	
	write_line_("comment PLY file generated using pcf::ply_exporter");
	
	file_ << "element vertex ";
	file_ << std::flush;
	vertex_count_string_position_ = file_.tellp();
	file_ << std::setfill(' ') << std::left << std::setw(vertex_count_string_length_) << 0;
	write_line_("");
	
	std::string scalar_type_name;
	if(sizeof(Eigen_scalar) == 4) scalar_type_name = "float";
	else if(sizeof(Eigen_scalar) == 8) scalar_type_name = "double";
	else throw std::logic_error("unsupported Eigen_scalar type");
	
	write_line_("property " + scalar_type_name + " x");
	write_line_("property " + scalar_type_name + " y");
	write_line_("property " + scalar_type_name + " z");
	if(full_) {
		write_line_("property " + scalar_type_name + " nx");
		write_line_("property " + scalar_type_name + " ny");
		write_line_("property " + scalar_type_name + " nz");
		write_line_("property uchar red");
		write_line_("property uchar green");
		write_line_("property uchar blue");
		write_line_("property " + scalar_type_name + " weight");
	}
	
	write_line_("end_header");
}


ply_exporter::~ply_exporter() {
	close();
}


void ply_exporter::close() {
	// write vertex count to file
	file_.seekp(vertex_count_string_position_);
	file_ << std::setfill(' ') << std::left << std::setw(vertex_count_string_length_) << count_;

	// close
	file_.close();
}


void ply_exporter::write(const ndarray_view<1, const point_xyz>& arr) {
	count_ += arr.size();
	if(full_)
		for(const point_xyz& pt : arr) {
			point_full full_pt(pt);
			if(ascii_) write_ascii_(full_pt);
			else write_binary_(full_pt);
		}
	else
		for(const point_xyz& pt : arr) {
			if(ascii_) write_ascii_(pt);
			else write_binary_(pt);
		}
}


void ply_exporter::write(const ndarray_view<1, const point_xyzrgb>& arr) {
	count_ += arr.size();
	if(full_)
		for(const point_xyzrgb& pt : arr) {
			point_full full_pt(pt);
			if(ascii_) write_ascii_(full_pt);
			else write_binary_(full_pt);
		}
	else
		for(const point_xyzrgb& pt : arr) {
			if(ascii_) write_ascii_(get<point_xyz>(pt));
			else write_binary_(get<point_xyz>(pt));
		}
}


void ply_exporter::write(const ndarray_view<1, const point_full>& arr) {
	count_ += arr.size();
	if(full_)
		for(const point_full& pt : arr) {
			if(ascii_) write_ascii_(pt);
			else write_binary_(pt);
		}
	else
		for(const point_full& pt : arr) {
			if(ascii_) write_ascii_(get<point_xyz>(pt));
			else write_binary_(get<point_xyz>(pt));
		}
}


void ply_exporter::write_line_(const std::string& ln) {
	write_line(file_, ln, line_delimitor_);
}


void ply_exporter::write_binary_(const point_xyz& p) {
	file_.write(reinterpret_cast<const char*>( p.position().data() ), 3 * sizeof(Eigen_scalar));
}


void ply_exporter::write_ascii_(const point_xyz& p) {
	Eigen_vec3 position = p.position();	
	file_ << position[0] << ' ' << position[1] << ' ' << position[2];
	end_line(file_, line_delimitor_);
}


void ply_exporter::write_binary_(const point_full& p) {
	Eigen_vec3 position = p.position();
	Eigen_vec3 normal = p.normal();
	Eigen_scalar weight = p.weight();
	rgb_color col = p.color();
	
	file_.write(reinterpret_cast<const char*>( position.data() ), 3 * sizeof(Eigen_scalar));
	file_.write(reinterpret_cast<const char*>( normal.data() ), 3 * sizeof(Eigen_scalar));
	file_.write(reinterpret_cast<const char*>( &col ), 3);
	file_.write(reinterpret_cast<const char*>( &weight ), sizeof(Eigen_scalar));
}


void ply_exporter::write_ascii_(const point_full& p) {
	Eigen_vec3 position = p.position();
	Eigen_vec3 normal = p.normal();
	Eigen_scalar weight = p.weight();
	rgb_color col = p.color();

	file_ << position[0] << ' ' << position[1] << ' ' << position[2]
		<< ' ' << normal[0] << ' ' << normal[1] << ' ' << normal[2]
		<< ' ' << (unsigned)col.r << ' ' << (unsigned)col.g << ' ' << (unsigned)col.b
		<< ' ' << weight;
	end_line(file_, line_delimitor_);
}



}
