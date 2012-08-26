#include "game.h"
#include "setvector.h"
#include "output.h"
#include "keypress.h"
#include "player.h"
#include "inventory.h"
#include "mapdata.h"
#include "skill.h"
#include "crafting.h" // For the use_comps use_tools functions


bool will_flood_stop(map *m, bool fill[SEEX * MAPSIZE][SEEY * MAPSIZE],
                     int x, int y);

void game::init_construction()
{
 int id = -1;
 int tl, cl, sl;

 #define CONSTRUCT(name, able, done) \
  sl = -1; id++; \
  constructions.push_back( new constructable(id, name, able, done))

 #define STAGE(...)\
  tl = 0; cl = 0; sl++; \
  constructions[id]->stages.push_back(construction_stage(__VA_ARGS__));
 #define TOOL(...)   setvector(constructions[id]->stages[sl].tools[tl], \
                               __VA_ARGS__); tl++
 #define COMP(...)   setvector(constructions[id]->stages[sl].components[cl], \
                               __VA_ARGS__); cl++

/* CONSTRUCT( name, time, able, done )
 * Name is the name as it appears in the menu; 30 characters or less, please.
 * time is the time in MINUTES that it takes to finish this construction.
 *  note that 10 turns = 1 minute.
 * able is a function which returns true if you can build it on a given tile
 *  See construction.h for options, and this file for definitions.
 * done is a function which runs each time the construction finishes.
 *  This is useful, for instance, for removing the trap from a pit, or placing
 *  items after a deconstruction.
 */

 CONSTRUCT("Dig and spike pit", &construct::able_dig, &construct::done_nothing);
  STAGE(0, t_pit_shallow, 10);
   TOOL(itm_shovel, NULL);
  STAGE(0, t_pit, 10);
   TOOL(itm_shovel, NULL);


  STAGE(0, t_pit_spiked, 5);
   COMP(itm_spear_wood, 4, NULL);

 CONSTRUCT("Fill pit", &construct::able_pit, &construct::done_nothing);
  STAGE(0, t_pit_shallow, 5);
   TOOL(itm_shovel, NULL);
  STAGE(0, t_dirt, 5);
   TOOL(itm_shovel, NULL);

 CONSTRUCT("Chop down tree", &construct::able_tree, &construct::done_tree);
  STAGE(0, t_dirt, 10);
   TOOL(itm_ax, itm_chainsaw_on, NULL);

 CONSTRUCT("Chop up log", &construct::able_log, &construct::done_log);
  STAGE(0, t_dirt, 20);
   TOOL(itm_ax, itm_chainsaw_on, NULL);

 CONSTRUCT("Clean broken window", &construct::able_broken_window,
                                  &construct::done_nothing);
  STAGE(0, t_window_empty, 5);

 CONSTRUCT("Remove window pane",  &construct::able_window_pane,
                                  &construct::done_window_pane);
  STAGE(1, t_window_empty, 10);
   TOOL(itm_hammer, itm_rock, itm_hatchet, NULL);
   TOOL(itm_screwdriver, itm_knife_butter, itm_toolset, NULL);

