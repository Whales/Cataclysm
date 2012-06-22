#if defined(_WIN32) || defined(WINDOWS)
	#include "catacurse.h"
#else
	#if defined(NCURSES)
		#include <ncurses/curses.h>
	#else
		#include <curses.h>
	#endif
#endif
