#include "player.h"
#include "bionics.h"
#include "game.h"
#include "keypress.h"
#include <sstream>
#include <stdlib.h>

#if (defined _WIN32 || defined WINDOWS)
	#include "catacurse.h"
#else
	#include <curses.h>
#endif


/* Melee Functions!
 * These all belong to class player.
 *
 * STATE QUERIES
 * bool is_armed() - True if we are armed with any weapon.
 * bool unarmed_attack() - True if we are NOT armed with any weapon, but still
 *  true if we're wielding a bionic weapon (at this point, just itm_bio_claws).
 *
 * HIT DETERMINATION
 * int base_to_hit() - The base number of sides we get in hit_roll().
 *                     Dexterity / 2 + sk_melee
 * int hit_roll() - The player's hit roll, to be compared to a monster's or
 *   player's dodge_roll().  This handles weapon bonuses, weapon-specific
 *   skills, torso encumberment penalties and drunken master bonuses.
 */

bool player::is_armed()
{
 return (weapon.type->id != 0);
}

bool player::unarmed_attack()
{
 return (weapon.type->id == 0 || weapon.type->id == itm_bio_claws);
}


int player::base_to_hit(bool real_life)
{
 int dex = (real_life ? dex_cur : dex_max);
 return 1 + int(dex / 2) + sklevel[sk_melee];
}

int player::hit_roll()
{
 int numdice = base_to_hit() + weapon.type->m_to_hit;
 int sides = 10 - encumb(bp_torso);
 int best_bonus = 0;
 if (sides < 2)
  sides = 2;

// Are we unarmed?
 if (unarmed_attack()) {
  best_bonus = sklevel[sk_unarmed];
  if (sklevel[sk_unarmed] > 4)
   best_bonus += sklevel[sk_unarmed] - 4; // Extra bonus for high levels
 }

// Using a bashing weapon?
 if (weapon.is_bashing_weapon()) {
  int bash_bonus = int(sklevel[sk_bashing] / 3);
  if (bash_bonus > best_bonus)
   best_bonus = bash_bonus;
 }

// Using a cutting weapon?
 if (weapon.is_cutting_weapon()) {
  int cut_bonus = int(sklevel[sk_cutting] / 2);
  if (cut_bonus > best_bonus)
   best_bonus = cut_bonus;
 }

// Using a spear?
 if (weapon.has_flag(IF_SPEAR) || weapon.has_flag(IF_STAB)) {
  int stab_bonus = int(sklevel[sk_stabbing] / 2);
  if (stab_bonus > best_bonus)
   best_bonus = stab_bonus;
 }

 numdice += best_bonus;
 if (has_trait(PF_DRUNKEN)) {
  if (unarmed_attack())
   numdice += rng(0, 1) + int(disease_level(DI_DRUNK) / 300);
  else
   numdice += int(disease_level(DI_DRUNK) / 400);
 }

 if (numdice < 1) {
  numdice = 1;
  sides = 8;
 }

 return dice(numdice, sides);
}


