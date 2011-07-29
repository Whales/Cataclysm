// Monster movement code; essentially, the AI

#include "monster.h"
#include "map.h"
#include "game.h"
#include "line.h"
#include "rng.h"
#include "pldata.h"
#include <stdlib.h>
#include <curses.h>

#ifndef SGN
#define SGN(a) (((a)<0) ? -1 : 1)
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

void monster::receive_moves()
{
 if (has_effect(ME_BEARTRAP))
  return;
 moves += speed;
}

bool monster::wander()
{
 return (plans.empty());
}

bool monster::can_move_to(map &m, int x, int y)
{
 if (m.move_cost(x, y) == 0 &&
     (!has_flag(MF_DESTROYS) || !m.is_destructable(x, y)) &&
     ((!has_flag(MF_AQUATIC) && !has_flag(MF_SWIMS)) ||
      !m.has_flag(swimmable, x, y)))
  return false;
 if (has_flag(MF_DIGS) && !m.has_flag(diggable, x, y))
  return false;
 if (has_flag(MF_AQUATIC) && !m.has_flag(swimmable, x, y))
  return false;
 return true;
}

// Resets plans (list of squares to visit) and builds it as a straight line
// to the destination (x,y). t is used to choose which eligable line to use.
// Currently, this assumes we can see (x,y), so shouldn't be used in any other
// circumstance (or else the monster will "phase" through solid terrain!)
void monster::set_dest(int x, int y, int &t)
{ 
 plans.clear();
// TODO: This causes a segfault, once in a blue moon!  Whyyyyy.
 plans = line_to(posx, posy, x, y, t);
}

// Move towards (x,y) for f more turns--generally if we hear a sound there
// "Stupid" movement; "if (wandx < posx) posx--;" etc.
void monster::wander_to(int x, int y, int f)
{
 wandx = x;
 wandy = y;
 wandf = f;
 if (has_flag(MF_GOODHEARING))
  wandf *= 6;
}

void monster::plan(game *g)
{
 int sightrange = g->light_level();
 int closest = -1;
 int dist = 1000;
 int tc, stc;
 if (friendly != 0) {	// Target monsters, not the player!
  for (int i = 0; i < g->z.size(); i++) {
   monster *tmp = &(g->z[i]);
   if (tmp->friendly == 0 && rl_dist(posx, posy, tmp->posx, tmp->posy) < dist &&
       g->m.sees(posx, posy, tmp->posx, tmp->posy, sightrange, tc)) {
    closest = i;
    dist = rl_dist(posx, posy, tmp->posx, tmp->posy);
    stc = tc;
   }
  }
  if (closest >= 0)
   set_dest(g->z[closest].posx, g->z[closest].posy, stc);
  else if (friendly > 0 && one_in(3))	// Grow restless with no targets
   friendly--;
  else if (friendly < 0 && g->sees_u(posx, posy, tc)) {
   if (rl_dist(posx, posy, g->u.posx, g->u.posy) > 2)
    set_dest(g->u.posx, g->u.posy, tc);
   else
    plans.clear();
  }
  return;
 }
 if (is_fleeing(g->u) && has_flag(MF_SEES) && g->sees_u(posx, posy, tc) &&
     (!g->u.has_trait(PF_ANIMALEMPATH) || !has_flag(MF_ANIMAL))) {
  wandx = posx * 2 - g->u.posx;
  wandy = posy * 2 - g->u.posy;
  wandf = 40;
 }
// If we can see, and we can see a character, start moving towards them
 if (!is_fleeing(g->u) && has_flag(MF_SEES)) {
  if (g->sees_u(posx, posy, tc)) {
   dist = rl_dist(posx, posy, g->u.posx, g->u.posy);
   closest = -2;
   stc = tc;
  }
  for (int i = 0; i < g->active_npc.size(); i++) {
   npc *me = &(g->active_npc[i]);
   if (rl_dist(posx, posy, me->posx, me->posy) < dist &&
       g->m.sees(posx, posy, me->posx, me->posy, sightrange, tc)) {
    dist = rl_dist(posx, posy, me->posx, me->posy);
    closest = i;
    stc = tc;
   }
  }
  for (int i = 0; i < g->z.size(); i++) {
   monster *mon = &(g->z[i]);
   if (mon->friendly != 0 && rl_dist(posx, posy, mon->posx, mon->posy) < dist &&
       g->m.sees(posx, posy, mon->posx, mon->posy, sightrange, tc)) {
    dist = rl_dist(posx, posy, mon->posx, mon->posy);
    closest = -3 - i;
    stc = tc;
   }
  }
  if (closest == -2)
   set_dest(g->u.posx, g->u.posy, stc);
  else if (closest <= -3)
   set_dest(g->z[-3 - closest].posx, g->z[-3 - closest].posy, stc);
  else if (closest >= 0)
   set_dest(g->active_npc[closest].posx, g->active_npc[closest].posy, stc);
 }
}
 