/*
 CONSTRUCT("Repair Door", &construct::able_door_broken, //able_door_broken used only here
                          &construct::done_nothing);
  STAGE(1, t_door_c, 10);
   TOOL(itm_hammer, itm_hatchet, itm_nailgun, NULL);
   COMP(itm_2x4, 3, NULL);
   COMP(itm_nail, 12, NULL);


 CONSTRUCT("Board up door", &construct::able_door, &construct::done_nothing); //able_door used only here
  STAGE(0, t_door_boarded, 8);
   TOOL(itm_hammer, itm_hatchet, itm_nailgun, NULL);
   COMP(itm_2x4, 4, NULL);
   COMP(itm_nail, 8, NULL);

 CONSTRUCT("Board up window", &construct::able_window, //able_window used only here
                              &construct::done_nothing);
  STAGE(0, t_window_boarded, 5);
   TOOL(itm_hammer, itm_hatchet, itm_nailgun, NULL);
   COMP(itm_2x4, 4, NULL);
   COMP(itm_nail, 8, NULL);
*/
 CONSTRUCT("Board up door or window",	&construct::able_boardup,
										&construct::done_boardup);
  STAGE(0, t_null, 6);
   TOOL(itm_hammer, itm_hatchet, itm_nailgun, NULL);
   COMP(itm_2x4, 4, NULL);
   COMP(itm_nail, 8, NULL);

 CONSTRUCT("Build wall", &construct::able_empty, &construct::done_nothing);
  STAGE(2, t_wall_half, 10);
   TOOL(itm_hammer, itm_hatchet, itm_nailgun, NULL);
   COMP(itm_2x4, 10, NULL);
   COMP(itm_nail, 20, NULL);
  STAGE(2, t_wall_wood, 10);
   TOOL(itm_hammer, itm_hatchet, itm_nailgun, NULL);
   COMP(itm_2x4, 10, NULL);
   COMP(itm_nail, 20, NULL);

 CONSTRUCT("Build window", &construct::able_wall_wood,
                           &construct::done_nothing);
  STAGE(2, t_window_empty, 10);
   TOOL(itm_saw, NULL);
  STAGE(3, t_window, 5);
   COMP(itm_glass_sheet, 1, NULL);

 CONSTRUCT("Build or repair door", &construct::able_wall_wood,
                                   &construct::done_nothing);
  STAGE(2, t_door_frame, 15);
   TOOL(itm_saw, NULL);
  STAGE(4, t_door_b, 15);
   TOOL(itm_hammer, itm_hatchet, itm_nailgun, NULL);
   COMP(itm_2x4, 4, NULL);
   COMP(itm_nail, 12, NULL);
  STAGE(1, t_door_c, 10);
   TOOL(itm_hammer, itm_hatchet, itm_nailgun, NULL);
   COMP(itm_2x4, 3, NULL);
   COMP(itm_nail, 8, NULL);

/*  Removed until we have some way of auto-aligning fences!
 CONSTRUCT("Build Fence", 15, &construct::able_empty);
  STAGE(1, t_fence_h, 10);
   TOOL(itm_hammer, itm_hatchet, NULL);
   COMP(itm_2x4, 5, itm_nail, 8, NULL);
*/

 CONSTRUCT("Build Roof", &construct::able_between_walls,
                         &construct::done_nothing);
  STAGE(4, t_floor, 40);
   TOOL(itm_hammer, itm_hatchet, itm_nailgun, NULL);
   COMP(itm_2x4, 8, NULL);
   COMP(itm_nail, 40, NULL);

 CONSTRUCT("Start vehicle construction", &construct::able_empty,
                                         &construct::done_vehicle);
  STAGE(0, t_null, 10);
   COMP(itm_frame, 1, NULL);

}