int player::hit_mon(game *g, monster *z)
{
 bool is_u = (this == &(g->u));	// Affects how we'll display messages
 if (is_u)
  z->add_effect(ME_HIT_BY_PLAYER, 100); // Flag as attacked by us
 int j;
 bool can_see = (is_u || g->u_see(posx, posy, j));
 std::string You  = (is_u ? "You"  : name);
 std::string Your = (is_u ? "Your" : name + "'s");
 std::string your = (is_u ? "your" : (male ? "his" : "her"));

// Types of combat (may overlap!)
 bool unarmed  = unarmed_attack(),
      bashing  = weapon.is_bashing_weapon(),
      cutting  = weapon.is_cutting_weapon(),
      stabbing = (weapon.has_flag(IF_SPEAR) ||
                  weapon.has_flag(IF_STAB)    );

 bool can_poison = false;

 bool conductive = (weapon.conductive() && !wearing_something_on(bp_hands));

// Recoil penalty
 if (recoil <= 30)
  recoil += 6;
// Movement cost
 int move_cost = weapon.attack_time() + 20 * encumb(bp_torso);
 if (has_trait(PF_LIGHT_BONES))
  move_cost *= .9;
 if (has_trait(PF_HOLLOW_BONES))
  move_cost *= .8;
 moves -= move_cost;
// Different sizes affect your chance to hit
 if (hit_roll() < z->dodge_roll() ||
     one_in(4 + dex_cur + weapon.type->m_to_hit)) {// A miss!
  stumble(g);
  return 0;
 }
// For very high hit rolls, we crit!
 bool critical_hit = scored_crit(z->dodge_roll());
 int dam = base_damage(true);
 int cutting_penalty = 0; // Moves lost from getting a cutting weapon stuck

// Drunken Master damage bonuses
 if (has_trait(PF_DRUNKEN) && has_disease(DI_DRUNK)) {
// Remember, a single drink gives 600 levels of DI_DRUNK
  int mindrunk, maxdrunk;
  if (unarmed) {
   mindrunk = disease_level(DI_DRUNK) / 600;
   maxdrunk = disease_level(DI_DRUNK) / 250;
  } else {
   mindrunk = disease_level(DI_DRUNK) / 900;
   maxdrunk = disease_level(DI_DRUNK) / 400;
  }
  dam += rng(mindrunk, maxdrunk);
 }

 if (unarmed) { // Unarmed bonuses
  dam += rng(0, sklevel[sk_unarmed]);
  if (has_trait(PF_NAILS) && z->armor_cut() == 0 &&
      !wearing_something_on(bp_hands)) {
   dam++;
   if (one_in(2))
    can_poison = true;
  }
  if (has_trait(PF_CLAWS) && z->armor_cut() < 6 &&
      !wearing_something_on(bp_hands)) {
   dam += 6;
   if (one_in(2))
    can_poison = true;
  }
  if (has_trait(PF_TALONS) && z->armor_cut() - sklevel[sk_unarmed] < 10) {
   int z_armor = (z->armor_cut() - sklevel[sk_unarmed]);
   if (z_armor < 0)
    z_armor = 0;
   dam += 10 - z_armor;
   if (one_in(2))
    can_poison = true;
  }
  if (has_trait(PF_THORNS) && z->armor_cut() < 4 &&
      !wearing_something_on(bp_hands)) {
   dam += 4 - z->armor_cut();
   if (one_in(2))
    can_poison = true;
  }
  if (has_trait(PF_SLIME_HANDS) && !z->has_flag(MF_ACIDPROOF) &&
      !wearing_something_on(bp_hands)) {
   dam += rng(4, 6);
   can_poison = true;
  }
 }

 if (rng(1, 45 - dex_cur) < 2 * sklevel[sk_unarmed] &&
     rng(1, 65 - dex_cur) < 2 * sklevel[sk_unarmed]   ) {
// Bonus unarmed attack!
  if (is_u || can_see) {
   switch (rng(1, 2)) {
    case 1: g->add_msg("%s elbow%s the %s!", You.c_str(), (is_u ? "" : "s"),
                       z->name().c_str()); break;
    case 2: g->add_msg("%s knee%s the %s!", You.c_str(), (is_u ? "" : "s"),
                       z->name().c_str()); break;
   }
  }
  if (sklevel[sk_unarmed] >= 4)
   dam += rng(1, sklevel[sk_unarmed] / 2);
  else
   dam++;
  practice(sk_unarmed, 2);
 }
// Melee skill bonus
 dam += rng(0, sklevel[sk_melee]);
// Bashing damage bonus
 int bash_dam = weapon.damage_bash() - z->armor_bash(),
     bash_cap = 5 + str_cur + sklevel[sk_bashing];
 if (bash_dam > bash_cap)// Cap for weak characters
  bash_dam = (bash_cap * 3 + bash_dam) / 4;
 if (bashing)
  bash_dam += rng(0, sklevel[sk_bashing] + sqrt(double(str_cur)));
 if (z->has_flag(MF_PLASTIC))
  bash_dam /= rng(2, 4);
 int bash_min = bash_dam / 4;
 if (bash_min < sklevel[sk_bashing] )
  bash_min = sklevel[sk_bashing];
 dam += rng(bash_min, bash_dam);
// Take some moves away from the target; at this point it's skill & bash damage
 z->moves -= rng(0, dam * 2);

// Spears treat cutting damage specially.
 if (weapon.has_flag(IF_SPEAR) &&
     weapon.damage_cut() > z->armor_cut() - 2 * sklevel[sk_stabbing]) {
  int z_armor = z->armor_cut() - 2 * sklevel[sk_stabbing];
  dam += int((weapon.damage_cut() - z_armor) / 5);
  if (z->speed > 100) // Bonus against fast monsters
   dam += rng( int((z->speed - 100) / 10), int((z->speed - 100) / 5));
  int minstab = sklevel[sk_stabbing] *  5 + weapon.volume() * 2,
      maxstab = sklevel[sk_stabbing] * 15 + weapon.volume() * 4;
  int monster_penalty = rng(minstab, maxstab);
  if (monster_penalty >= 150)
   g->add_msg("You force the %s to the ground!", z->name().c_str());
  else if (monster_penalty >= 50)
   g->add_msg("The %s is skewered and flinches!", z->name().c_str());
  z->moves -= monster_penalty;
  cutting_penalty = weapon.damage_cut() * 4 + z_armor * 8 -
                    dice(sklevel[sk_stabbing], 10);
  practice(sk_stabbing, 2);

// Cutting damage bonus
 } else if ((weapon.damage_cut() >
             z->armor_cut() - int(sklevel[sk_cutting] / 2)) ||
            (stabbing && 
             weapon.damage_cut() > z->armor_cut() - 2 * sklevel[sk_stabbing])) {

  int z_armor_cut = z->armor_cut() - int(sklevel[sk_cutting] / 2);
  if (z_armor_cut < 0)
   z_armor_cut = 0;
  int z_armor_stab = z->armor_cut() - 2 * sklevel[sk_stabbing];
  if (z_armor_stab < 0)
   z_armor_stab = 0;
// Check cut dam vs. stab dam and automatically pick one
  int cutdam = weapon.damage_cut() - z_armor_cut;
  int stabdam = int((weapon.damage_cut() - z_armor_stab) / 5);
  if (z->speed > 100)
   stabdam += rng( int((z->speed - 100) / 10), int((z->speed - 100) / 5));
  if (cutdam > stabdam || !stabbing) {
   dam += cutdam;
   cutting_penalty = weapon.damage_cut() * 3 + z_armor_cut * 8 -
                     dice(sklevel[sk_cutting], 10);
  } else {
   dam += stabdam;
   cutting_penalty = weapon.damage_cut() * 3 + z_armor_stab * 8 -
                     dice(sklevel[sk_cutting], 10);
  }
 }

 if (weapon.has_flag(IF_MESSY)) { // e.g. chainsaws
  cutting_penalty /= 6; // Harder to get stuck
  for (int x = z->posx - 1; x <= z->posx + 1; x++) {
   for (int y = z->posy - 1; y <= z->posy + 1; y++) {
    if (!one_in(3)) {
     if (g->m.field_at(x, y).type == fd_blood &&
         g->m.field_at(x, y).density < 3)
      g->m.field_at(x, y).density++;
     else
      g->m.add_field(g, x, y, fd_blood, 1);
    }
   }
  }
 }


// Critical hit effects
 if (critical_hit) {
  bool headshot = (!z->has_flag(MF_NOHEAD) && !one_in(3));

  if (weapon.has_flag(IF_SPEAR) || weapon.has_flag(IF_STAB)) {
   dam += weapon.damage_cut();
   dam += weapon.damage_cut() * double(sklevel[sk_stabbing] / 10);
   practice(sk_stabbing, 5);
  }

  if (unarmed) {
   dam += rng(1, 4) * sklevel[sk_unarmed];
   z->moves -= dam;	// Stunning blow

   if (weapon.type->id == itm_bio_claws) {
    if (sklevel[sk_cutting] >= 3)
     dam += 5;
    headshot &= z->hp < dam && one_in(2);
    if (headshot && can_see)
     g->add_msg("%s claws pierce the %s's skull!", Your.c_str(),
                z->name().c_str());
    else if (can_see)
     g->add_msg("%s claws stab straight through the %s!", Your.c_str(),
                z->name().c_str());
   } else if (has_trait(PF_TALONS)) {
    dam += 2;
    headshot &= z->hp < dam && one_in(2);
    if (headshot && can_see)
     g->add_msg("%s talons tear the %s's head open!", Your.c_str(),
                z->name().c_str());
    else if (can_see)
     g->add_msg("%s bur%s %s talons into the %s!", You.c_str(),(is_u?"y":"ies"),
                your.c_str(), z->name().c_str());
   } else {
    headshot &= z->hp < dam && one_in(2);
    if (headshot && can_see)
     g->add_msg("%s crush%s the %s's skull in a single blow!", 
                You.c_str(), (is_u ? "" : "es"), z->name().c_str());
    else if (can_see)
     g->add_msg("%s deliver%s a crushing punch!",You.c_str(),(is_u ? "" : "s"));
   }
   if (z->hp > 0 && rng(1, 5) < sklevel[sk_unarmed])
    z->add_effect(ME_STUNNED, 1 + sklevel[sk_unarmed]);

  } else {	// Not unarmed

   if (bashing) {
    dam += (str_cur / 2);
    int turns_stunned = int(dam / 20) + rng(0, int(sklevel[sk_bashing] / 2));
    if (turns_stunned > 6)
     turns_stunned = 6;
    z->add_effect(ME_STUNNED, turns_stunned);
   }
   if (cutting || stabbing) {
    double cut_multiplier;
    if (cutting)
     cut_multiplier = double(sklevel[sk_cutting]  / 12);
    else
     cut_multiplier = double(sklevel[sk_stabbing] /  5);
    if (cut_multiplier > 1.5)
     cut_multiplier = 1.5;
    dam += cut_multiplier * weapon.damage_cut();
    headshot &= z->hp < dam;

    if (stabbing) {
     if (headshot && can_see)
      g->add_msg("%s %s stabs through the %s's skull!", Your.c_str(),
                 weapon.tname(g).c_str(), z->name().c_str());
     else if (can_see)
      g->add_msg("%s stab %s %s through the %s!", You.c_str(), your.c_str(),
                 weapon.tname(g).c_str(), z->name().c_str());
    } else {
     if (headshot && can_see)
      g->add_msg("%s %s slices the %s's head off!", Your.c_str(),
                 weapon.tname(g).c_str(), z->name().c_str());
     else
      g->add_msg("%s %s cuts the %s deeply!", Your.c_str(),
                 weapon.tname(g).c_str(), z->name().c_str());
    }
   } else if (bashing) {
    headshot &= z->hp < dam;
    if (headshot && can_see)
     g->add_msg("%s crush%s the %s's skull!", You.c_str(), (is_u ? "" : "es"),
                z->name().c_str());
    else if (can_see)
     g->add_msg("%s crush%s the %s's body!", You.c_str(), (is_u ? "" : "es"),
                z->name().c_str());
   }
  }	// End of not-unarmed
 }	// End of critical hit

// Bonus attacks!
 bool shock_them = (has_bionic(bio_shock) && power_level >= 2 && unarmed &&
                    !z->has_flag(MF_ELECTRIC) && one_in(3));
 bool drain_them = (has_bionic(bio_heat_absorb) && power_level >= 1 &&
                    !is_armed() && z->has_flag(MF_WARM));
 if (drain_them)
  power_level--;
 drain_them &= one_in(2);	// Only works half the time

 std::vector<special_attack> special_attacks = mutation_attacks(z);

 if (shock_them) {
  power_level -= 2;
  if (can_see)
   g->add_msg("%s shock%s the %s!", You.c_str(), (is_u ? "" : "s"),
              z->name().c_str());
  int shock = rng(2, 5);
  dam += shock * rng(1, 3);
  z->moves -= shock * 180;
 }
 if (drain_them) {
  charge_power(rng(0, 4));
  if (can_see)
   g->add_msg("%s drain%s the %s's body heat!", You.c_str(), (is_u ? "" : "s"),
              z->name().c_str());
  dam += rng(4, 10);
  z->moves -= rng(80, 120);
 }

 for (int i = 0; i < special_attacks.size(); i++) {
  int spec_dam = 0;
  spec_dam += special_attacks[i].bash;
  if (special_attacks[i].cut > z->armor_cut())
   spec_dam += special_attacks[i].cut - z->armor_cut();
  if (special_attacks[i].stab > z->armor_cut() * .8)
   spec_dam += special_attacks[i].stab - z->armor_cut() * .8;

  if (!can_poison && one_in(2) &&
      (special_attacks[i].cut > z->armor_cut() ||
       special_attacks[i].stab > z->armor_cut() * .8))
   can_poison = true;

  if (spec_dam > 0) {
   g->add_msg( special_attacks[i].text.c_str() );
   dam += spec_dam;
  }
 }

 if (can_poison && has_trait(PF_POISONOUS)) {
  z->add_effect(ME_POISONED, 6);
  if (is_u)
   g->add_msg("You poison the %s!", z->name().c_str());
 }

 if (z->has_flag(MF_ELECTRIC) && conductive) {
  hurtall(rng(0, 1));
  moves -= rng(0, 50);
  if (is_u)
   g->add_msg("Contact with the %s shocks you!", z->name().c_str());
 }

// Make a rather quiet sound, to alert any nearby monsters
 g->sound(posx, posy, 8, "");

// Glass weapons shatter sometimes
 if (weapon.made_of(GLASS) &&
     rng(0, weapon.volume() + 8) < weapon.volume() + str_cur) {
  if (can_see)
   g->add_msg("%s %s shatters!", Your.c_str(), weapon.tname(g).c_str());
  g->sound(posx, posy, 16, "");
// Dump its contents on the ground
  for (int i = 0; i < weapon.contents.size(); i++)
   g->m.add_item(posx, posy, weapon.contents[i]);
  hit(g, bp_arms, 1, 0, rng(0, weapon.volume() * 2));// Take damage
  if (weapon.is_two_handed(this))// Hurt left arm too, if it was big
   hit(g, bp_arms, 0, 0, rng(0, weapon.volume()));
  dam += rng(0, 5 + int(weapon.volume() * 1.5));// Hurt the monster extra
  remove_weapon();
 }

 if (dam <= 0) {
  if (is_u)
   g->add_msg("You hit the %s, but do no damage.", z->name().c_str());
  else if (can_see)
   g->add_msg("%s's %s hits the %s, but does no damage.", You.c_str(),
              weapon.tname(g).c_str(), z->name().c_str());
  practice(sk_melee, rng(2, 5));
  if (unarmed)
   practice(sk_unarmed, 2);
  if (bashing)
   practice(sk_bashing, 2);
  if (cutting)
   practice(sk_cutting, 2);
  if (stabbing)
   practice(sk_stabbing, 2);
  return 0;
 }
 if (is_u)
  g->add_msg("You hit the %s for %d damage.", z->name().c_str(), dam);
 else if (can_see)
  g->add_msg("%s hits the %s with %s %s.", You.c_str(), z->name().c_str(),
             (male ? "his" : "her"),
             (weapon.type->id == 0 ? "fists" : weapon.tname(g).c_str()));
 practice(sk_melee, rng(5, 10));
 if (unarmed)
  practice(sk_unarmed, rng(5, 10));
 if (bashing)
  practice(sk_bashing, rng(5, 10));
 if (cutting)
  practice(sk_cutting, rng(5, 10));
 if (stabbing)
  practice(sk_stabbing, rng(5, 10));

// Penalize the player if their cutting weapon got stuck
 if (!unarmed && dam < z->hp && cutting_penalty > dice(str_cur * 2, 20)) {
  if (is_u)
   g->add_msg("Your %s gets stuck in the %s, pulling it out of your hands!",
              weapon.tname().c_str(), z->type->name.c_str());
  z->add_item(remove_weapon());
  if (weapon.has_flag(IF_SPEAR) || weapon.has_flag(IF_STAB))
   z->speed *= .7;
  else
   z->speed *= .85;
 } else {
  if (dam >= z->hp) {
   cutting_penalty /= 2;
   cutting_penalty -= rng(sklevel[sk_cutting], sklevel[sk_cutting] * 2 + 2);
  }
  if (cutting_penalty > 0)
   moves -= cutting_penalty;
  if (cutting_penalty >= 50 && is_u)
   g->add_msg("Your %s gets stuck in the %s, but you yank it free.",
              weapon.tname().c_str(), z->type->name.c_str());
  if (weapon.has_flag(IF_SPEAR) || weapon.has_flag(IF_STAB))
   z->speed *= .9;
 }

 return dam;
}