// General movement.
// Currently, priority goes:
// 1) Special Attack
// 2) Sight-based tracking
// 3) Scent-based tracking
// 4) Sound-based tracking
void monster::move(game *g)
{
// We decrement wandf no matter what.  We'll save our wander_to plans until
// after we finish out set_dest plans, UNLESS they time out first.
 if (wandf > 0)
  wandf--;

// First, use the special attack, if we can!
 if (sp_timeout > 0)
  sp_timeout--;
 if (sp_timeout == 0 && (friendly == 0 || has_flag(MF_FRIENDLY_SPECIAL))) {
  mattack ma;
  (ma.*type->sp_attack)(g, this);
 }
 if (moves < 0)
  return;
 if (has_flag(MF_IMMOBILE)) {
  moves = 0;
  return;
 }
 if (has_effect(ME_STUNNED)) {
  stumble(g, false);
  moves = 0;
  return;
 }
 if (friendly != 0 ||
     (g->u.has_trait(PF_ANIMALEMPATH) && has_flag(MF_ANIMAL))) {
  if (friendly > 0)
   friendly--;
  friendly_move(g);
  return;
 }
/*
 if (!is_fleeing(g->u) && has_flag(MF_ANIMAL)) {
  stumble(g, false);
  moves = 0;
  return;
 }
*/

 moves -= 100;
 int rn=0, hmove=0;
 bool moved=false;
 point next;
 rn = rng(0, 50);
 hmove = rng(0, 5);
 int mondex = (plans.size() > 0 ? g->mon_at(plans[0].x, plans[0].y) : -1);

 if (plans.size() > 0 &&
     (mondex == -1 || g->z[mondex].friendly != 0 || has_flag(MF_ATTACKMON)) &&
     (can_move_to(g->m, plans[0].x, plans[0].y) ||
      (plans[0].x == g->u.posx && plans[0].y == g->u.posy) || 
     (g->m.has_flag(bashable, plans[0].x, plans[0].y) && has_flag(MF_BASHES)))){
  // CONCRETE PLANS - Most likely based on sight
  next = plans[0];
  moved = true;
 } else if (has_flag(MF_SMELLS)) {
// No sight... or our plans are invalid (e.g. moving through a transparent, but
//  solid, square of terrain).  Fall back to smell if we have it.
  point tmp = scent_move(g);
  if (tmp.x != -1) {
   next = tmp;
   moved = true;
  }
 }
 if (wandf > 0 && !moved) { // No LOS, no scent, so as a fall-back follow sound
  point tmp = sound_move(g);
  if (tmp.x != posx || tmp.y != posy) {
   next = tmp;
   moved = true;
  }
 }

// Finished logic section.  By this point, we should have chosen a square to
//  move to (moved = true).
 if (moved) {	// Actual effects of moving to the square we've chosen
  mondex = g->mon_at(next.x, next.y);
  int npcdex = g->npc_at(next.x, next.y);
  if (next.x == g->u.posx && next.y == g->u.posy && type->melee_dice > 0)
   hit_player(g, g->u);
  else if (mondex != -1 && type->melee_dice > 0 &&
           (g->z[mondex].friendly != 0 || has_flag(MF_ATTACKMON)))
   hit_monster(g, mondex);
  else if (npcdex != -1 && type->melee_dice > 0)
   hit_player(g, g->active_npc[npcdex]);
  else if ((!can_move_to(g->m, next.x, next.y) || one_in(3)) &&
             g->m.has_flag(bashable, next.x, next.y) && has_flag(MF_BASHES)) {
   std::string bashsound = "NOBASH"; // If we hear "NOBASH" it's time to debug!
   int bashskill = int(type->melee_dice * type->melee_sides);
   g->m.bash(next.x, next.y, bashskill, bashsound);
   g->sound(next.x, next.y, 18, bashsound);
  } else if (g->m.move_cost(next.x, next.y) == 0 && has_flag(MF_DESTROYS)) {
   g->m.destroy(g, next.x, next.y, true);
   moves -= 250;
  } else if (can_move_to(g->m, next.x, next.y) && g->is_empty(next.x, next.y))
   move_to(g, next.x, next.y);
  else
   moves -= 100;
 }

// If we're close to our target, we get focused and don't stumble
 if ((has_flag(MF_STUMBLES) && (plans.size() > 3 || plans.size() == 0)) ||
     !moved)
  stumble(g, moved);
}

