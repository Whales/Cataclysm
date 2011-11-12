#ifndef _OUTPUT_CPP_
#define _OUTPUT_CPP_

#include <curses.h>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstring>
#include <stdlib.h>
#include <sstream>
#include "color.h"
#include "output.h"
#include "rng.h"
#include "keypress.h"

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

nc_color hilite(nc_color c)
{
 switch (c) {
  case c_white:		return h_white;
  case c_ltgray:	return h_ltgray;
  case c_dkgray:	return h_dkgray;
  case c_red:		return h_red;
  case c_green:		return h_green;
  case c_blue:		return h_blue;
  case c_cyan:		return h_cyan;
  case c_magenta:	return h_magenta;
  case c_brown:		return h_brown;
  case c_ltred:		return h_ltred;
  case c_ltgreen:	return h_ltgreen;
  case c_ltblue:	return h_ltblue;
  case c_ltcyan:	return h_ltcyan;
  case c_pink:		return h_pink;
  case c_yellow:	return h_yellow;
 }
 return h_white;
}

nc_color invert_color(nc_color c)
{
 switch (c) {
  case c_white:   return i_white;
  case c_ltgray:  return i_ltgray;
  case c_dkgray:  return i_dkgray;
  case c_red:     return i_red;
  case c_green:   return i_green;
  case c_blue:    return i_blue;
  case c_cyan:    return i_cyan;
  case c_magenta: return i_magenta;
  case c_brown:   return i_brown;
  case c_yellow:  return i_yellow;
  case c_ltred:   return i_ltred;
  case c_ltgreen: return i_ltgreen;
  case c_ltblue:  return i_ltblue;
  case c_ltcyan:  return i_ltcyan;
  case c_pink:    return i_pink;
 }
}

nc_color red_background(nc_color c)
{
 switch (c) {
  case c_white:		return c_white_red;
  case c_ltgray:	return c_ltgray_red;
  case c_dkgray:	return c_dkgray_red;
  case c_red:		return c_red_red;
  case c_green:		return c_green_red;
  case c_blue:		return c_blue_red;
  case c_cyan:		return c_cyan_red;
  case c_magenta:	return c_magenta_red;
  case c_brown:		return c_brown_red;
  case c_ltred:		return c_ltred_red;
  case c_ltgreen:	return c_ltgreen_red;
  case c_ltblue:	return c_ltblue_red;
  case c_ltcyan:	return c_ltcyan_red;
  case c_pink:		return c_pink_red;
  case c_yellow:	return c_yellow_red;
 }
 return c_white_red;
}

nc_color rand_color()
{
 switch (rng(0, 9)) {
  case 0:	return c_white;
  case 1:	return c_ltgray;
  case 2:	return c_green;
  case 3:	return c_red;
  case 4:	return c_yellow;
  case 5:	return c_blue;
  case 6:	return c_ltblue;
  case 7:	return c_pink;
  case 8:	return c_magenta;
  case 9:	return c_brown;
 }
 return c_dkgray;
}


void mvputch(int y, int x, nc_color FG, long ch)
{
 attron(FG);
 mvaddch(y, x, ch);
 attroff(FG);
}

void wputch(WINDOW* w, nc_color FG, long ch)
{
 wattron(w, FG);
 waddch(w, ch);
 wattroff(w, FG);
}

void mvwputch(WINDOW *w, int y, int x, nc_color FG, long ch)
{
 wattron(w, FG);
 mvwaddch(w, y, x, ch);
 wattroff(w, FG);
}

void mvputch_inv(int y, int x, nc_color FG, long ch)
{
 nc_color HC = invert_color(FG);
 attron(HC);
 mvaddch(y, x, ch);
 attroff(HC);
}

void mvwputch_inv(WINDOW* w, int y, int x, nc_color FG, long ch)
{
 nc_color HC = invert_color(FG);
 wattron(w, HC);
 mvwaddch(w, y, x, ch);
 wattroff(w, HC);
}

void mvputch_hi(int y, int x, nc_color FG, long ch)
{
 nc_color HC = hilite(FG);
 attron(HC);
 mvaddch(y, x, ch);
 attroff(HC);
}

void mvwputch_hi(WINDOW* w, int y, int x, nc_color FG, long ch)
{
 nc_color HC = hilite(FG);
 wattron(w, HC);
 mvwaddch(w, y, x, ch);
 wattroff(w, HC);
}

void mvprintz(int y, int x, nc_color FG, const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[6000];
 vsprintf(buff, mes, ap);
 va_end(ap);
 attron(FG);
 mvprintw(y, x, buff);
 attroff(FG);
}

