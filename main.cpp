/* Main Loop for cataclysm
 * Linux only I guess
 * But maybe not
 * Who knows
 */

#include <curses.h>
#include <ctime>
#include "game.h"
#include "color.h"

int main(int argc, char *argv[])
{
 srand(time(NULL));

 initscr();
 noecho();
 cbreak();
 keypad(stdscr, true);
 init_colors();
 curs_set(0);

 rand();
 game g;
 while (!g.do_turn());
 erase();
 endwin();
 #if !(defined _WIN32 || defined __WIN32__) || defined __CYGWIN__
 system("clear");
 #endif
 return 0;
}