void monster::friendly_move(game *g)
{
 point next;
 bool moved = false;
 moves -= 100;
 if (plans.size() > 0 && (plans[0].x != g->u.posx || plans[0].y != g->u.posy) &&
     (can_move_to(g->m, plans[0].x, plans[0].y) ||
     (g->m.has_flag(bashable, plans[0].x, plans[0].y) && has_flag(MF_BASHES)))){
  next = plans[0];
  plans.erase(plans.begin());
  moved = true;
 } else
  stumble(g, moved);
 if (moved) {
  int mondex = g->mon_at(next.x, next.y);
  int npcdex = g->npc_at(next.x, next.y);
  if (mondex != -1 && g->z[mondex].friendly == 0 && type->melee_dice > 0)
   hit_monster(g, mondex);
  else if (npcdex != -1 && type->melee_dice > 0)
   hit_player(g, g->active_npc[g->npc_at(next.x, next.y)]);
  else if (mondex == -1 && npcdex == -1 && can_move_to(g->m, next.x, next.y))
   move_to(g, next.x, next.y);
  else if ((!can_move_to(g->m, next.x, next.y) || one_in(3)) &&
           g->m.has_flag(bashable, next.x, next.y) && has_flag(MF_BASHES)) {
   std::string bashsound = "NOBASH"; // If we hear "NOBASH" it's time to debug!
   int bashskill = int(type->melee_dice * type->melee_sides);
   g->m.bash(next.x, next.y, bashskill, bashsound);
   g->sound(next.x, next.y, 18, bashsound);
  } else if (g->m.move_cost(next.x, next.y) == 0 && has_flag(MF_DESTROYS)) {
   g->m.destroy(g, next.x, next.y, true);
   moves -= 250;
  }
 }
}

point monster::scent_move(game *g)
{
 plans.clear();
 std::vector<point> smoves;
 int maxsmell = 1; // Squares with smell 0 are not eligable targets
 int minsmell = 9999;
 point pbuff, next(-1, -1);
 unsigned int smell;
 for (int x = -1; x <= 1; x++) {
  for (int y = -1; y <= 1; y++) {
   smell = g->scent(posx + x, posy + y);
   if ((g->mon_at(posx + x, posy + y) == -1 || has_flag(MF_ATTACKMON)) &&
       (can_move_to(g->m, posx + x, posy + y) ||
        (posx + x == g->u.posx && posx + y == g->u.posy) ||
        (g->m.has_flag(bashable, posx + x, posy + y) && has_flag(MF_BASHES)))) {
    if ((!is_fleeing(g->u) && smell > maxsmell) ||
        ( is_fleeing(g->u) && smell < minsmell)   ) {
     smoves.clear();
     pbuff.x = posx + x;
     pbuff.y = posy + y;
     smoves.push_back(pbuff);
     maxsmell = smell;
     minsmell = smell;
    } else if ((!is_fleeing(g->u) && smell == maxsmell) ||
               ( is_fleeing(g->u) && smell == minsmell)   ) {
     pbuff.x = posx + x;
     pbuff.y = posy + y;
     smoves.push_back(pbuff);
    }
   }
  }
 }
 if (smoves.size() > 0) {
  int nextsq = rng(0, smoves.size() - 1);
  next = smoves[nextsq];
 }
 return next;
}

