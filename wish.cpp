#include "game.h"
#include "output.h"
#include "keypress.h"
#include "skill.h"
#include <sstream>

#define LESS(a, b) ((a)<(b)?(a):(b))

void game::wish()
{
 WINDOW* w_list = newwin(25, 30, 0,  0);
 WINDOW* w_info = newwin(25, 50, 0, 30);
 int a = 0, shift = 0, result_selected = 0;
 int line;
 char ch = '.';
 bool search = false, found = false;
 std::string pattern;
 std::string info;
 std::vector<int> search_results;
 item tmp;
 tmp.corpse = mtypes[0];
 do {
  werase(w_info);
  werase(w_list);
  mvwprintw(w_list, 0, 0, "Wish for a: ");
  if (search) {
   found = false;
   if (ch == '\n') {
    search = false;
    found = true;
    ch = '.';
   } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 7) { // backspace seems to be processed as 7 (\a) on some debian systems.
    if (pattern.length() > 0)
     pattern.erase(pattern.end() - 1);
   } else if (ch == '>') {
    search = false;
    if (!search_results.empty()) {
     result_selected++;
     if (result_selected > search_results.size())
      result_selected = 0;
     shift = search_results[result_selected];
     a = 0;
     if (shift + 23 > itypes.size()) {
      a = shift + 23 - itypes.size();
      shift = itypes.size() - 23;
     }
    }
   } else if (ch == '<') {
    search = false;
    if (!search_results.empty()) {
     result_selected--;
     if (result_selected < 0)
      result_selected = search_results.size() - 1;
     shift = search_results[result_selected];
     a = 0;
     if (shift + 23 > itypes.size()) {
      a = shift + 23 - itypes.size();
      shift = itypes.size() - 23;
     }
    }
   } else {
    pattern += ch;
    search_results.clear();
   }

   if (search) {
    for (int i = 0; i < itypes.size(); i++) {
     if (itypes[i]->name.find(pattern) != std::string::npos) {
      shift = i;
      a = 0;
      result_selected = 0;
      if (shift + 23 > itypes.size()) {
       a = shift + 23 - itypes.size();
       shift = itypes.size() - 23;
      }
      found = true;
      search_results.push_back(i);
     }
    }
    if (search_results.size() > 0) {
     shift = search_results[0];
     a = 0;
    }
   }

  } else {	// Not searching; scroll by keys
   if (ch == 'j') a++;
   if (ch == 'k') a--;
   if (ch == '/') {
    search = true;
    pattern =  "";
    found = false;
    search_results.clear();
   }
   if (ch == '>' && !search_results.empty()) {
    result_selected++;
    if (result_selected > search_results.size())
     result_selected = 0;
    shift = search_results[result_selected];
    a = 0;
    if (shift + 23 > itypes.size()) {
     a = shift + 23 - itypes.size();
     shift = itypes.size() - 23;
    }
   } else if (ch == '<' && !search_results.empty()) {
    result_selected--;
    if (result_selected < 0)
     result_selected = search_results.size() - 1;
    shift = search_results[result_selected];
    a = 0;
    if (shift + 23 > itypes.size()) {
     a = shift + 23 - itypes.size();
     shift = itypes.size() - 23;
    }
   }
  }
  if (!search_results.empty())
   mvwprintz(w_list, 0, 11, c_green, "%s               ", pattern.c_str());
  else if (pattern.length() > 0)
   mvwprintz(w_list, 0, 11, c_red, "%s not found!            ",pattern.c_str());
  if (a < 0) {
   a = 0;
   shift--;
   if (shift < 0) shift = 0;
  }
  if (a > 22) {
   a = 22;
   shift++;
   if (shift + 23 > itypes.size()) shift = itypes.size() - 23;
  }
  for (int i = 1; i < 24; i++) {
   nc_color col = c_white;
   if (i == a + 1)
    col = h_white;
   mvwprintz(w_list, i, 0, col, itypes[i-1+shift]->name.c_str());
   wprintz(w_list, itypes[i-1+shift]->color, "%c%", itypes[i-1+shift]->sym);
  }
  tmp.make(itypes[a + shift]);
  if (tmp.is_tool())
   tmp.charges = dynamic_cast<it_tool*>(tmp.type)->max_charges;
  else if (tmp.is_ammo())
   tmp.charges = 100;
  else
   tmp.charges = -1;
  info = tmp.info(true);
  mvwprintw(w_info, 1, 0, info.c_str());
  wrefresh(w_info);
  wrefresh(w_list);
  if (search)
   ch = getch();
  else
   ch = input();
 } while (ch != '\n');
 clear();
 mvprintw(0, 0, "\nWish granted - %d (%d).", tmp.type->id, itm_antibiotics);
 tmp.invlet = nextinv;
 u.i_add(tmp);
 advance_nextinv();
 getch();
 delwin(w_info);
 delwin(w_list);
}