void game::construction_menu()
{
 if (u.morale_level() < MIN_MORALE_CRAFT) {	// See morale.h
  add_msg("Your morale is too low to construct...");
  return;
 }
 WINDOW *w_con = newwin(25, 80, 0, 0);
 wborder(w_con, LINE_XOXO, LINE_XOXO, LINE_OXOX, LINE_OXOX,
                LINE_OXXO, LINE_OOXX, LINE_XXOO, LINE_XOOX );
 mvwprintz(w_con, 0,  1, c_yellow, "Construction");
 mvwprintz(w_con, 0, 31, c_yellow, "Your carpentry skill: %d.%s%d",
            u.sklevel[sk_carpentry], ((u.skexercise[sk_carpentry] < 10 ||
            u.skexercise[sk_carpentry] == 0) ? "0" : ""),
            (u.skexercise[sk_carpentry] < 0 ? 0 : u.skexercise[sk_carpentry]));
 mvwputch(w_con,  0, 30, c_white, LINE_OXXX);
 mvwputch(w_con, 24, 30, c_white, LINE_XXOX);
 for (int i = 1; i < 24; i++)
  mvwputch(w_con, i, 30, c_white, LINE_XOXO);

 wrefresh(w_con);

 bool update_info = true;
 int select = 0;
 char ch;

 inventory total_inv;
 total_inv.form_from_map(this, point(u.posx, u.posy), PICKUP_RANGE);
 total_inv.add_stack(u.inv_dump());
 if (u.has_bionic(bio_tools)) {
  item tools(itypes[itm_toolset], turn);
  tools.charges = u.power_level;
  total_inv += tools;
 }

 do {
// Erase existing list of constructions
  for (int i = 1; i < 24; i++) {
   for (int j = 1; j < 29; j++)
    mvwputch(w_con, i, j, c_black, 'x');
  }
// Determine where in the master list to start printing
  int offset = select - 11;
//  if (offset > constructions.size() - 22) // acts weird with this
//   offset = constructions.size() - 22;
  if (offset <= 0) {
   offset = 0;
   mvwputch(w_con, 1, 0, c_dkgray, LINE_XOXO);
  } else
   mvwputch(w_con, 1, 0, h_white, '^');
  if (constructions.size() - offset > 23)
   mvwputch(w_con, 23, 0, h_white, 'v');
  else
   mvwputch(w_con, 23, 0, c_dkgray, LINE_XOXO);
// Print the constructions between offset and max (or how many will fit)
  for (int i = 0; i < 23 && i + offset < constructions.size(); i++) {
   int current = i + offset;
   nc_color col = (player_can_build(u, total_inv, constructions[current]) ?
                   c_white : c_dkgray);
   if (current == select)
    col = hilite(col);
   mvwprintz(w_con, 1 + i, 1, col, constructions[current]->name.c_str());
  }

  if (update_info) {
   update_info = false;
   constructable* current_con = constructions[select];
// Clear out lines for tools & materials
   int posx = 31, posy = 0;
   for (int i = posy + 1; i < 24; i++) {
    for (int j = posx; j < 79; j++)
     mvwputch(w_con, i, j, c_black, 'x');
   }

// Print stages and their requirements
   for (int n = 0; n < current_con->stages.size(); n++) {
    posx = 31;
    posy++;
    nc_color color_stage = (player_can_build(u, total_inv,
                             current_con, n, true) ? c_white : c_dkgray);
// Print stage number and resulting terrain type
    posy++;
    if (current_con->stages[n].terrain == t_null)
     mvwprintz(w_con, posy, posx, color_stage, "Stage %d", n + 1);
    else
     mvwprintz(w_con, posy, posx, color_stage, "Stage %d: %s", n + 1,
               terlist[current_con->stages[n].terrain].name.c_str());
// Print difficulty
    if (current_con->stages[n].difficulty > 0){
     posy++;
     int pskill = u.sklevel[sk_carpentry],
         diff = current_con->stages[n].difficulty;
     mvwprintz(w_con, posy, posx, color_stage, "Difficulty:");
     mvwprintz(w_con, posy, posx + 12, (pskill >= diff ? c_green : c_red),
                "%d", diff);
    }
// Print time (in minutes)
    posy++;
    mvwprintz(w_con, posy, posx, color_stage, "Time to complete: %d minutes",
               current_con->stages[n].time);
// Print tools
    construction_stage stage = current_con->stages[n];
    bool has_tool[3] = {stage.tools[0].empty(),
                        stage.tools[1].empty(),
                        stage.tools[2].empty()};
    for (int i = 0; i < 3 && !has_tool[i]; i++) {
     posy++;
     posx = 33;
     mvwputch(w_con, posy, posx - 2, color_stage, '>');
     for (int j = 0; j < stage.tools[i].size(); j++) {
      itype_id tool = stage.tools[i][j];
      nc_color col = c_red;
      if (total_inv.has_amount(tool, 1)) {
       has_tool[i] = true;
       col = c_green;
      }
      int length = itypes[tool]->name.length();
      if (posx + length > 79) {
       posy++;
       posx = 33;
      }
      mvwprintz(w_con, posy, posx, col, itypes[tool]->name.c_str());
      posx += length + 1; // + 1 for an empty space
      if (j < stage.tools[i].size() - 1) { // "OR" if there's more
       if (posx > 77) {
        posy++;
        posx = 33;
       }
       mvwprintz(w_con, posy, posx, c_white, "OR");
       posx += 3;
      }
     }
    }
// Print components
    bool has_component[3] = {stage.components[0].empty(),
                             stage.components[1].empty(),
                             stage.components[2].empty()};
    for (int i = 0; i < 3; i++) {
     posx = 33;
     while (has_component[i])
      i++;
     for (int j = 0; j < stage.components[i].size() && i < 3; j++) {
      nc_color col = c_red;
      component comp = stage.components[i][j];
      if (( itypes[comp.type]->is_ammo() &&
           total_inv.has_charges(comp.type, comp.count)) ||
          (!itypes[comp.type]->is_ammo() &&
           total_inv.has_amount(comp.type, comp.count))) {
       has_component[i] = true;
       col = c_green;
      }
      int length = itypes[comp.type]->name.length();
      if (posx + length > 79) {
       posy++;
       posx = 33;
      }
      posy++;
      mvwputch(w_con, posy, posx - 2, color_stage, '>');
      mvwprintz(w_con, posy, posx, col, "%dx %s",
                comp.count, itypes[comp.type]->name.c_str());
      posx += length + 3; // + 2 for "x ", + 1 for an empty space
// Add more space for the length of the count
      if (comp.count < 10)
       posx++;
      else if (comp.count < 100)
       posx += 2;
      else
       posx += 3;

      if (j < stage.components[i].size() - 1) { // "OR" if there's more
       if (posx > 77) {
        posy++;
        posx = 33;
       }
       mvwprintz(w_con, posy, posx, c_white, "OR");
       posx += 3;
      }
     }
    }
   }
   wrefresh(w_con);
  } // Finished updating
 
  ch = input();
  switch (ch) {
   case 'j':
    update_info = true;
    select++;
    if (select >= constructions.size())
     select = 0;
    break;
   case 'k':
    update_info = true;
    select--;
    if (select < 0)
     select = constructions.size() - 1;
    break;
   case '\n':
   case 'l':
    if (player_can_build(u, total_inv, constructions[select])) {
     place_construction(constructions[select]);
     ch = 'q';
    } else {
     popup("You can't build that!");
     for (int i = 1; i < 24; i++)
      mvwputch(w_con, i, 30, c_white, LINE_XOXO);
     update_info = true;
    }
    break;
  }
 } while (ch != 'q' && ch != 'Q' && ch != KEY_ESCAPE);
 refresh_all();
}