point monster::sound_move(game *g)
{
 plans.clear();
 point next;
 bool xbest = true;
 if (abs(wandy - posy) > abs(wandx - posx))// which is more important
  xbest = false;
 next.x = posx;
 next.y = posy;
 int x = posx, x2 = posx - 1, x3 = posx + 1;
 int y = posy, y2 = posy - 1, y3 = posy + 1;
 if (wandx < posx) { x--; x2++;          }
 if (wandx > posx) { x++; x2++; x3 -= 2; }
 if (wandy < posy) { y--; y2++;          }
 if (wandy > posy) { y++; y2++; y3 -= 2; }
 if (xbest) {
  if (can_move_to(g->m, x, y) || (x == g->u.posx && y == g->u.posy) ||
      (has_flag(MF_BASHES) && g->m.has_flag(bashable, x, y))) {
   next.x = x;
   next.y = y;
  } else if (can_move_to(g->m, x, y2) || (x == g->u.posx && y == g->u.posy) ||
             (has_flag(MF_BASHES) && g->m.has_flag(bashable, x, y2))) {
   next.x = x;
   next.y = y2;
  } else if (can_move_to(g->m, x2, y) || (x == g->u.posx && y == g->u.posy) ||
             (has_flag(MF_BASHES) && g->m.has_flag(bashable, x2, y))) {
   next.x = x2;
   next.y = y;
  } else if (can_move_to(g->m, x, y3) || (x == g->u.posx && y == g->u.posy) ||
             (has_flag(MF_BASHES) && g->m.has_flag(bashable, x, y3))) {
   next.x = x;
   next.y = y3;
  } else if (can_move_to(g->m, x3, y) || (x == g->u.posx && y == g->u.posy) ||
             (has_flag(MF_BASHES) && g->m.has_flag(bashable, x3, y))) {
   next.x = x3;
   next.y = y;
  }
 } else {
  if (can_move_to(g->m, x, y) || (x == g->u.posx && y == g->u.posy) ||
      (has_flag(MF_BASHES) && g->m.has_flag(bashable, x, y))) {
   next.x = x;
   next.y = y;
  } else if (can_move_to(g->m, x2, y) || (x == g->u.posx && y == g->u.posy) ||
             (has_flag(MF_BASHES) && g->m.has_flag(bashable, x2, y))) {
   next.x = x2;
   next.y = y;
  } else if (can_move_to(g->m, x, y2) || (x == g->u.posx && y == g->u.posy) ||
             (has_flag(MF_BASHES) && g->m.has_flag(bashable, x, y2))) {
   next.x = x;
   next.y = y2;
  } else if (can_move_to(g->m, x3, y) || (x == g->u.posx && y == g->u.posy) ||
             (has_flag(MF_BASHES) && g->m.has_flag(bashable, x3, y))) {
   next.x = x3;
   next.y = y;
  } else if (can_move_to(g->m, x, y3) || (x == g->u.posx && y == g->u.posy) ||
             (has_flag(MF_BASHES) && g->m.has_flag(bashable, x, y3))) {
   next.x = x;
   next.y = y3;
  }
 }
 return next;
}

void monster::hit_player(game *g, player &p)
{
 if (type->melee_dice == 0) // We don't attack, so just return
  return;
 bool is_npc = p.is_npc();
 int  junk;
 bool u_see = (!is_npc || g->u_see(p.posx, p.posy, junk));
 std::string you  = (is_npc ? p.name : "you");
 std::string your = (is_npc ? p.name + "'s" : "your");
 std::string Your = (is_npc ? p.name + "'s" : "Your");
 body_part bphit;
 int side = rng(0, 1);
 int dam = hit(p, bphit);
 if (dam == 0 && u_see)
  g->add_msg("The %s misses %s.", name().c_str(), you.c_str());
 else if (dam > 0) {
  if (u_see)
   g->add_msg("The %s hits %s %s.", name().c_str(), your.c_str(),
              body_part_name(bphit, side).c_str());
  if (!is_npc) {
   if (g->u.activity.type == ACT_RELOAD)
    g->add_msg("You stop reloading.");
   else if (g->u.activity.type == ACT_READ)
    g->add_msg("You stop reading.");
   else if (g->u.activity.type == ACT_CRAFT)
    g->add_msg("You stop crafting.");
   g->u.activity.type = ACT_NULL;
  }
  if (p.has_active_bionic(bio_ods)) {
   if (u_see)
    g->add_msg("%s offensive defense system shocks it!", Your.c_str());
   hurt(rng(10, 40));
  }
  if (p.encumb(bphit) == 0 &&
      (p.has_trait(PF_SPINES) || p.has_trait(PF_QUILLS))) {
   int spine = rng(1, (p.has_trait(PF_QUILLS) ? 20 : 8));
   g->add_msg("%s %s puncture it!", Your.c_str(),
              (g->u.has_trait(PF_QUILLS) ? "quills" : "spines"));
   hurt(spine);
  }
  p.hit(g, bphit, side, dam, type->melee_cut);
  if (has_flag(MF_VENOM)) {
   if (!is_npc)
    g->add_msg("You're poisoned!");
   p.add_disease(DI_POISON, 30, g);
  }
 }
 if (is_npc) {
  if (p.hp_cur[hp_head] <= 0 || p.hp_cur[hp_torso] <= 0) {
   npc* tmp = dynamic_cast<npc*>(&p);
   tmp->die(g);
   int index = g->npc_at(p.posx, p.posy);
   g->active_npc.erase(g->active_npc.begin() + index);
   plans.clear();
  }
 }
}