void game::monster_wish()
{
 WINDOW* w_list = newwin(25, 30, 0,  0);
 WINDOW* w_info = newwin(25, 50, 0, 30);
 int a = 0, shift = 1, result_selected = 0;
 int line;
 char ch = '.';
 bool search = false, found = false, friendly = false;
 std::string pattern;
 std::string info;
 std::vector<int> search_results;
 monster tmp;
 do {
  werase(w_info);
  werase(w_list);
  mvwprintw(w_list, 0, 0, "Spawn a: ");
  if (search) {
   found = false;
   if (ch == '\n') {
    search = false;
    found = true;
    ch = '.';
   } else if (ch == KEY_BACKSPACE || ch == 127) {
    if (pattern.length() > 0)
     pattern.erase(pattern.end() - 1);
   } else if (ch == '>') {
    search = false;
    if (!search_results.empty()) {
     result_selected++;
     if (result_selected > search_results.size())
      result_selected = 0;
     shift = search_results[result_selected];
     a = 0;
     if (shift + 23 > mtypes.size()) {
      a = shift + 23 - mtypes.size();
      shift = mtypes.size() - 23;
     }
    }
   } else if (ch == '<') {
    search = false;
    if (!search_results.empty()) {
     result_selected--;
     if (result_selected < 0)
      result_selected = search_results.size() - 1;
     shift = search_results[result_selected];
     a = 0;
     if (shift + 23 > mtypes.size()) {
      a = shift + 23 - mtypes.size();
      shift = mtypes.size() - 23;
     }
    }
   } else {
    pattern += ch;
    search_results.clear();
   }

   if (search) {
    for (int i = 1; i < mtypes.size(); i++) {
     if (mtypes[i]->name.find(pattern) != std::string::npos) {
      shift = i;
      a = 0;
      result_selected = 0;
      if (shift + 23 > mtypes.size()) {
       a = shift + 23 - mtypes.size();
       shift = mtypes.size() - 23;
      }
      found = true;
      search_results.push_back(i);
     }
    }
   }

  } else {	// Not searching; scroll by keys
   if (ch == 'j') a++;
   if (ch == 'k') a--;
   if (ch == 'f') friendly = !friendly;
   if (ch == '/') {
    search = true;
    pattern =  "";
    found = false;
    search_results.clear();
   }
   if (ch == '>' && !search_results.empty()) {
    result_selected++;
    if (result_selected > search_results.size())
     result_selected = 0;
    shift = search_results[result_selected];
    a = 0;
    if (shift + 23 > mtypes.size()) {
     a = shift + 23 - mtypes.size();
     shift = mtypes.size() - 23;
    }
   } else if (ch == '<' && !search_results.empty()) {
    result_selected--;
    if (result_selected < 0)
     result_selected = search_results.size() - 1;
    shift = search_results[result_selected];
    a = 0;
    if (shift + 23 > mtypes.size()) {
     a = shift + 23 - mtypes.size();
     shift = mtypes.size() - 23;
    }
   }
  }
  if (!search_results.empty())
   mvwprintz(w_list, 0, 11, c_green, "%s               ", pattern.c_str());
  else if (pattern.length() > 0)
   mvwprintz(w_list, 0, 11, c_red, "%s not found!            ",pattern.c_str());
  if (a < 0) {
   a = 0;
   shift--;
   if (shift < 1) shift = 1;
  }
  if (a > 22) {
   a = 22;
   shift++;
   if (shift + 23 > mtypes.size()) shift = mtypes.size() - 23;
  }
  for (int i = 1; i < 24; i++) {
   nc_color col = c_white;
   if (i == a + 1)
    col = h_white;
   mvwprintz(w_list, i, 0, col, mtypes[i-1+shift]->name.c_str());
   wprintz(w_list, mtypes[i-1+shift]->color, " %c%", mtypes[i-1+shift]->sym);
  }
  tmp = monster(mtypes[a + shift]);
  if (friendly)
   tmp.friendly = -1;
  tmp.print_info(this, w_info);
  wrefresh(w_info);
  wrefresh(w_list);
  if (search)
   ch = getch();
  else
   ch = input();
 } while (ch != '\n');
 clear();
 delwin(w_info);
 delwin(w_list);
 refresh_all();
 wrefresh(w_terrain);
 point spawn = look_around();
 if (spawn.x == -1)
  return;
 tmp.spawn(spawn.x, spawn.y);
 z.push_back(tmp);
}