void mvwprintz(WINDOW* w, int y, int x, nc_color FG, const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
// char buff[4096];
// vsprintf(buff, mes, ap);
 char buff[6000];
 vsprintf(buff, mes, ap);
 wattron(w, FG);
// wmove(w, y, x);
 mvwprintw(w, y, x, buff);
 wattroff(w, FG);
 va_end(ap);
}

void printz(nc_color FG, const char *mes, ...)
{
 va_list ap;
 va_start(ap,mes);
 char buff[6000];
 vsprintf(buff, mes, ap);
 va_end(ap);
 attron(FG);
 printw(buff);
 attroff(FG);
}

void wprintz(WINDOW *w, nc_color FG, const char *mes, ...)
{
 va_list ap;
 va_start(ap,mes);
 char buff[6000];
 vsprintf(buff, mes, ap);
 va_end(ap);
 wattron(w, FG);
 wprintw(w, buff);
 wattroff(w, FG);
}

void draw_tabs(WINDOW *w, int active_tab, ...)
{
 int win_width;
 win_width = getmaxx(w);
 std::vector<std::string> labels;
 va_list ap;
 va_start(ap, active_tab);
 char *tmp;
 while (tmp = (char *)va_arg(ap, int))
  labels.push_back((std::string)(tmp));
 va_end(ap);

// Draw the line under the tabs
 for (int x = 0; x < win_width; x++)
  mvwputch(w, 2, x, c_white, LINE_OXOX);

 int total_width = 0;
 for (int i = 0; i < labels.size(); i++)
  total_width += labels[i].length() + 6; // "< |four| >"

 if (total_width > win_width) {
  //debugmsg("draw_tabs not given enough space! %s", labels[0]);
  return;
 }

// Extra "buffer" space per each side of each tab
 double buffer_extra = (win_width - total_width) / (labels.size() * 2);
 int buffer = int(buffer_extra);
// Set buffer_extra to (0, 1); the "extra" whitespace that builds up
 buffer_extra = buffer_extra - buffer;
 int xpos = 0;
 double savings = 0;

 for (int i = 0; i < labels.size(); i++) {
  int length = labels[i].length();
  xpos += buffer + 2;
  savings += buffer_extra;
  if (savings > 1) {
   savings--;
   xpos++;
  }
  mvwputch(w, 0, xpos, c_white, LINE_OXXO);
  mvwputch(w, 1, xpos, c_white, LINE_XOXO);
  mvwputch(w, 0, xpos + length + 1, c_white, LINE_OOXX);
  mvwputch(w, 1, xpos + length + 1, c_white, LINE_XOXO);
  if (i == active_tab) {
   mvwputch(w, 1, xpos - 2, h_white, '<');
   mvwputch(w, 1, xpos + length + 3, h_white, '>');
   mvwputch(w, 2, xpos, c_white, LINE_XOOX);
   mvwputch(w, 2, xpos + length + 1, c_white, LINE_XXOO);
   mvwprintz(w, 1, xpos + 1, h_white, labels[i].c_str());
   for (int x = xpos + 1; x <= xpos + length; x++) {
    mvwputch(w, 0, x, c_white, LINE_OXOX);
    mvwputch(w, 2, x, c_black, 'x');
   }
  } else {
   mvwputch(w, 2, xpos, c_white, LINE_XXOX);
   mvwputch(w, 2, xpos + length + 1, c_white, LINE_XXOX);
   mvwprintz(w, 1, xpos + 1, c_white, labels[i].c_str());
   for (int x = xpos + 1; x <= xpos + length; x++)
    mvwputch(w, 0, x, c_white, LINE_OXOX);
  }
  xpos += length + 1 + buffer;
 }
}

void debugmsg(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[1024];
 vsprintf(buff, mes, ap);
 va_end(ap);
 attron(c_red);
 mvprintw(0, 0, "DEBUG: %s                \n  Press spacebar...", buff);
 while(getch() != ' ');
 attroff(c_red);
}

bool query_yn(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[1024];
 vsprintf(buff, mes, ap);
 va_end(ap);
 int win_width = strlen(buff) + 10;
 WINDOW* w = newwin(3, win_width, 11, 0);
 wborder(w, LINE_XOXO, LINE_XOXO, LINE_OXOX, LINE_OXOX,
            LINE_OXXO, LINE_OOXX, LINE_XXOO, LINE_XOOX );
 mvwprintz(w, 1, 1, c_ltred, "%s (Y/N)", buff);
 wrefresh(w);
 char ch;
 do
  ch = getch();
 while (ch != 'Y' && ch != 'N');
 werase(w);
 wrefresh(w);
 delwin(w);
 refresh();
 if (ch == 'Y')
  return true;
 return false;
}

