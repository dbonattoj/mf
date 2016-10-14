#ifndef MF_OS_CONSOLE_H_
#define MF_OS_CONSOLE_H_

namespace mf {

enum class console_color {
	black,
	blue,
	red,
	green,
	magenta,
	cyan,
	yellow
};

void set_console_text_color(console_color);
void set_console_text_style(bool bold, bool underline);
void reset_console();


}

#endif