void player::stumble(game *g)
{
 int stumble_pen = 2 * weapon.volume() + weapon.weight();
 if (has_trait(PF_DEFT))
  stumble_pen = int(stumble_pen * .3) - 10;
 if (stumble_pen < 0)
  stumble_pen = 0;
 if (stumble_pen > 0 && (str_cur >= 15 || dex_cur >= 21 ||
                         one_in(16 - str_cur) || one_in(22 - dex_cur)))
  stumble_pen = rng(0, stumble_pen);
 if (!is_npc()) {	// Only display messages if this is the player
  if (stumble_pen >= 60)
   g->add_msg("You miss and stumble with the momentum.");
  else if (stumble_pen >= 10)
   g->add_msg("You swing wildly and miss.");
  else
   g->add_msg("You miss.");
 }
 moves -= stumble_pen;
}


bool player::hit_player(game *g, player &p, body_part &bp,
                        int &hitdam, int &hitcut)
{
// TODO: Add bionics and other bonus (e.g. heat drain, shock, etc)
 if (!is_npc() && p.is_npc()) {
  npc *foe = dynamic_cast<npc*>(&p);
  foe->make_angry();
 }
// Movement cost
 moves -= weapon.attack_time() + 20 * encumb(bp_torso);
 bool unarmed = unarmed_attack(), bashing = weapon.is_bashing_weapon(),
      cutting = weapon.is_cutting_weapon();
 int hitit = hit_roll() - p.dodge_roll(g);
 if (hitit < 0) {	// They dodged
  practice(sk_melee, rng(2, 4));
  if (unarmed)
   practice(sk_unarmed, 3);
  if (bashing)
   practice(sk_bashing, 1);
  if (cutting)
   practice(sk_cutting, 2);
  return false;
 }

 if (hitit >= 15)
  bp = bp_eyes;
 else if (hitit >= 12)
  bp = bp_mouth;
 else if (hitit >= 10)
  bp = bp_head;
 else if (hitit >= 6)
  bp = bp_torso;
 else if (hitit >= 2)
  bp = bp_arms;
 else
  bp = bp_legs;
 
 hitdam = base_damage();

 if (unarmed) {// Unarmed bonuses
  hitdam += rng(0, sklevel[sk_unarmed]);
  if (sklevel[sk_unarmed] >= 5)
   hitdam += rng(sklevel[sk_unarmed], 3 * sklevel[sk_unarmed]);
  if (has_trait(PF_TALONS))
   hitcut += 10;
  if (sklevel[sk_unarmed] >= 8 &&
      (one_in(3) || rng(5, 20) < sklevel[sk_unarmed]))
   hitdam *= rng(2, 3);
 }
// Weapon adds (melee_dam / 4) to (melee_dam)
 hitdam += rng(weapon.damage_bash() / 4, weapon.damage_bash());
 if (bashing)
  hitdam += rng(0, sklevel[sk_bashing]) * sqrt(double(str_cur));

 hitdam += int(pow(1.5, double(sklevel[sk_melee])));
 hitcut = weapon.damage_cut();
 if (hitcut > 0)
  hitcut += int(sklevel[sk_cutting] / 3);
 if (hitdam < 0) hitdam = 0;
 if (hitdam > 0 || hitcut > 0) { // Practicing
  practice(sk_melee, rng(5, 10));
  if (unarmed)
   practice(sk_unarmed, rng(5, 10));
  if (bashing)
   practice(sk_bashing, rng(5, 10));
  if (cutting)
   practice(sk_cutting, rng(5, 10));
 } else { // Less practice if we missed
  practice(sk_melee, rng(2, 5));
  if (unarmed)
   practice(sk_unarmed, 2);
  if (bashing)
   practice(sk_bashing, 2);
  if (cutting)
   practice(sk_cutting, 3);
 }
 return true;
}