void game::mutation_wish()
{
 WINDOW* w_list = newwin(25, 30, 0,  0);
 WINDOW* w_info = newwin(25, 50, 0, 30);
 int a = 0, shift = 0, result_selected = 0;
 int line;
 char ch = '.';
 bool search = false, found = false;
 std::string pattern;
 std::string info;
 std::vector<int> search_results;
 do {
  werase(w_info);
  werase(w_list);
  mvwprintw(w_list, 0, 0, "Mutate: ");
  if (search) {
   found = false;
   if (ch == '\n') {
    search = false;
    found = true;
    ch = '.';
   } else if (ch == KEY_BACKSPACE || ch == 127) {
    if (pattern.length() > 0)
     pattern.erase(pattern.end() - 1);
   } else if (ch == '>') {
    search = false;
    if (!search_results.empty()) {
     result_selected++;
     if (result_selected > search_results.size())
      result_selected = 0;
     shift = search_results[result_selected];
     a = 0;
     if (shift + 23 > PF_MAX2) {
      a = shift + 23 - PF_MAX2;
      shift = PF_MAX2 - 23;
     }
    }
   } else if (ch == '<') {
    search = false;
    if (!search_results.empty()) {
     result_selected--;
     if (result_selected < 0)
      result_selected = search_results.size() - 1;
     shift = search_results[result_selected];
     a = 0;
     if (shift + 23 > PF_MAX2) {
      a = shift + 23 - PF_MAX2;
      shift = PF_MAX2 - 23;
     }
    }
   } else {
    pattern += ch;
    search_results.clear();
   }

   if (search) {
    for (int i = 0; i < PF_MAX2; i++) {
     if (traits[i].name.find(pattern) != std::string::npos) {
      shift = i;
      a = 0;
      result_selected = 0;
      if (shift + 23 > PF_MAX2) {
       a = shift + 23 - PF_MAX2;
       shift = PF_MAX2 - 23;
      }
      found = true;
      search_results.push_back(i);
     }
    }
    if (search_results.size() > 0) {
     shift = search_results[0];
     a = 0;
    }
   }

  } else {	// Not searching; scroll by keys
   if (ch == 'j') a++;
   if (ch == 'k') a--;
   if (ch == '/') {
    search = true;
    pattern =  "";
    found = false;
    search_results.clear();
   }
   if (ch == '>' && !search_results.empty()) {
    result_selected++;
    if (result_selected > search_results.size())
     result_selected = 0;
    shift = search_results[result_selected];
    a = 0;
    if (shift + 23 > PF_MAX2) {
     a = shift + 23 - PF_MAX2;
     shift = PF_MAX2 - 23;
    }
   } else if (ch == '<' && !search_results.empty()) {
    result_selected--;
    if (result_selected < 0)
     result_selected = search_results.size() - 1;
    shift = search_results[result_selected];
    a = 0;
    if (shift + 23 > PF_MAX2) {
     a = shift + 23 - PF_MAX2;
     shift = PF_MAX2 - 23;
    }
   }
  }
  if (!search_results.empty())
   mvwprintz(w_list, 0, 11, c_green, "%s               ", pattern.c_str());
  else if (pattern.length() > 0)
   mvwprintz(w_list, 0, 11, c_red, "%s not found!            ",pattern.c_str());
  if (a < 0) {
   a = 0;
   shift--;
   if (shift < 0) shift = 0;
  }
  if (a > 22) {
   a = 22;
   shift++;
   if (shift + 23 > PF_MAX2) shift = PF_MAX2 - 23;
  }
  for (int i = 1; i < 24; i++) {
   nc_color col = c_white;
   if (i == a + 1)
    col = h_white;
   mvwprintz(w_list, i, 0, col, traits[i-1+shift].name.c_str());
  }
  mvwprintw(w_info, 1, 0, mutation_data[a+shift].valid ? "Valid" : "Nonvalid");
  int line2 = 2;
  mvwprintw(w_info, line2, 0, "Prereqs:");
  for (int j = 0; j < mutation_data[a+shift].prereqs.size(); j++) {
   mvwprintw(w_info, line2, 9, traits[ mutation_data[a+shift].prereqs[j] ].name.c_str());
   line2++;
  }
  mvwprintw(w_info, line2, 0, "Cancels:");
  for (int j = 0; j < mutation_data[a+shift].cancels.size(); j++) {
   mvwprintw(w_info, line2, 9, traits[ mutation_data[a+shift].cancels[j] ].name.c_str());
   line2++;
  }
  mvwprintw(w_info, line2, 0, "Becomes:");
  for (int j = 0; j < mutation_data[a+shift].replacements.size(); j++) {
   mvwprintw(w_info, line2, 9, traits[ mutation_data[a+shift].replacements[j] ].name.c_str());
   line2++;
  }
  mvwprintw(w_info, line2, 0, "Add-ons:");
  for (int j = 0; j < mutation_data[a+shift].additions.size(); j++) {
   mvwprintw(w_info, line2, 9, traits[ mutation_data[a+shift].additions[j] ].name.c_str());
   line2++;
  }
  wrefresh(w_info);
  wrefresh(w_list);
  if (search)
   ch = getch();
  else
   ch = input();
 } while (ch != '\n');
 clear();
 if (a+shift == 0)
  u.mutate(this);
 else
  u.mutate_towards(this, pl_flag(a + shift));
 delwin(w_info);
 delwin(w_list);
}

