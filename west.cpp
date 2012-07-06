#include "gamemode.h"
#include "game.h"
#include "setvector.h"
#include "keypress.h"
#include "itype.h"
#include "mtype.h"
#include <string>
#include <vector>
#include <sstream>

bool west_game::init (game *g)
{
  if(!g->u.create(g, PLTYPE_CUSTOM)) 
    return false;

  if (g->load(g->u.name)) {
    if(g->u.scent == 500)
      horde_location = g->global_location().x - 25;
    else {
     horde_location = g->u.scent - 500;
     g->u.scent = 500;
    }

    g->add_msg("The horde is %d map squares away.", distance_to_horde(g));
    popup_top("The horde comes.");
    
    return true;
  }

  // g->start_game();
  g->turn = MINUTES(STARTING_MINUTES + 60);// It's turn 0...
  // run_mode = 1;	// run_mode is on by default...
  // g->mostseen = 0;	// ...and mostseen is 0, we haven't seen any monsters yet.
  
  // Init some factions.
  // if (!g->load_master())	// Master data record contains factions.
  //  g->create_factions();
  g->cur_om = overmap(g, 0, 0, 0);	// We start in the (0,0,0) overmap.
  // Find a random house on the map, and set us there.
  g->cur_om.first_house(g->levx, g->levy);
  g->levx -= int(int(MAPSIZE / 2) / 2);
  g->levy -= int(int(MAPSIZE / 2) / 2);
  g->levz = 0;
  // Start the overmap out with none of it seen by the player...
  for (int i = 0; i < OMAPX; i++) {
    for (int j = 0; j < OMAPX; j++)
      g->cur_om.seen(i, j) = false;
  }
  // ...except for our immediate neighborhood.
  for (int i = -15; i <= 15; i++) {
    for (int j = -15; j <= 15; j++)
      g->cur_om.seen(g->levx + i, g->levy + j) = true;
  }
  // Convert the overmap coordinates to submap coordinates
  g->levx = g->levx * 2 - 1;
  g->levy = g->levy * 2 - 1;
  g->set_adjacent_overmaps(true);
  // Init the starting map at this location.
  g->m.load(g, g->levx, g->levy);
  // Start us off somewhere in the shelter.
  g->u.posx = SEEX * int(MAPSIZE / 2) + 5;
  g->u.posy = SEEY * int(MAPSIZE / 2) + 5;
  g->u.str_cur = g->u.str_max;
  g->u.per_cur = g->u.per_max;
  g->u.int_cur = g->u.int_max;
  g->u.dex_cur = g->u.dex_max;
  // g->nextspawn = int(g->turn);	
  g->temperature = 65; // Springtime-appropriate?
  
  g->u.normalize(g);
  g->u.weapon = item(g->itypes[itm_baton], 0, 'a' + g->u.worn.size());

  horde_location = g->global_location().x - 25;

  return true;
}

int west_game::distance_to_horde(game *g) {
  return g->global_location().x - horde_location;
}

void west_game::spam_zombies (game * g, int n, bool fast_only) {
 if (n <= 0) return;
 g->cancel_activity();
 g->u.rem_disease(DI_SLEEP);
 g->u.rem_disease(DI_LYING_DOWN);
 int monx, mony;
 for (int i = 0; i < n; i++) {
  mon_id type = fast_only ? mon_zombie_fast : g->valid_monster_from(g->moncats[mcat_zombie]);
  if (type != mon_null) {
   monx = rng(0, SEEX * MAPSIZE - 1);
   mony = rng(0, SEEY * MAPSIZE - 1);
   monster zom = monster(g->mtypes[type]);
   zom.spawn(monx, mony);
   zom.wandx = g->u.posx;
   zom.wandy = g->u.posy;
   zom.moves = -100;
   g->z.push_back(zom);
  }
 }
}


void west_game::per_turn(game *g)
{
 //  auto spam_zombies = [&] (int n = 200, bool fast_only = false) {
 //    if (n <= 0) return;
 //    g->cancel_activity();
 //    g->u.rem_disease(DI_SLEEP);
 //    g->u.rem_disease(DI_LYING_DOWN);
 //    int monx, mony;
 //    for (int i = 0; i < n; i++) {
 //      mon_id type = fast_only ? mon_zombie_fast : g->valid_monster_from(g->moncats[mcat_zombie]);
 //      if (type != mon_null) {
	// monx = rng(0, SEEX * MAPSIZE - 1);
	// mony = rng(0, SEEY * MAPSIZE - 1);
	// monster zom = monster(g->mtypes[type]);
	// zom.spawn(monx, mony);
	// zom.wandx = g->u.posx;
	// zom.wandy = g->u.posy;
	// zom.moves = -100;
	// g->z.push_back(zom);
 //      }
 //    }
 //  };

  if (int(g->turn) % 300 == 0) {
    horde_location++;
    int dh = distance_to_horde(g);
    if (dh <= 0) {
      popup("The horde is upon you!!");
      spam_zombies(g);
    }
    else if (dh <= 5) {
      popup("The horde is starting to catch up with you!\nThe main body of the horde is only %d map squares away!", dh);
      int badness = 5 - dh;
      spam_zombies(g, dice(badness,4), true);
    }
    else if (dh < 15) {
      popup("The horde comes!  They are only %d map squares away.", dh);
      g->cancel_activity_query("The horde comes!");
      g->u.rem_disease(DI_SLEEP);
      g->u.rem_disease(DI_LYING_DOWN);
    }
    else if (dh < 30) {
      // if(g->u.has_disease(DI_SLEEP) || g->u.has_disease(DI_LYING_DOWN))
	g->add_msg("The distant horde approaches.");
 //      else
	// popup("The distant horde approaches.");
    }
    else {
      // if(g->u.has_disease(DI_SLEEP) || g->u.has_disease(DI_LYING_DOWN))
	g->add_msg("In the far distance, the horde moves.");
 //      else
	// popup("In the far distance, the horde moves ever forward.");
    }   
  }
}

void west_game::pre_action(game *g, action_id &act)
{
 if (act == ACTION_SAVE) {
   g->u.scent = 500 + horde_location;
  }
}