bool player::scored_crit(int target_dodge)
{
 bool to_hit_crit = false, dex_crit = false, skill_crit = false;
 int num_crits = 0;

 int chance = 25;
 if (weapon.type->m_to_hit > 0) {
  for (int i = 1; i <= weapon.type->m_to_hit; i++)
   chance += (50 / (2 + i));
 } else if (chance < 0) {
  for (int i = 0; i > weapon.type->m_to_hit; i--)
   chance /= 2;
 }
 if (rng(0, 99) < chance)
  num_crits++;

 chance = 25;
 if (dex_cur > 8) {
  for (int i = 9; i <= dex_cur; i++)
   chance += (21 - i); // 12, 11, 10...
 } else {
  int decrease = 5;
  for (int i = 7; i >= dex_cur; i--) {
   chance -= decrease;
   if (i % 2 == 0)
    decrease--;
  }
 }
 if (rng(0, 99) < chance)
  num_crits++;

 int best_skill = 0;
 if (weapon.is_bashing_weapon() && sklevel[sk_bashing] > best_skill)
  best_skill = sklevel[sk_bashing];
 if (weapon.is_cutting_weapon() && sklevel[sk_cutting] > best_skill)
  best_skill = sklevel[sk_cutting];
 if ((weapon.has_flag(IF_SPEAR) || weapon.has_flag(IF_STAB)) &&
     sklevel[sk_stabbing] > best_skill)
  best_skill = sklevel[sk_stabbing];
 if (unarmed_attack() && sklevel[sk_unarmed] > best_skill)
  best_skill = sklevel[sk_unarmed];

 best_skill += int(sklevel[sk_melee] / 2.5);

 chance = 25;
 if (best_skill > 3) {
  for (int i = 3; i < best_skill; i++)
   chance += (50 / (2 + i));
 } else if (chance < 3) {
  for (int i = 3; i > best_skill; i--)
   chance /= 2;
 }
 if (rng(0, 99) < chance)
  num_crits++;

 if (num_crits == 3)
  return true;
 else if (num_crits == 2)
  return (hit_roll() >= target_dodge * 1.5 && !one_in(4));

 return false;
}