bool game::player_can_build(player &p, inventory inv, constructable* con,
                            int level, bool specific)
// defaults: level==0,  specific==false
{
 if (level < 0) // used as escape value in place_construction()
  return false;

 int stop = (specific ? level : con->stages.size());
 do {
  construction_stage stage = con->stages[level];
  int number_of_tools = 0, number_of_components = 0;
  int number_of_req_tools = 0, number_of_req_components = 0;

  for (int j = 0; j < 3; j++) {
// counting available tools
   if (stage.tools[j].size() > 0) {
    number_of_req_tools++;
    for (int k = 0; k < stage.tools[j].size(); k++)
     if (inv.has_amount(stage.tools[j][k], 1)) {
      number_of_tools++;
      break;
     }
   }
// counting available components
   if (stage.components[j].size() > 0) {
    number_of_req_components++;
    for (int k = 0; k < stage.components[j].size(); k++)
     if (( itypes[stage.components[j][k].type]->is_ammo() &&
          inv.has_charges(stage.components[j][k].type,
                          stage.components[j][k].count)    ) ||
         (!itypes[stage.components[j][k].type]->is_ammo() &&
          inv.has_amount (stage.components[j][k].type,
                          stage.components[j][k].count)    )) {
      number_of_components++;
      break;
     }
   }
  }
// difficulty check + summary
  if (!(p.sklevel[sk_carpentry] < stage.difficulty) &&
      number_of_tools == number_of_req_tools &&
      number_of_components == number_of_req_components)
   return true;

  level++;
 } while (level < stop);
 return false;
}

void game::place_construction(constructable *con)
{
 refresh_all();
 inventory total_inv;
 total_inv.form_from_map(this, point(u.posx, u.posy), PICKUP_RANGE);
 total_inv.add_stack(u.inv_dump());

 std::vector<point> valid;
 for (int x = u.posx - 1; x <= u.posx + 1; x++) {
  for (int y = u.posy - 1; y <= u.posy + 1; y++) {
   if (x == u.posx && y == u.posy)
    y++;
   construct test;
// initial function check
   int stage_num = ((test.*(con->able))(this, point(x, y)) ? 0 : -1);
// looking for result tiles from completed previous stages
   for (int i = 0; i < con->stages.size() && stage_num < 0; i++)
    if (m.ter(x, y) == con->stages[i].terrain && i + 1 < con->stages.size())
     stage_num = i + 1;
   if (player_can_build(u, total_inv, con, stage_num, true)) {
    valid.push_back(point(x, y));
    m.drawsq(this, w_terrain, x, y, true, false);
    wrefresh(w_terrain);
   }
  }
 }
 if (valid.empty()) {
  add_msg("There is no proper place here!");
  return;
 }
 mvprintz(0, 0, c_red, "Pick a direction in which to construct:");
 int dirx, diry;
 get_direction(this, dirx, diry, input());
 if (dirx == -2) {
  add_msg("Invalid direction.");
  return;
 }
 dirx += u.posx;
 diry += u.posy;
 bool point_is_okay = false;
 for (int i = 0; i < valid.size() && !point_is_okay; i++) {
  if (valid[i].x == dirx && valid[i].y == diry)
   point_is_okay = true;
 }
 if (!point_is_okay) {
  add_msg("You cannot build there!");
  return;
 }

// Figure out what stage to start at, and what stage is the maximum
 int starting_stage = 0, max_stage = 0;
 for (int i = 0; i < con->stages.size(); i++) {
  if (m.ter(dirx, diry) == con->stages[i].terrain)
   starting_stage = i + 1;
  if (player_can_build(u, total_inv, con, i, true))
   max_stage = i;
 }

 u.assign_activity(ACT_BUILD, con->stages[starting_stage].time * 1000, con->id);

 u.moves = 0;
 std::vector<int> stages;
 for (int i = starting_stage; i <= max_stage; i++)
  stages.push_back(i);
 u.activity.values = stages;
 u.activity.placement = point(dirx, diry);
}