int query_int(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[1024];
 vsprintf(buff, mes, ap);
 va_end(ap);
 int win_width = strlen(buff) + 10;
 WINDOW* w = newwin(3, win_width, 11, 0);
 wborder(w, LINE_XOXO, LINE_XOXO, LINE_OXOX, LINE_OXOX,
            LINE_OXXO, LINE_OOXX, LINE_XXOO, LINE_XOOX );
 mvwprintz(w, 1, 1, c_ltred, "%s (0-9)", buff);
 wrefresh(w);

 int temp;
 do
  temp = getch();
 while ((temp-48)<0 || (temp-48)>9);
 werase(w);
 wrefresh(w);
 delwin(w);
 refresh();

 return temp-48;
}

std::string string_input_popup(const char *mes, ...)
{
 std::string ret;
 va_list ap;
 va_start(ap, mes);
 char buff[1024];
 vsprintf(buff, mes, ap);
 va_end(ap);
 int startx = strlen(buff) + 2;
 WINDOW* w = newwin(3, 80, 11, 0);
 wborder(w, LINE_XOXO, LINE_XOXO, LINE_OXOX, LINE_OXOX,
            LINE_OXXO, LINE_OOXX, LINE_XXOO, LINE_XOOX );
 mvwprintz(w, 1, 1, c_ltred, "%s", buff);
 for (int i = startx + 1; i < 79; i++)
  mvwputch(w, 1, i, c_ltgray, '_');
 int posx = startx;
 mvwputch(w, 1, posx, h_ltgray, '_');
 do {
  wrefresh(w);
  long ch = getch();
  if (ch == 27) {	// Escape
   werase(w);
   wrefresh(w);
   delwin(w);
   refresh();
   return "";
  } else if (ch == '\n') {
   werase(w);
   wrefresh(w);
   delwin(w);
   refresh();
   return ret;
  } else if ((ch == KEY_BACKSPACE || ch == 127) && posx > startx) {
   ret = ret.substr(0, ret.size() - 1);
   mvwputch(w, 1, posx, c_ltgray, '_');
   posx--;
   mvwputch(w, 1, posx, h_ltgray, '_');
  } else {
   ret += ch;
   mvwputch(w, 1, posx, c_magenta, ch);
   posx++;
   mvwputch(w, 1, posx, h_ltgray, '_');
  }
 } while (true);
}

std::string string_input_popup(int max_length, const char *mes, ...)
{
 std::string ret;
 va_list ap;
 va_start(ap, mes);
 char buff[1024];
 vsprintf(buff, mes, ap);
 va_end(ap);
 int startx = strlen(buff) + 2;
 WINDOW* w = newwin(3, 80, 11, 0);
 wborder(w, LINE_XOXO, LINE_XOXO, LINE_OXOX, LINE_OXOX,
            LINE_OXXO, LINE_OOXX, LINE_XXOO, LINE_XOOX );
 mvwprintz(w, 1, 1, c_ltred, "%s", buff);
 for (int i = startx + 1; i < 79; i++)
  mvwputch(w, 1, i, c_ltgray, '_');
 int posx = startx;
 mvwputch(w, 1, posx, h_ltgray, '_');
 do {
  wrefresh(w);
  long ch = getch();
  if (ch == 27) {	// Escape
   werase(w);
   wrefresh(w);
   delwin(w);
   refresh();
   return "";
  } else if (ch == '\n') {
   werase(w);
   wrefresh(w);
   delwin(w);
   refresh();
   return ret;
  } else if ((ch == KEY_BACKSPACE || ch == 127) && posx > startx) {
   ret = ret.substr(0, ret.size() - 1);
   mvwputch(w, 1, posx, c_ltgray, '_');
   posx--;
   mvwputch(w, 1, posx, h_ltgray, '_');
  } else if(ret.size() < max_length){
   ret += ch;
   mvwputch(w, 1, posx, c_magenta, ch);
   posx++;
   mvwputch(w, 1, posx, h_ltgray, '_');
  }
 } while (true);
}

char popup_getkey(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[8192];
 vsprintf(buff, mes, ap);
 va_end(ap);
 std::string tmp = buff;
 int width = 0;
 int height = 2;
 size_t pos = tmp.find_first_of('\n');
 while (pos != std::string::npos) {
  height++;
  if (pos > width)
   width = pos;
  tmp = tmp.substr(pos + 1);
  pos = tmp.find_first_of('\n');
 }
 if (width == 0 || tmp.length() > width)
  width = tmp.length();
 width += 2;
 if (height > 25)
  height = 25;
 WINDOW* w = newwin(height + 1, width, int((25 - height) / 2),
                    int((80 - width) / 2));
 wborder(w, LINE_XOXO, LINE_XOXO, LINE_OXOX, LINE_OXOX,
            LINE_OXXO, LINE_OOXX, LINE_XXOO, LINE_XOOX );
 tmp = buff;
 pos = tmp.find_first_of('\n');
 int line_num = 0;
 while (pos != std::string::npos) {
  std::string line = tmp.substr(0, pos);
  line_num++;
  mvwprintz(w, line_num, 1, c_white, line.c_str());
  tmp = tmp.substr(pos + 1);
  pos = tmp.find_first_of('\n');
 }
 line_num++;
 mvwprintz(w, line_num, 1, c_white, tmp.c_str());

 wrefresh(w);
 char ch = getch();;
 werase(w);
 wrefresh(w);
 delwin(w);
 refresh();
 return ch;
}