int player::dodge(game *g)
{
 if (has_disease(DI_SLEEP) || has_disease(DI_LYING_DOWN))
  return 0;
 if (activity.type != ACT_NULL)
  return 0;
 int ret = 4 + (dex_cur / 2);
 ret += sklevel[sk_dodge];
 ret -= (encumb(bp_legs) / 2) + encumb(bp_torso);
 ret += int(current_speed(g) / 150);
 if (has_trait(PF_TAIL_LONG))
  ret += 4;
 if (has_trait(PF_TAIL_FLUFFY))
  ret += 8;
 if (has_trait(PF_WHISKERS))
  ret += 1;
 if (has_trait(PF_WINGS_BAT))
  ret -= 3;
 if (str_max >= 16)
  ret--; // Penalty if we're hyuuge
 else if (str_max <= 5)
  ret++; // Bonus if we're small
 if (!can_dodge) { // We already dodged this turn
  if (rng(1, sklevel[sk_dodge] + dex_cur + 15) <= sklevel[sk_dodge] + dex_cur)
   ret = rng(0, ret);
  else
   ret = 0;
 }
 can_dodge = false;
 if (ret > int(dex_cur / 2) + sklevel[sk_dodge] * 2)
  ret = int(dex_cur / 2) + sklevel[sk_dodge] * 2;
 return ret;
}