void game::complete_construction()
{
 inventory map_inv;
 map_inv.form_from_map(this, point(u.posx, u.posy), PICKUP_RANGE);
 int stage_num = u.activity.values[0];
 constructable *built = constructions[u.activity.index];
 construction_stage stage = built->stages[stage_num];
 std::vector<component> player_use;
 std::vector<component> map_use;

 u.practice(sk_carpentry, stage.difficulty * 10);
 if (stage.difficulty < 1)
  u.practice(sk_carpentry, 10);
 for (int i = 0; i < 3; i++) {
  if (!stage.components[i].empty())
   consume_items(this, stage.components[i]);
 }
 
// Make the terrain change
 int terx = u.activity.placement.x, tery = u.activity.placement.y;
 if (stage.terrain != t_null)
  m.ter(terx, tery) = stage.terrain;

// Strip off the first stage in our list...
 u.activity.values.erase(u.activity.values.begin());
// ...and start the next one, if it exists
 if (u.activity.values.size() > 0) {
  construction_stage next = built->stages[u.activity.values[0]];
  u.activity.moves_left = next.time * 1000;
 } else // We're finished!
  u.activity.type = ACT_NULL;

// This comes after clearing the activity, in case the function interrupts
// activities
 construct effects;
 (effects.*(built->done))(this, point(terx, tery));
}

bool construct::able_empty(game *g, point p)
{
 return (g->m.move_cost(p.x, p.y) == 2);
}

bool construct::able_tree(game *g, point p)
{
 return (g->m.ter(p.x, p.y) == t_tree);
}

bool construct::able_log(game *g, point p)
{
 return (g->m.ter(p.x, p.y) == t_log);
}

bool construct::able_boardup(game *g, point p)
{
 return (g->m.ter(p.x, p.y) == t_door_c ||
         g->m.ter(p.x, p.y) == t_door_b ||
         g->m.ter(p.x, p.y) == t_door_o ||
         g->m.ter(p.x, p.y) == t_door_locked ||
         g->m.ter(p.x, p.y) == t_door_frame ||
         g->m.ter(p.x, p.y) == t_window ||
         g->m.ter(p.x, p.y) == t_window_empty ||
         g->m.ter(p.x, p.y) == t_window_frame);

}
/* unused
bool construct::able_window(game *g, point p)
{
 return (g->m.ter(p.x, p.y) == t_window_frame ||
         g->m.ter(p.x, p.y) == t_window_empty ||
         g->m.ter(p.x, p.y) == t_window);
}
*/
bool construct::able_window_pane(game *g, point p)
{
 return (g->m.ter(p.x, p.y) == t_window);
}

bool construct::able_broken_window(game *g, point p)
{
 return (g->m.ter(p.x, p.y) == t_window_frame);
}
/* unused
bool construct::able_door(game *g, point p)
{
 return (g->m.ter(p.x, p.y) == t_door_c ||
         g->m.ter(p.x, p.y) == t_door_b ||
         g->m.ter(p.x, p.y) == t_door_o ||
         g->m.ter(p.x, p.y) == t_door_locked);
}


bool construct::able_door_broken(game *g, point p)
{
 return (g->m.ter(p.x, p.y) == t_door_b);
}

*/
bool construct::able_wall(game *g, point p)
{
 return (g->m.ter(p.x, p.y) == t_wall_h || g->m.ter(p.x, p.y) == t_wall_v ||
         g->m.ter(p.x, p.y) == t_wall_wood);
}

bool construct::able_wall_wood(game *g, point p)
{
 return (g->m.ter(p.x, p.y) == t_wall_wood);
}

