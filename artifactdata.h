#ifndef _ARTIFACTDATA_H_
#define _ARTIFACTDATA_H_

#include <vector>
#include "artifact.h"
#include "itype.h"

enum artifact_weapon_type
{
 ARTWEAP_NULL,
 ARTWEAP_BULK, // A bulky item that works okay for bashing
 ARTWEAP_CLUB, // An item designed to bash
 ARTWEAP_SPEAR, // A stab-only weapon
 ARTWEAP_SWORD, // A long slasher
 ARTWEAP_KNIFE, // Short, slash and stab
 NUM_ARTWEAPS
};

struct artifact_tool_form_datum
{
 std::string name;
 char sym;
 nc_color color;
 material m1;
 material m2;
 int volume_min, volume_max;
 int weight_min, weight_max;
 artifact_weapon_type base_weapon;
 artifact_weapon_type extra_weapons[3];

/*
 artifact_tool_form_datum
   (std::string Name, char Sym, nc_color Color, material M1, material M2,
    int Volmin, int Volmax, int Wgtmin, int Wgtmax, artifact_weapon_type Base,
    artifact_weapon_type extra[3])
   : name (Name), sym (Sym), color (Color), m1 (M1), m2 (M2),
     volume_min (Volmin), volume_max (Volmax), weight_min (Wgtmin),
     weight_max (Wgtmax), base_weapon (Base), extra_weapons (extra) { };
*/
                           
};

enum artifact_tool_form
{
 ARTTOOLFORM_NULL,
 ARTTOOLFORM_HARP,
 ARTTOOLFORM_STAFF,
 ARTTOOLFORM_SWORD,
 ARTTOOLFORM_KNIFE,
 ARTTOOLFORM_CUBE,
 NUM_ARTTOOLFORMS
};

artifact_tool_form_datum artifact_tool_form_data[NUM_ARTTOOLFORMS] = {
{"", '*', c_white, MNULL, MNULL, 0, 0, 0, 0, ARTWEAP_BULK,
 {ARTWEAP_NULL, ARTWEAP_NULL, ARTWEAP_NULL}},

{"Harp", ';', c_yellow, WOOD, MNULL, 20, 30, 10, 18, ARTWEAP_BULK,
 {ARTWEAP_SPEAR, ARTWEAP_SWORD, ARTWEAP_KNIFE}},

{"Staff", '/', c_brown, WOOD, MNULL, 6, 12, 4, 10, ARTWEAP_CLUB,
 {ARTWEAP_BULK, ARTWEAP_SPEAR, ARTWEAP_KNIFE}},

{"Sword", '/', c_ltblue, STEEL, MNULL, 8, 14, 8, 28, ARTWEAP_SWORD,
 {ARTWEAP_BULK, ARTWEAP_NULL, ARTWEAP_NULL}},

{"Dagger", ';', c_ltblue, STEEL, MNULL, 1, 4, 1, 6, ARTWEAP_KNIFE,
 {ARTWEAP_NULL, ARTWEAP_NULL, ARTWEAP_NULL}},

{"Cube", '*', c_white, STEEL, MNULL, 1, 3, 1, 20, ARTWEAP_BULK,
 {ARTWEAP_SPEAR, ARTWEAP_NULL, ARTWEAP_NULL}}
};

struct artifact_weapon_datum
{
 std::string adjective;
 int volume, weight; // Only applicable if this is an *extra* weapon
 int bash_min, bash_max;
 int cut_min, cut_max;
 int to_hit_min, to_hit_max;
 unsigned flags : NUM_ITEM_FLAGS;
};

artifact_weapon_datum artifact_weapon_data[NUM_ARTWEAPS] = {
{"", 0, 0, 0, 0, 0, 0, 0, 0, 0},
// Adjective	Vol,wgt		Bash		Cut		To-Hit
{"Heavy",	 0, 12,		10, 20,		 0,  0,		-2,  0,
 0},
{"Knobbed",	 1,  2,		14, 30,		 0,  0,		-1,  1,
 0},
{"Spiked",	 1,  1,		 0,  0,		20, 40,		-1,  1,
 mfb(IF_SPEAR)},
{"Edged",	 2,  4,		 0,  0,		20, 50,		-1,  2,
 0},
{"Bladed",	 1,  2,		 0,  0,		12, 30,		-1,  1,
 mfb(IF_STAB)}
};

enum artifact_armor_mod
{
 ARMORMOD_NULL,
 ARMORMOD_LIGHT,
 ARMORMOD_BULKY,
 ARMORMOD_POCKETED,
 ARMORMOD_FURRED,
 ARMORMOD_PADDED,
 ARMORMOD_PLATED,
 NUM_ARMORMODS
};

struct artifact_armor_form_datum
{
 std::string name;
 nc_color color;
 material m1;
 material m2;
 int volume, weight;
 int encumb;
 int dmg_resist;
 int cut_resist;
 int env_resist;
 int warmth;
 int storage;
 int melee_bash, melee_cut, melee_hit;
 unsigned char covers;
 bool plural;
 artifact_armor_mod available_mods[5];

/*
// Constructor
 artifact_armor_form_datum
   (std::string Name, nc_color Color, material M1, material M2, int Volume,
    int Weight, int Encumb, int Dmg_res, int Cut_res, int Env_res,
    int Warmth, int Storage, int Bash, int Cut, int Hit, unsigned char Covers,
    bool Plural, artifact_armor_mod Mods[5])
   : name (Name), color (Color), m1 (M1), m2 (M2), volume (Volume),
     weight (Weight), encumb (Encumb), dmg_resist (Dmg_res),
     cut_resist (Cut_res), env_resist (Env_res), warmth (Warmth),
     storage (Storage), melee_bash (Bash), melee_cut (Cut), melee_hit (Hit),
     covers (Covers), plural (Plural), available_mods (Mods) { };

*/
};
 
