#ifndef MF_OS_CONSOLE_H_
#define MF_OS_CONSOLE_H_

#include <iosfwd>

namespace mf {

enum class console_color {
	default_color,
	black,
	white,
	blue,
	red,
	green,
	magenta,
	cyan,
	yellow
};

void set_console_style(std::ostream&, console_color, bool bold = false);
void reset_console(std::ostream&);


}

#endif