bool construct::able_between_walls(game *g, point p)
{
 bool fill[SEEX * MAPSIZE][SEEY * MAPSIZE];
 for (int x = 0; x < SEEX * MAPSIZE; x++) {
  for (int y = 0; y < SEEY * MAPSIZE; y++)
   fill[x][y] = false;
 }

 return (will_flood_stop(&(g->m), fill, p.x, p.y)); // See bottom of file
}

bool construct::able_dig(game *g, point p)
{
 return (g->m.has_flag(diggable, p.x, p.y) &&
         g->m.tr_at(p.x, p.y) == tr_null); // g->traps[g->m.tr_at(p.x, p.y)]
}

bool construct::able_pit(game *g, point p)
{
 return (g->m.ter(p.x, p.y) == t_pit ||
//         g->m.ter(p.x, p.y) == t_pit_shallow ||
         g->m.ter(p.x, p.y) == t_pit_spiked);
}

bool will_flood_stop(map *m, bool fill[SEEX * MAPSIZE][SEEY * MAPSIZE],
                     int x, int y)
{
 if (x == 0 || y == 0 || x == SEEX * MAPSIZE - 1 || y == SEEY * MAPSIZE - 1)
  return false;

 fill[x][y] = true;
 bool skip_north = (fill[x][y - 1] || m->has_flag(supports_roof, x, y - 1)),
      skip_south = (fill[x][y + 1] || m->has_flag(supports_roof, x, y + 1)),
      skip_east  = (fill[x + 1][y] || m->has_flag(supports_roof, x + 1, y)),
      skip_west  = (fill[x - 1][y] || m->has_flag(supports_roof, x - 1, y));

 return ((skip_north || will_flood_stop(m, fill, x    , y - 1)) &&
         (skip_east  || will_flood_stop(m, fill, x + 1, y    )) &&
         (skip_south || will_flood_stop(m, fill, x    , y + 1)) &&
         (skip_west  || will_flood_stop(m, fill, x - 1, y    ))   );
}

void construct::done_window_pane(game *g, point p)
{
 g->m.add_item(g->u.posx, g->u.posy, g->itypes[itm_glass_sheet], 0);
}

void construct::done_boardup(game *g, point p)
{
 switch (g->m.ter(p.x, p.y)) {
 case t_door_o:          //opened wood door
 case t_door_c:          //closed wood door
  g->m.ter(p.x, p.y) = t_door_boarded;
  break;
 case t_door_b:          //damaged wood door
  g->m.ter(p.x, p.y) = t_door_b_boarded;
  break;
 case t_door_locked:	//closed wood door
  g->m.ter(p.x, p.y) = t_door_locked_boarded;
  break;
 case t_door_frame:      // empty door frame
  g->m.ter(p.x, p.y) = t_door_frame_boarded;
  break;
 case t_window:          //window
  g->m.ter(p.x, p.y) = t_window_boarded;
  break;
 case t_window_empty:    //empty window
  g->m.ter(p.x, p.y) = t_window_empty_boarded;
  break;
 case t_window_frame:    //window frame (sharp)
  g->m.ter(p.x, p.y) = t_window_frame_boarded;
  break;
 }

}

void construct::done_tree(game *g, point p)
{
 mvprintz(0, 0, c_red, "Press a direction for the tree to fall in:");
 int x = 0, y = 0;
 do
  get_direction(g, x, y, input());
 while (x == -2 || y == -2);
 x = p.x + x * 3 + rng(-1, 1);
 y = p.y + y * 3 + rng(-1, 1);
 std::vector<point> tree = line_to(p.x, p.y, x, y, rng(1, 8));
 for (int i = 0; i < tree.size(); i++) {
  g->m.destroy(g, tree[i].x, tree[i].y, true);
  g->m.ter(tree[i].x, tree[i].y) = t_log;
 }
}

void construct::done_log(game *g, point p)
{
 int num_sticks = rng(10, 20);
 for (int i = 0; i < num_sticks; i++)
  g->m.add_item(p.x, p.y, g->itypes[itm_2x4], int(g->turn));
}

void construct::done_vehicle(game *g, point p)
{
    std::string name = string_input_popup(20, "Enter new vehicle name");
    vehicle *veh = g->m.add_vehicle (g, veh_custom, p.x, p.y, 270);
    if (!veh)
    {
        debugmsg ("error constructing vehicle");
        return;
    }
    veh->name = name;
    veh->install_part (0, 0, vp_frame_v2);
}