// Allows player to spawn a field type at a selected point near the player.
void game::field_wish()
{
 std::vector<std::string> vec;
 
 for(int i = 0; i < num_fields; i++) {
  if(i == fd_fire_vent)
   vec.push_back("fire vent"); // Fire vent doesn't have any display name, this hack adds one to the selection list.
  else if(i == fd_null)
   vec.push_back("null field"); // Ditto for null field
  else
   vec.push_back(fieldlist[i].name[2]);
 }
 
 field_id id = (field_id) select_item("Select a field: ", vec);
 
 if(id != -1) {
  point spawn = look_around();
  m.add_field(this, spawn.x, spawn.y, id, 3);
 }
}

void game::modify_character()
{
 std::vector<std::string> vec;
 int selection, input;
 
 vec.push_back("Modify stats...");
 vec.push_back("Modify skills...");
 vec.push_back("Set HP...");
 
 selection = select_item("Modify Character", vec); 
 
 vec.clear();
 
 if(selection == 0) // Modify stats
 {
  vec.push_back("Set all stats to 20");
  vec.push_back("Set str...");
  vec.push_back("Set dex...");
  vec.push_back("Set int...");
  vec.push_back("Set per...");
  
  selection = select_item("Modify stats", vec);
  vec.clear();
  
  if(selection == 0) // Set all stats to 20
  {
   u.str_max = u.str_cur = 20;
   u.dex_max = u.dex_cur = 20;
   u.int_max = u.int_cur = 20;
   u.per_max = u.per_cur = 20;
  }
  
  else if(selection >= 1 && selection <= 4) // Set individual stats
  {
   input = query_int(100, "Input a number from 0-100");
   if(selection == 1)
    u.str_max = u.str_cur = input;
   else if(selection == 2)
    u.dex_max = u.dex_cur = input;
   else if(selection == 3)
    u.int_max = u.int_cur = input;
   else if(selection == 4)
    u.per_max = u.per_cur = input;
  }

   
   
 }
 else if(selection == 1) // Modify skills
 {
  vec.push_back("Set all skills to 20");
  
  for(int i = 1; i < num_skill_types; i++)
   vec.push_back("Set " + skill_name(i) + "...");
  
  selection = select_item("Modify skills", vec);
  vec.clear();
  
  if(selection == 0) // Set all skills to 20
  {
   for(int i = 1; i < num_skill_types; i++)
    u.sklevel[i] = 20;
  }
  
  else if(selection >= 1 && selection < num_skill_types) // Set individual skills
  {
   input = query_int(100, "Input a number from 0-100");
   u.sklevel[selection] = input;
  }
 }
 
 else if(selection == 2) // Set HP
 {
  input = query_int(10000, "Input a number from 0-10000");
  for(int i = 0; i < num_hp_parts; i++)
   u.hp_max[i] = u.hp_cur[i] = input;
 }
}