enum artifact_armor_form
{
 ARTARMFORM_NULL,
 ARTARMFORM_ROBE,
 ARTARMFORM_COAT,
 ARTARMFORM_MASK,
 ARTARMFORM_HELM,
 ARTARMFORM_GLOVES,
 ARTARMFORM_BOOTS,
 NUM_ARTARMFORMS
};

artifact_armor_form_datum artifact_armor_form_data[NUM_ARTARMFORMS] = {
{"", c_white, MNULL, MNULL,        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
 0, false,
 {ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL}},
// Name    color  Materials      Vol Wgt Enc Dmg Cut Env Wrm Sto Bsh Cut Hit
{"Robe",   c_red, WOOL, MNULL,     6,  6,  1,  3,  1,  0,  2,  0, -8,  0, -3,
 mfb(bp_torso)|mfb(bp_legs), false,
 {ARMORMOD_LIGHT, ARMORMOD_BULKY, ARMORMOD_POCKETED, ARMORMOD_FURRED,
  ARMORMOD_PADDED}},

{"Coat",   c_brown,LEATHER,MNULL, 14, 14,  2,  4, 12,  1,  4,  4, -6,  0, -3,
 mfb(bp_torso), false,
 {ARMORMOD_LIGHT, ARMORMOD_POCKETED, ARMORMOD_FURRED, ARMORMOD_PADDED,
  ARMORMOD_PLATED}},

{"Mask",   c_white, WOOD, MNULL,   4,  1,  2,  2, 16,  1,  2,  0,  2,  0, -2,
 mfb(bp_eyes)|mfb(bp_mouth), false,
 {ARMORMOD_FURRED, ARMORMOD_FURRED, ARMORMOD_NULL, ARMORMOD_NULL,
  ARMORMOD_NULL}},

// Name    color  Materials      Vol Wgt Enc Dmg Cut Env Wrm Sto Bsh Cut Hit
{"Helm",   c_dkgray,SILVER, MNULL, 6,  6,  2,  4, 18,  0,  1,  0,  8,  0, -2,
 mfb(bp_head), false,
 {ARMORMOD_BULKY, ARMORMOD_FURRED, ARMORMOD_PADDED, ARMORMOD_PLATED,
  ARMORMOD_NULL}},

{"Gloves", c_ltblue,LEATHER,MNULL, 2,  1,  1,  6,  6,  1,  2,  0, -4,  0, -2,
 mfb(bp_hands), true,
 {ARMORMOD_BULKY, ARMORMOD_FURRED, ARMORMOD_PADDED, ARMORMOD_PLATED, 
  ARMORMOD_NULL}},

// Name    color  Materials      Vol Wgt Enc Dmg Cut Env Wrm Sto Bsh Cut Hit
{"Boots", c_blue, LEATHER, MNULL,  6,  2,  1,  6,  6,  1,  3,  0,  4,  0, -1,
 mfb(bp_feet), true,
 {ARMORMOD_LIGHT, ARMORMOD_BULKY, ARMORMOD_PADDED, ARMORMOD_PLATED,
  ARMORMOD_NULL}}

};

/* Armor mods alter the normal values of armor.
 * If the basic armor type has MNULL as its second material, and the mod has a
 * material attached, the second material will be changed.
 */
artifact_armor_form_datum artifact_armor_mod_data[NUM_ARMORMODS] = {

{"", c_white, MNULL, MNULL, 0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, false,
 {ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL}},
// Description; "It is ..." or "They are ..."
{"very thin and light.", c_white, MNULL, MNULL,
// Vol Wgt Enc Dmg Cut Env Wrm Sto
    -4, -8, -2, -1, -1, -1, -1,  0, 0, 0, 0, 0,  false,
 {ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL}},

{"extremely bulky.", c_white, MNULL, MNULL,
     8, 10,  2,  1,  1,  0,  1,  0, 0, 0, 0, 0,  false,
 {ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL}},

{"covered in pockets.", c_white, MNULL, MNULL,
     1,  1,  1,  0,  0,  0,  0, 16, 0, 0, 0, 0,  false,
 {ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL}},

{"disgustingly furry.", c_white, WOOL, MNULL,
// Vol Wgt Enc Dmg Cut Env Wrm Sto
     4,  2,  1,  4,  0,  1,  3,  0, 0, 0, 0, 0,  false,
 {ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL}},

{"leather-padded.", c_white, LEATHER, MNULL,
     4,  4,  1, 10,  4,  0,  1, -3, 0, 0, 0, 0,  false,
 {ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL}},

{"plated in iron.", c_white, IRON, MNULL,
     4, 12,  2,  8, 14,  0,  1, -4, 0, 0, 0, 0, false,
 {ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL, ARMORMOD_NULL}},

};


#define NUM_ART_ADJS 20
std::string artifact_adj[NUM_ART_ADJS] = {
"Forbidden", "Unknown", "Forgotten", "Hideous", "Eldritch",
"Gelatinous", "Ancient", "Cursed", "Bloody", "Undying",
"Shadowy", "Silent", "Cyclopean", "Fungal", "Unspeakable",
"Grotesque", "Frigid", "Shattered", "Sleeping", "Repellent"
};

#define NUM_ART_NOUNS 20
// Prepending + makes it proper, e.g. "The Forbidden Abyss"
std::string artifact_noun[NUM_ART_NOUNS] = {
"Technique", "Dreams", "Beasts", "Evil", "Miasma",
"+Abyss", "+City", "Shadows", "Shade", "Illusion",
"Justice", "+Necropolis", "Ichor", "+Monolith", "Aeons",
"Graves", "Horrors", "Suffering", "Death", "Horror"
};

#endif