int menu_vec(const char *mes, std::vector<std::string> options)
{
 if (options.size() == 0) {
  debugmsg("0-length menu (\"%s\")", mes);
  return -1;
 }
 std::string title = mes;
 int height = 3 + options.size(), width = title.length() + 2;
 for (int i = 0; i < options.size(); i++) {
  if (options[i].length() + 6 > width)
   width = options[i].length() + 6;
 }
 WINDOW* w = newwin(height, width, 6, 10);
 wattron(w, c_white);
 wborder(w, LINE_XOXO, LINE_XOXO, LINE_OXOX, LINE_OXOX,
            LINE_OXXO, LINE_OOXX, LINE_XXOO, LINE_XOOX );
 mvwprintw(w, 1, 1, title.c_str());
 for (int i = 0; i < options.size(); i++)
  mvwprintw(w, i + 2, 1, "%d: %s", i + 1, options[i].c_str());
 long ch;
 wrefresh(w);
 do
  ch = getch();
 while (ch < '1' || ch >= '1' + options.size());
 werase(w);
 wrefresh(w);
 delwin(w);
 return (ch - '1' + 1);
}

int menu(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char* tmp;
 std::vector<std::string> options;
 bool done = false;
 while (!done) {
  tmp = va_arg(ap, char*);
  if (tmp != NULL) {
   std::string strtmp = tmp;
   options.push_back(strtmp);
  } else
   done = true;
 }
 return (menu_vec(mes, options));
}

void popup_top(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[1024];
 vsprintf(buff, mes, ap);
 va_end(ap);
 std::string tmp = buff;
 int width = 0;
 int height = 2;
 size_t pos = tmp.find_first_of('\n');
 while (pos != std::string::npos) {
  height++;
  if (pos > width)
   width = pos;
  tmp = tmp.substr(pos + 1);
  pos = tmp.find_first_of('\n');
 }
 if (width == 0 || tmp.length() > width)
  width = tmp.length();
 width += 2;
 WINDOW* w = newwin(height + 1, width, 0, int((80 - width) / 2));
 wborder(w, LINE_XOXO, LINE_XOXO, LINE_OXOX, LINE_OXOX,
            LINE_OXXO, LINE_OOXX, LINE_XXOO, LINE_XOOX );
 tmp = buff;
 pos = tmp.find_first_of('\n');
 int line_num = 0;
 while (pos != std::string::npos) {
  std::string line = tmp.substr(0, pos);
  line_num++;
  mvwprintz(w, line_num, 1, c_white, line.c_str());
  tmp = tmp.substr(pos + 1);
  pos = tmp.find_first_of('\n');
 }
 line_num++;
 mvwprintz(w, line_num, 1, c_white, tmp.c_str());

 wrefresh(w);
 char ch;
 do
  ch = getch();
 while(ch != ' ' && ch != '\n' && ch != KEY_ESCAPE);
 werase(w);
 wrefresh(w);
 delwin(w);
 refresh();
}

void popup(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[8192];
 vsprintf(buff, mes, ap);
 va_end(ap);
 std::string tmp = buff;
 int width = 0;
 int height = 2;
 size_t pos = tmp.find_first_of('\n');
 while (pos != std::string::npos) {
  height++;
  if (pos > width)
   width = pos;
  tmp = tmp.substr(pos + 1);
  pos = tmp.find_first_of('\n');
 }
 if (width == 0 || tmp.length() > width)
  width = tmp.length();
 width += 2;
 if (height > 25)
  height = 25;
 WINDOW* w = newwin(height + 1, width, int((25 - height) / 2),
                    int((80 - width) / 2));
 wborder(w, LINE_XOXO, LINE_XOXO, LINE_OXOX, LINE_OXOX,
            LINE_OXXO, LINE_OOXX, LINE_XXOO, LINE_XOOX );
 tmp = buff;
 pos = tmp.find_first_of('\n');
 int line_num = 0;
 while (pos != std::string::npos) {
  std::string line = tmp.substr(0, pos);
  line_num++;
  mvwprintz(w, line_num, 1, c_white, line.c_str());
  tmp = tmp.substr(pos + 1);
  pos = tmp.find_first_of('\n');
 }
 line_num++;
 mvwprintz(w, line_num, 1, c_white, tmp.c_str());

 wrefresh(w);
 char ch;
 do
  ch = getch();
 while(ch != ' ' && ch != '\n' && ch != KEY_ESCAPE);
 werase(w);
 wrefresh(w);
 delwin(w);
 refresh();
}

