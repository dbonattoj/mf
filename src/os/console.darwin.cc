#include "os.h"
#ifdef MF_OS_DARWIN

#include "console.h"
#include <map>
#include <iostream>

namespace mf {

void set_console_style(std::ostream& str, console_color text_color, bool bold, bool underline) {
	const static std::map<console_color, const char*> color_code = {
		{ console_color::default_color, "30" },
		{ console_color::black, "30" },
		{ console_color::white, "30" },
		{ console_color::blue, "34" },
		{ console_color::red, "31" },
		{ console_color::green, "32" },
		{ console_color::magenta, "35" },
		{ console_color::cyan, "36" },
		{ console_color::yellow, "33" }
	};

	if(bold) str << "\x1b[" << color_code.at(text_color) << ";1m" << std::flush;
	else str << "\x1b[" << color_code.at(text_color) << "m" << std::flush;
}


void reset_console(std::ostream& str) {
	str << "\x1b[0m" << std::flush;
}

}

#endif