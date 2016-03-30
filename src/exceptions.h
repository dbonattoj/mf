#ifndef MF_EXCEPTIONS_H_
#define MF_EXCEPTIONS_H_

#include <stdexcept>
#include <string>

#define MF_DEFINE_EXCEPTION_(name, base) \
	class name : public base { \
	public: \
		explicit name(const std::string& what) : base(what) { } \
		explicit name(const char* what) : base(what) { } \
	};


namespace mf {

MF_DEFINE_EXCEPTION_(sequencing_error, std::runtime_error);
MF_DEFINE_EXCEPTION_(ply_importer_error, std::runtime_error);

}

#endif
