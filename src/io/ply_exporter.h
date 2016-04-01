#include <fstream>
#include <string>
#include "../point_cloud/point.h"
#include "../utility/io.h"
#include "../ndarray/ndarray_view.h"

namespace mf {

/// Exports point cloud into PLY file.
/** Can generate binary or ASCII format, and either only XYZ coordinates or with color, weight and normals. */
class ply_exporter {
private:
	std::ofstream file_;
	std::ofstream::pos_type vertex_count_string_position_;
	std::size_t count_ = 0;
	
	const line_delimitor line_delimitor_;
	bool full_;
	bool ascii_;
		
	void write_line_(const std::string& ln);
	
	void write_binary_(const point_xyz&);
	void write_ascii_(const point_xyz&);
	void write_binary_(const point_full&);
	void write_ascii_(const point_full&);


public:
	explicit ply_exporter(
		const std::string& filename,
		bool full = true,
		bool ascii = false,
		line_delimitor ld = line_delimitor::LF
	);
	~ply_exporter();
	
	void write(const ndarray_view<1, const point_xyz>&);
	void write(const ndarray_view<1, const point_xyzrgb>&);
	void write(const ndarray_view<1, const point_full>&);
	void close();
};

}
