#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include "color.h"
#include <string.h>
#include <cstdarg>
#include <string>
#include <vector>
#include <map>

//      LINE_NESW  - X for on, O for off
#ifndef LINE_XOXO
    #if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
        // have to inline the pdcurses stuff here, since catacurses.h doesn't support it
        #ifdef CHTYPE_LONG
        # define ACS_PICK(w, n) ((chtype)w | A_ALTCHARSET)
        #else
        # define ACS_PICK(w, n) ((chtype)n)
        #endif

        #define ACS_ULCORNER  ACS_PICK('l', '+')
        #define ACS_LLCORNER  ACS_PICK('m', '+')
        #define ACS_URCORNER  ACS_PICK('k', '+')
        #define ACS_LRCORNER  ACS_PICK('j', '+')
        #define ACS_RTEE      ACS_PICK('u', '+')
        #define ACS_LTEE      ACS_PICK('t', '+')
        #define ACS_BTEE      ACS_PICK('v', '+')
        #define ACS_TTEE      ACS_PICK('w', '+')
        #define ACS_HLINE     ACS_PICK('q', '-')
        #define ACS_VLINE     ACS_PICK('x', '|')
        #define ACS_PLUS      ACS_PICK('n', '+')

    
        #define LINE_XOXO ACS_VLINE
        #define LINE_OXOX ACS_HLINE
        #define LINE_XXOO ACS_LLCORNER
        #define LINE_OXXO ACS_ULCORNER
        #define LINE_OOXX ACS_URCORNER
        #define LINE_XOOX ACS_LRCORNER
        #define LINE_XXXO ACS_LTEE
        #define LINE_XXOX ACS_BTEE
        #define LINE_XOXX ACS_RTEE
        #define LINE_OXXX ACS_TTEE
        #define LINE_XXXX ACS_PLUS
    #else
        #define LINE_XOXO 4194424
        #define LINE_OXOX 4194417
        #define LINE_XXOO 4194413
        #define LINE_OXXO 4194412
        #define LINE_OOXX 4194411
        #define LINE_XOOX 4194410
        #define LINE_XXXO 4194420
        #define LINE_XXOX 4194422
        #define LINE_XOXX 4194421
        #define LINE_OXXX 4194423
        #define LINE_XXXX 4194414
    #endif
#endif

void mvputch(int y, int x, nc_color FG, long ch);
void wputch(WINDOW* w, nc_color FG, long ch);
void mvwputch(WINDOW* w, int y, int x, nc_color FG, long ch);
void mvputch_inv(int y, int x, nc_color FG, long ch);
void mvwputch_inv(WINDOW *w, int y, int x, nc_color FG, long ch);
void mvputch_hi(int y, int x, nc_color FG, long ch);
void mvwputch_hi(WINDOW *w, int y, int x, nc_color FG, long ch);
void mvprintz(int y, int x, nc_color FG, const char *mes, ...);
void mvwprintz(WINDOW *w, int y, int x, nc_color FG, const char *mes, ...);
void printz(nc_color FG, const char *mes, ...);
void wprintz(WINDOW *w, nc_color FG, const char *mes, ...);
void draw_tabs(WINDOW *w, int active_tab, ...);

void debugmsg(const char *mes, ...);
bool query_yn(const char *mes, ...);
int  query_int(const char *mes, ...);
std::string string_input_popup(const char *mes, ...);
std::string string_input_popup(int max_length, const char *mes, ...);
char popup_getkey(const char *mes, ...);
int  menu_vec(const char *mes, std::vector<std::string> options);
int  menu(const char *mes, ...);
void popup_top(const char *mes, ...); // Displayed at the top of the screen
void popup(const char *mes, ...);
void full_screen_popup(const char *mes, ...);

nc_color hilite(nc_color c);
nc_color invert_color(nc_color c);
nc_color red_background(nc_color c);
nc_color rand_color();
char rand_char();


// Graphical tileset code
// ==================================================================

#include "catacurse.h"

// if we use SDL, load the header
#ifdef __TILESET
 #include <SDL/SDL.h>
 #undef main

extern "C" {
 PDCEX int pdc_sheight, pdc_swidth;
 int pdc_toggle_fullscreen();
}

#endif

// struct used to compare \0-terminated strings
struct ltstr {
    bool operator()(const char* s1, const char* s2) const {
        return strcmp(s1, s2) < 0;
    }
};

// this class will be used to make it possible to define multiple tilesets
// tilesets will map strings to positions within their file
class Tileset {
public:
 Tileset();
 int name_to_position(const char* name);

private:
 std::map<const char*,int,ltstr> string_to_position;
};

// In tileset mode, this can be used to adjust the window size.
void set_screen_size(int argc, char *argv[]);

extern Tileset* active_tileset;

// if in tileset mode, draws the sprite associated with symbol, otherwise returns false
bool draw_object(WINDOW* w, int x, int y, int symbol, bool highlight=false, bool alpha=false);


#endif
