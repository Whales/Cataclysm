/* Main Loop for cataclysm
 * Linux only I guess
 * But maybe not
 * Who knows
 */

#include <curses.h>
#include <ctime>
#include "game.h"
#include "color.h"
#include <signal.h>
#include <sys/wait.h>


void print_trace(int signum) {
    int child_pid = fork();
    if (!child_pid) {
    	sleep(1);
        execlp("gdb", "gdb", "--batch","-ex","set logging on", "-ex", "bt full", "-c","core","-f","cataclysm", NULL);
        abort(); /* If gdb failed to start */
    } else {
    	signal(signum, SIG_DFL);
    	kill(getpid(), signum);

    }
}


void handler(int sig) {
  erase(); // Clear screen
  endwin(); // End ncurses
	print_trace(sig);
}


int main(int argc, char *argv[])
{
 struct rlimit limit;

 /* Set the file size resource limit.
  * Probably a stupid number just fiddled with it
  * until it worked */
 limit.rlim_cur = 6813952L;
 limit.rlim_max = 6813952L;
 setrlimit(RLIMIT_CORE, &limit);
 setrlimit(RLIMIT_FSIZE, &limit);
 setrlimit(RLIMIT_STACK, &limit);

 signal(SIGSEGV, handler);
 /* Don't know if those should be catched too?
 signal(SIGILL, handler);
 signal(SIGFPE, handler);
*/
 srand(time(NULL));

// ncurses stuff
 initscr(); // Initialize ncurses
 noecho();  // Don't echo keypresses
 cbreak();  // C-style breaks (e.g. ^C to SIGINT)
 keypad(stdscr, true); // Numpad is numbers
 init_colors(); // See color.cpp
 curs_set(0); // Invisible cursor

 rand();  // For some reason a call to rand() seems to be necessary to avoid
          // repetion.
 bool quit_game = false;
 game *g;
 do {
  g = new game();
  while (!g->do_turn());
  if (g->game_quit())
   quit_game = true;
  delete g;
 } while (!quit_game);
 erase(); // Clear screen
 endwin(); // End ncurses
 system("clear"); // Tell the terminal to clear itself
 return 0;
}