void monster::move_to(game *g, int x, int y)
{
 int mondex = g->mon_at(x, y);
 if (mondex == -1) { //...assuming there's no monster there
  if (plans.size() > 0)
   plans.erase(plans.begin());
  if (has_flag(MF_SWIMS) && g->m.has_flag(swimmable, x, y))
   moves += 50;
  if (!has_flag(MF_DIGS) && !has_flag(MF_FLIES) &&
      (!has_flag(MF_SWIMS) || !g->m.has_flag(swimmable, x, y)))
   moves -= (g->m.move_cost(x, y) - 2) * 50;
  posx = x;
  posy = y;
  if (g->m.tr_at(posx, posy) != tr_null) { // Monster stepped on a trap!
   trap* tr = g->traps[g->m.tr_at(posx, posy)];
   if (dice(3, sk_dodge + 1) < dice(3, tr->avoidance)) {
    trapfuncm f;
    (f.*(tr->actm))(g, this, posx, posy);
   }
  }
// Diggers turn the dirt into dirtmound
  if (has_flag(MF_DIGS))
   g->m.ter(posx, posy) = t_dirtmound;
// Acid trail monsters leave... a trail of acid
  if (has_flag(MF_ACIDTRAIL))
   g->m.add_field(g, posx, posy, fd_acid, 1);
 } else if (has_flag(MF_ATTACKMON) || g->z[mondex].friendly != 0)
// If there IS a monster there, and we fight monsters, fight it!
  hit_monster(g, mondex);
}

/* Random walking even when we've moved
 * To simulate zombie stumbling and ineffective movement
 * Note that this is sub-optimal; stumbling may INCREASE a zombie's speed.
 * Most of the time (out in the open) this effect is insignificant compared to
 * the negative effects, but in a hallway it's perfectly even
 */
void monster::stumble(game *g, bool moved)
{
 std::vector <point> valid_stumbles;
 for (int i = -1; i <= 1; i++) {
  for (int j = -1; j <= 1; j++) {
   if (can_move_to(g->m, posx + i, posy + j) &&
       (g->u.posx != posx + i || g->u.posy != posy + j) && 
       (g->mon_at(posx + i, posy + j) == -1 || (i == 0 && j == 0))) {
    point tmp(posx + i, posy + j);
    valid_stumbles.push_back(tmp);
   }
  }
 }
 if (valid_stumbles.size() > 0 && (one_in(8) || (!moved && one_in(3)))) {
  int choice = rng(0, valid_stumbles.size() - 1);
  posx = valid_stumbles[choice].x;
  posy = valid_stumbles[choice].y;
  if (!has_flag(MF_DIGS) || !has_flag(MF_FLIES))
   moves -= (g->m.move_cost(posx, posy) - 2) * 50;
// Here we have to fix our plans[] list, trying to get back to the last point
// Otherwise the stumble will basically have no effect!
  if (plans.size() > 0) {
   int tc;
   if (g->m.sees(posx, posy, plans[0].x, plans[0].y, -1, tc)) {
// Copy out old plans...
    std::vector <point> plans2;
    for (int i = 0; i < plans.size(); i++)
     plans2.push_back(plans[i]);
// Set plans to a route between where we are now, and where we were
    set_dest(plans[0].x, plans[0].y, tc);
// Append old plans to the new plans
    for (int index = 0; index < plans2.size(); index++)
     plans.push_back(plans2[index]);
   } else
    plans.clear();
  }
 }
}