void full_screen_popup(const char* mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[8192];
 vsprintf(buff, mes, ap);
 va_end(ap);
 std::string tmp = buff;
 WINDOW* w = newwin(25, 80, 0, 0);
 wborder(w, LINE_XOXO, LINE_XOXO, LINE_OXOX, LINE_OXOX,
            LINE_OXXO, LINE_OOXX, LINE_XXOO, LINE_XOOX );
 size_t pos = tmp.find_first_of('\n');
 int line_num = 0;
 while (pos != std::string::npos) {
  std::string line = tmp.substr(0, pos);
  line_num++;
  mvwprintz(w, line_num, 1, c_white, line.c_str());
  tmp = tmp.substr(pos + 1);
  pos = tmp.find_first_of('\n');
 }
 line_num++;
 mvwprintz(w, line_num, 1, c_white, tmp.c_str());
 wrefresh(w);
 char ch;
 do
  ch = getch();
 while(ch != ' ' && ch != '\n' && ch != KEY_ESCAPE);
 werase(w);
 wrefresh(w);
 delwin(w);
 refresh();
}

// Graphical tileset code
// ==============================================================================================

Tileset* active_tileset;

#ifdef __TILESET
extern "C" {
 PDCEX SDL_Surface* pdc_tileset;
}
#endif

bool draw_object(WINDOW* w, int x, int y, int name, bool highlight, bool alpha) {
 if(name == 0) { return false; }
 if(highlight) highlight = A_BOLD;
 #ifdef __TILESET
  if(active_tileset) {
   if(!alpha) {
    mvwaddch(w,y,x, ' ');
    wrefresh(w); // need to call refresh because otherwise the draw will be quequed and overwritten
   }
   mvwaddch(w,y,x, A_TILESET | (name + 0x80) | highlight );
   wrefresh(w);
   return true;
  } // if symbol doesn't exist, it's rendered as the normal character
 #endif
 return false;
}

void set_screen_size(int argc, char *argv[]) {
 #ifdef __TILESET
 int tmp_sheight = 0, tmp_swidth = 0;
 for(int i=0; i<argc; i++) {
  if(i == 1) {
   char* param = argv[i];
   // magic to convert char* to int cross-platform
   std::stringstream paramstream(param);
   paramstream >> tmp_swidth;
  } else if(i == 2) {
   char* param = argv[i];
   std::stringstream paramstream(param);
   paramstream >> tmp_sheight;
  }
 }
 pdc_sheight = tmp_sheight;
 pdc_swidth = tmp_swidth;
 #endif
}

char rand_char()
{
 switch (rng(0, 9)) {
  case 0:	return '|';
  case 1:	return '-';
  case 2:	return '#';
  case 3:	return '?';
  case 4:	return '&';
  case 5:	return '.';
  case 6:	return '%';
  case 7:	return '{';
  case 8:	return '*';
  case 9:	return '^';
 }
 return '?';
}


int Tileset::name_to_position(const char* name) {
 #ifdef __TILESET
 if(!pdc_tileset) return 0;
 #endif
 if(string_to_position.find(name) == string_to_position.end()) return 0;
 return string_to_position[name];
}

// Deon's tileset sprite positions
// ===============================