int player::dodge_roll(game *g)
{
 return dice(dodge(g), 6);
}


int player::base_damage(bool real_life)
{
 int str = (real_life ? str_cur : str_max);
 int dam = (real_life ? rng(0, str / 2) : str / 2);
// Bonus for strong characters
 if (str > 10)
  dam += int((str - 9) / 2);
// Big bonus for super-human characters
 if (str > 20)
  dam += int((str - 20) * 1.5);

 return dam;
}

std::vector<special_attack> player::mutation_attacks(monster *z)
{
 bool is_u = (!is_npc());// Affects how we'll display messages
 std::string You  = (is_u ? "You"  : name);
 std::string Your = (is_u ? "Your" : name + "'s");
 std::string your = (is_u ? "your" : (male ? "his" : "her"));

 std::vector<special_attack> ret;
 std::stringstream text;

 if (has_trait(PF_FANGS) && !wearing_something_on(bp_mouth) &&
     one_in(20 - dex_cur - sklevel[sk_unarmed])) {
  special_attack tmp;
  text << You << " sink" << (is_u ? " " : "s ") << your << " fangs into the " <<
          z->name() << "!";
  tmp.text = text.str();
  tmp.stab = 20;
  ret.push_back(tmp);
 }

 if (has_trait(PF_MANDIBLES) && one_in(22 - dex_cur - sklevel[sk_unarmed])) {
  special_attack tmp;
  text << You << " slice" << (is_u ? " " : "s ") << "the " << z->name() <<
          " with " << your << " mandibles!";
  tmp.text = text.str();
  tmp.cut = 12;
  ret.push_back(tmp);
 }

 if (has_trait(PF_BEAK) && one_in(15 - dex_cur - sklevel[sk_unarmed])) {
  special_attack tmp;
  text << You << " peck" << (is_u ? " " : "s ") << "the " << z->name() << "!";
  tmp.text = text.str();
  tmp.stab = 15;
  ret.push_back(tmp);
 }
  
 if (has_trait(PF_HOOVES) && one_in(25 - dex_cur - 2 * sklevel[sk_unarmed])) {
  special_attack tmp;
  text << You << " kick" << (is_u ? " " : "s ") << "the " << z->name() <<
          " with " << your << " hooves!";
  tmp.text = text.str();
  tmp.bash = str_cur * 3;
  if (tmp.bash > 40)
   tmp.bash = 40;
  ret.push_back(tmp);
 }

 if (has_trait(PF_HORNS) && one_in(20 - dex_cur - sklevel[sk_unarmed])) {
  special_attack tmp;
  text << You << " headbutt" << (is_u ? " " : "s ") << "the " << z->name() <<
          " with " << your << " horns!";
  tmp.text = text.str();
  tmp.bash = 3;
  tmp.stab = 3;
  ret.push_back(tmp);
 }

 if (has_trait(PF_HORNS_CURLED) && one_in(20 - dex_cur - sklevel[sk_unarmed])) {
  special_attack tmp;
  text << You << " headbutt" << (is_u ? " " : "s ") << "the " << z->name() <<
          " with " << your << " curled horns!";
  tmp.text = text.str();
  tmp.bash = 14;
  ret.push_back(tmp);
 }

 if (has_trait(PF_HORNS_POINTED) && one_in(22 - dex_cur - sklevel[sk_unarmed])){
  special_attack tmp;
  text << You << " stab" << (is_u ? " " : "s ") << "the " << z->name() <<
          " with " << your << " pointed horns!";
  tmp.text = text.str();
  tmp.stab = 24;
  ret.push_back(tmp);
 }

 if (has_trait(PF_ANTLERS) && one_in(20 - dex_cur - sklevel[sk_unarmed])) {
  special_attack tmp;
  text << You << " butt" << (is_u ? " " : "s ") << "the " << z->name() <<
          " with " << your << " antlers!";
  tmp.text = text.str();
  tmp.bash = 4;
  ret.push_back(tmp);
 }

 if (has_trait(PF_TAIL_STING) && one_in(3) && one_in(10 - dex_cur)) {
  special_attack tmp;
  text << You << " sting" << (is_u ? " " : "s ") << "the " << z->name() <<
          " with " << your << " tail!";
  tmp.text = text.str();
  tmp.stab = 20;
  ret.push_back(tmp);
 }

 if (has_trait(PF_TAIL_CLUB) && one_in(3) && one_in(10 - dex_cur)) {
  special_attack tmp;
  text << You << " hit" << (is_u ? " " : "s ") << "the " << z->name() <<
          " with " << your << " tail!";
  tmp.text = text.str();
  tmp.bash = 18;
  ret.push_back(tmp);
 }

 if (has_trait(PF_ARM_TENTACLES) || has_trait(PF_ARM_TENTACLES_4) ||
     has_trait(PF_ARM_TENTACLES_8)) {
  int num_attacks = 1;
  if (has_trait(PF_ARM_TENTACLES_4))
   num_attacks = 3;
  if (has_trait(PF_ARM_TENTACLES_8))
   num_attacks = 7;
  if (weapon.is_two_handed(this))
   num_attacks--;

  for (int i = 0; i < num_attacks; i++) {
   if (one_in(18 - dex_cur - sklevel[sk_unarmed])) {
    special_attack tmp;
    text.str("");
    text << You << " slap" << (is_u ? " " : "s ") << "the " << z->name() <<
            " with " << your << " tentacle!";
    tmp.text = text.str();
    tmp.bash = str_cur / 2;
    ret.push_back(tmp);
   }
  }
 }

 return ret;
}