Tileset::Tileset() {
 string_to_position["cursor"]          = 0;
 string_to_position["human corpse"]    = 0;

 string_to_position["self"]            = 1;

 string_to_position["corpse"]          = 0;
 string_to_position["cursor"]          = 9;
 string_to_position["zombie"]          = 17;
 string_to_position["shrieker zombie"] = 18;
 string_to_position["spitter zombie"]  = 19;
 string_to_position["shocker zombie"]  = 20;
 string_to_position["zombie hunter"]   = 21;
 string_to_position["zombie brute"]    = 22;
 string_to_position["zombie hulk"]     = 23;
 string_to_position["fungal zombie"]   = 24;
 string_to_position["boomer"]          = 25;
 string_to_position["fungal boomer"]   = 26;
 string_to_position["skeleton"]        = 27;
 string_to_position["zombie necromancer"] = 28;
 string_to_position["zombie scientist"]= 29;

 string_to_position["triffid"]         = 33;
 string_to_position["young triffid"]   = 34;
 string_to_position["queen triffid"]   = 35;
 string_to_position["fungaloid"]       = 36;
 string_to_position["young fungaloid"] = 37;
 string_to_position["spore"]           = 38;
 string_to_position["blob"]            = 39;
 string_to_position["small blob"]      = 40;
 string_to_position["ant larva"]       = 41;
 string_to_position["giant ant"]       = 42;
 string_to_position["soldier ant"]     = 43;
 string_to_position["ant queen"]       = 44;
 string_to_position["fungal insect"]   = 45;
 string_to_position["giant bee"]       = 46;
 string_to_position["giant wasp"]      = 47;

 string_to_position["sewer fish"]      = 49;
 string_to_position["sewer snake"]     = 50;
 string_to_position["sewer rat"]       = 51;
 string_to_position["dog"]             = 52;
 string_to_position["wolf"]            = 53;
 string_to_position["squirrel"]        = 54;
 string_to_position["rabbit"]          = 55;
 string_to_position["deer"]            = 56;
 string_to_position["bear"]            = 57;
 string_to_position["graboid"]         = 58;
 string_to_position["giant worm"]      = 59;
 string_to_position["half worm"]       = 60;

 string_to_position["giant mosquito"]  = 65;
 string_to_position["giant dragonfly"] = 66;
 string_to_position["giant centipede"] = 67;
 string_to_position["giant frog"]      = 68;
 string_to_position["giant slug"]      = 69;
 string_to_position["dermatik larva"]  = 70;
 string_to_position["dermatik"]        = 71;

 string_to_position["flying polyp"]        = 81;
 string_to_position["hunting horror"]      = 82;
 string_to_position["Mi-go"]               = 83;
 string_to_position["yugg"]                = 84;
 string_to_position["gelatinous blob"]     = 85;
 string_to_position["flaming eye"]         = 86;
 string_to_position["kreck"]               = 87;
 string_to_position["blank body"]          = 88;
 string_to_position["Gozu"]                = 89;

 string_to_position["eyebot"]               = 97;
 string_to_position["manhack"]              = 98;
 string_to_position["skitterbot"]           = 99;
 string_to_position["secubot"]              = 100;
 string_to_position["copbot"]               = 101;
 string_to_position["molebot"]              = 102;
 string_to_position["tripod robot"]         = 103;
 string_to_position["chicken walker"]       = 104;
 string_to_position["tankbot"]              = 105;
 string_to_position["turret"]               = 106;

 string_to_position["wall NS"]            = 144;
 string_to_position["wall WE"]            = 144;
 string_to_position["solid rock"]         = 145;
 string_to_position["metal wall"]         = 145;
 string_to_position["glass wall"]         = 146;
 string_to_position["reinforced glass"]   = 146;
 string_to_position["closed wood door"]   = 147;
 string_to_position["locked wood door"]   = 147;
 string_to_position["damaged wood door"]   = 148;
 string_to_position["open wood door"]   = 149;
 string_to_position["empty door frame"]   = 149;
 string_to_position["boarded up door"]   = 150;
 string_to_position["window"]   = 151;
 string_to_position["window frame"]   = 152;
 string_to_position["boarded up window"]   = 153;
 string_to_position["metal grate"]   = 154;
 string_to_position["closed metal door"]   = 155;
 string_to_position["open metal door"]   = 156;

 string_to_position["dirt"]            = 160;
 string_to_position["grass"]            = 161;
 string_to_position["floor"]            = 162;
 string_to_position["rock floor"]            = 164;
 string_to_position["sidewalk"]            = 163;
 string_to_position["pavement"]            = 164;
 string_to_position["yellow pavement"]     = 165;
 string_to_position["wax floor"]     = 166;
 string_to_position["wax wall"]     = 167;
 string_to_position["picket fence WE"]     = 168;
 string_to_position["railing WE"]     = 168;
 string_to_position["picket fence NS"]     = 169;
 string_to_position["railing NS"]     = 169;
 string_to_position["backboard"]     = 170;
 string_to_position["gasoline pump"]     = 171;
 string_to_position["smashed gas pump"]     = 172;
 string_to_position["toilet"]     = 173;
 string_to_position["dumpster"]     = 174;
 string_to_position["radio controls"]     = 175;
 string_to_position["computer console"]     = 175;
 string_to_position["card reader"]     = 175;
 string_to_position["slot machine"]     = 175;

 string_to_position["broken computer console"]     = 176;
 string_to_position["broken card reader"]     = 176;


 string_to_position["book"]            = 208;
 string_to_position["bandages"]        = 209;
 string_to_position["first aid"]       = 210;
 string_to_position["vitamins"]        = 211;
 string_to_position["aspirin"]        = 211;
 string_to_position["caffeine pills"]        = 211;
 string_to_position["sleeping pills"]        = 211;
 string_to_position["iodine tablets"]        = 211;
 string_to_position["Dayquil"]        = 211;
 string_to_position["Nyquil"]        = 211;
 string_to_position["inhaler"]        = 212;
 string_to_position["cigarettes"]        = 213;

 string_to_position["batteries"]           = 192;
 string_to_position["plutonium cell"]           = 193;
 string_to_position["nails"]           = 194;

 string_to_position["BB"]           = 195;
 string_to_position["birdshot"]           = 195;
 string_to_position["00 shot"]           = 195;
 string_to_position["shotgun slug"]           = 195;
 string_to_position[".22 LR"]           = 195;
 string_to_position[".22 CB"]           = 195;
 string_to_position[".22 rat-shot"]           = 195;
 string_to_position["9mm"]           = 195;
 string_to_position["9mm +P"]           = 195;
 string_to_position["9mm +P+"]           = 195;
 string_to_position[".38 Special"]           = 195;
 string_to_position[".38 Super"]           = 195;
 string_to_position["10mm Auto"]           = 195;
 string_to_position[".40 S&W"]           = 195;
 string_to_position[".44 Magnum"]           = 195;
 string_to_position[".45 ACP"]           = 195;
 string_to_position[".45 FMJ"]           = 195;
 string_to_position[".45 Super"]           = 195;
 string_to_position["7.62x51mm"]           = 195;
 string_to_position["7.62x51mm incendiary"]           = 195;
 string_to_position["5.7x28mm"]           = 195;

 string_to_position["holster"]           = 203;
 string_to_position["bootstrap"]           = 203;

 string_to_position["chitinous helmet"]           = 204;
 string_to_position["army helmet"]           = 205;
 string_to_position["skid lid"]           = 206;
 string_to_position["bike helmet"]           = 206;
 string_to_position["motorcycle helmet"]           = 206;
 string_to_position["baseball helmet"]           = 207;

 string_to_position["fur hat"]           = 215;
 string_to_position["cotton hat"]           = 216;
 string_to_position["knit hat"]           = 217;
 string_to_position["boonie hat"]           = 217;
 string_to_position["baseball cap"]           = 218;
 string_to_position["hunting cap"]           = 218;

 string_to_position["safety glasses"]           = 219;
 string_to_position["ski goggles"]           = 219;
 string_to_position["welding goggles"]           = 219;
 string_to_position["light amp goggles"]           = 219;

 string_to_position["eyeglasses"]           = 220;
 string_to_position["reading glasses"]           = 220;
 string_to_position["swim goggles"]           = 220;

 string_to_position["gas mask"]           = 221;
 string_to_position["dust mask"]           = 222;
 string_to_position["filter mask"]           = 222;

 string_to_position["light gloves"]           = 223;
 string_to_position["mittens"]           = 223;
 string_to_position["wool gloves"]           = 223;
 string_to_position["winter gloves"]           = 223;
 string_to_position["leather gloves"]           = 223;
 string_to_position["fingerless gloves"]           = 223;
 string_to_position["rubber gloves"]           = 223;
 string_to_position["medical gloves"]           = 223;
 string_to_position["fire gauntlets"]           = 223;

 string_to_position["boots"]           = 224;
 string_to_position["steeltoed boots"] = 224;
 string_to_position["winter boots"]    = 224;
 string_to_position["mocassins"]       = 224;
 string_to_position["flip-flops"]      = 225;
 string_to_position["sneakers"]        = 225;
 string_to_position["dress shoes"]     = 226;
 string_to_position["heels"]           = 226;
 string_to_position["messenger bag"]           = 227;
 string_to_position["backpack"]           = 228;
 string_to_position["purse"]           = 229;
 string_to_position["fanny pack"]      = 229;

 string_to_position["jeans"]      = 230;
 string_to_position["pants"]      = 231;
 string_to_position["leather pants"]      = 232;
 string_to_position["cargo pants"]      = 233;
 string_to_position["army pants"]      = 234;
 string_to_position["skirt"]      = 235;
 string_to_position["jumpsuit"]      = 236;
 string_to_position["dress"]      = 237;
 string_to_position["chitinous armor"]      = 238;
 string_to_position["kevlar vest"]      = 237;

 string_to_position["scissors"]        = 240;
 string_to_position["hammer"]          = 241;
 string_to_position["wrench"]          = 242;
 string_to_position["screwdriver"]     = 243;
 string_to_position["soldering iron"]     = 243;
 string_to_position["wood saw"]        = 244;
 string_to_position["hack saw"]        = 245;
 string_to_position["sledge hammer"]   = 246;
 string_to_position["jack hammer"]     = 247;
 string_to_position["chainsaw"]   = 248;
 string_to_position["crowbar"]   = 249;
 string_to_position["two by four"]   = 250;
 string_to_position["heavy stick"]   = 251;
 string_to_position["pipe"]   = 252;
 string_to_position["muffler"]   = 253;
 string_to_position["baseball bat"]   = 254;
 string_to_position["rubber hose"]   = 255;
 string_to_position["syringe"]   = 256;
 string_to_position["rag"]   = 257;
 string_to_position["fur pelt"]   = 258;
 string_to_position["leather pelt"]   = 258;
 string_to_position["superglue"]   = 259;
 string_to_position["ID card"]   = 260;
 string_to_position["electrohack"]   = 261;
 string_to_position["string - 6 in"]   = 262;
 string_to_position["string - 3 ft"]   = 262;
 string_to_position["rope - 6 ft"]   = 263;
 string_to_position["rope - 30 ft"]   = 263;
 string_to_position["steel chain"]   = 264;
 string_to_position["processor board"]   = 265;
 string_to_position["RAM"]   = 266;
 string_to_position["power converter"]   = 267;
 string_to_position["amplifier circuit"]   = 268;
 string_to_position["transponder circuit"]   = 268;
 string_to_position["signar receiver"]   = 269;
 string_to_position["antenna"]   = 270;
 string_to_position["chunk of steel"]   = 271;
 string_to_position["electric motor"]   = 272;
 string_to_position["sheet of glass"]   = 273;
 string_to_position["manhole cover"]   = 274;
 string_to_position["rock"]   = 275;
 string_to_position["broom"]   = 276;
 string_to_position["mop"]   = 277;
 string_to_position["hatchet"]   = 278;
 string_to_position["wood ax"]   = 279;
 string_to_position["nail board"]   = 280;
 string_to_position["X-Acto knife"]   = 281;
 string_to_position["pot"]   = 282;
 string_to_position["frying pan"]   = 283;
 string_to_position["butter knife"]   = 284;
 string_to_position["steak knife"]   = 284;
 string_to_position["butcher knife"]   = 284;
 string_to_position["combat knife"]   = 284;
 string_to_position["machete"]   = 285;
 string_to_position["katana"]   = 286;
 string_to_position["wood spear"]   = 287;
 string_to_position["steel spear"]   = 288;
 string_to_position["expandable baton"]   = 289;
 string_to_position["bee sting"]   = 290;
 string_to_position["wasp sting"]   = 290;
 string_to_position["chunk of chitin"]   = 291;
 string_to_position["empty canister"]   = 292;
 string_to_position["gold bar"]   = 293;
 string_to_position["flashlight (on)"]   = 294;
 string_to_position["flashlight (off)"]   = 295;
 string_to_position["lighter"]   = 296;
 string_to_position["fire extinguisher"]   = 297;
 string_to_position["plastic bottle"]   = 298;
 string_to_position["glass bottle"]   = 299;
 string_to_position["sm. cardboard box"]   = 300;
 string_to_position["aluminum can"]   = 301;
 string_to_position["tin can"]   = 301;
 string_to_position["plastic bag"]   = 302;

 string_to_position["rain coat"]   = 304;
 string_to_position["trenchcoat"]   = 305;
 string_to_position["peacoat"]   = 305;
 string_to_position["fur coat"]   = 305;
 string_to_position["winter coat"]   = 305;
 string_to_position["wool poncho"]   = 306;
 string_to_position["t shirt"]   = 307;
 string_to_position["polo shirt"]   = 307;
 string_to_position["dress shirt"]   = 308;
 string_to_position["tank top"]   = 309;
 string_to_position["sweatshirt"]   = 310;
 string_to_position["sweater"]   = 310;

 string_to_position["shallow water"]   = 125;
 string_to_position["deep water"]   = 126;
 string_to_position["sewage"]   = 127;

 string_to_position["subway station"]   = 128;
 string_to_position["police station"]   = 129;
 string_to_position["library"]   = 130;
 string_to_position["grocery store"]   = 131;
 string_to_position["sporting goods store"]   = 132;
 string_to_position["gun store"]   = 133;
 string_to_position["clothing store"]   = 134;
 string_to_position["hardware store"]   = 135;
 string_to_position["pharmacy"]   = 136;
 string_to_position["bar"]   = 137;
 string_to_position["liquor store"]   = 138;
 string_to_position["bank"]   = 139;

 string_to_position["bee hive"]     = 167;
 string_to_position["house"]   = 140;
 string_to_position["swamp"]   = 141;
 string_to_position["park"]   = 142;
 string_to_position["forest"]   = 143;

 string_to_position["river"]   = 126;
 string_to_position["river bank"]   = 125;
 string_to_position["field"]   = 160;
 string_to_position["road"]   = 164;
 string_to_position["highway"]   = 164;
 string_to_position["road, manhole"]   = 165;
 string_to_position["parking lot"]     = 165;
 string_to_position["gas station"]     = 171;
 string_to_position["science lab"]     = 124;

 string_to_position["underbrush"]   = 141;
 string_to_position["marloss bush"]   = 141;
 string_to_position["young tree"]   = 142;
 string_to_position["tree"]   = 143;
}

#endif // _OUTPUT_CPP_
