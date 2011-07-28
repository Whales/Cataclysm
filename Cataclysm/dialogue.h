#ifndef _DIALOGUE_H_
#define _DIALOGUE_H_

#include "player.h"
#include "output.h"
#include "npc.h"
#include <vector>
#include <string>

struct dialogue {
 player *alpha;
 npc *beta;
 WINDOW *win;
 bool done;
 std::vector<std::string> history;

 int opt(std::string challenge, ...);
};

/* There is a array of tag_data, "tags", at the bottom of this file.
 * It maps tags to the array of string replacements;
 * e.g. "<name_g>" => talk_good_names
 * Other tags, like "<yrwp>", are mapped to dynamic things
 *  (like the player's weapon), and are defined in parse_tags() (npctalk.cpp)
 */
struct tag_data
{
 std::string tag;
 std::string (*replacement)[10];
};

enum talk_topic {
 TALK_NONE = 0,		// Nothing; let the player take the lead
 TALK_INQUIRE,		// Inquire about what the player's doing
 TALK_JOIN,		// Ask to join forces
 TALK_TRADE,		// Look to trade goods
 NUM_TALK_TOPICS
};

std::string talk_needs[num_needs][5] = {
{"", "", "", "", ""},
{"Hey<punc> You got any <ammo>?", "I'll need some <ammo> soon, got any?",
 "I really need some <ammo><punc>", "I need <ammo> for my <mywp>, got any?",
 "I need some <ammo> <very> bad<punc>"},
{"Got anything I can use as a weapon?",
 "<ill_die> without a good weapon<punc>",
 "I'm sick of fighting with my <swear> <mywp>, got something better?",
 "Hey <name_g>, care to sell me a weapon?",
 "My <mywp> just won't cut it, I need a real weapon..."},
{"Hey <name_g>, I could really use a gun.",
 "Hey, you got a spare gun?  It'd be better than my <swear> <mywp><punc>",
 "<ill_die> if I don't find a gun soon!",
 "<name_g><punc> Feel like selling me a gun?",
 "I need a gun, any kind will do!"},
{"I could use some food, here.", "I need some food, <very> bad!",
 "Man, am I <happy> to see you!  Got any food to trade?",
 "<ill_die> unless I get some food in me<punc> <okay>?",
 "Please tell me you have some food to trade!"},
{"Got anything to drink?", "I need some water or something.",
 "<name_g>, I need some water... got any?",
 "<ill_die> without something to drink.", "You got anything to drink?"}
/*
{"<ill_die> unless I get healed<punc>", "You gotta heal me up, <name_g><punc>",
 "Help me<punc> <ill_die> if you don't heal me<punc>",
 "Please... I need medical help<punc>", "
*/
};

std::string talk_okay[10] = {
"okay", "get it", "you dig", "dig", "got it", "you see", "see, <name_g>",
"alright", "that clear"};

std::string talk_bad_names[10] = {
"punk",		"bitch",	"dickhead",	"asshole",	"fucker",
"sucker",	"fuckwad",	"fuckface",	"motherfucker",	"shithead"};

std::string talk_good_names[10] = {
"stranger",	"friend",	"pilgrim",	"traveler",	"pal",
"fella",	"you",		"dude",		"buddy",	"man"};

std::string talk_swear[10] = { // e.g. "drop the <swear> weapon"
"fucking", "goddamn", "motherfucking", "freaking", "damn", "<swear> <swear>",
"shit-sucking", "fuckin'", "god damn", "mafuckin'"};

std::string talk_very[10] = { // Synonyms for "very" -- applied to adjectives
"really", "fucking", "super", "wicked", "very", "mega", "uber", "ultra",
"so <very>", "<very> <very>"};

std::string talk_really[10] = { // Synonyms for "really" -- applied to verbs
"really", "fucking", "absolutely", "definitely", "for real", "honestly",
"<really> <really>", "most <really>", "urgently", "REALLY"};

std::string talk_happy[10] = {
"glad", "happy", "overjoyed", "ecstatic", "thrilled", "stoked", "<very> <happy>",
"tickled pink", "delighted", "pumped"};

std::string talk_sad[10] = {
"sad", "bummed", "depressed", "pissed", "unhappy", "<very> <sad>", "dejected",
"down", "blue", "glum"};

std::string talk_greeting_gen[10] = {
"Hey <name_g>.", "Greetings <name_g>.", "Hi <name_g><punc> You okay?",
"<name_g><punc>  Let's talk.", "Well hey there.",
"<name_g><punc>  Hello.", "What's up, <name_g>?", "You okay, <name_g>?",
"Hello, <name_g>.", "Hi <name_g>"};

std::string talk_ill_die[10] = {
"I'm not gonna last much longer", "I'll be dead soon", "I'll be a goner",
"I'm dead, <name_g>,", "I'm dead meat", "I'm in <very> serious trouble",
"I'm <very> doomed", "I'm done for", "I won't last much longer",
"my days are <really> numbered"};

std::string talk_drop_weapon[10] = {
"Drop your <swear> weapon!",
"Okay <name_b>, drop your weapon!",
"Put your <swear> weapon down!",
"Drop the <yrwp>, <name_b>!",
"Drop the <swear> <yrwp>!",
"Drop your <yrwp>!",
"Put down the <yrwp>!",
"Drop your <swear> weapon, <name_b>!",
"Put down your <yrwp>!",
"Alright, drop the <yrwp>!"
};

std::string talk_hands_up[10] = {
"Put your <swear> hands up!",
"Put your hands up, <name_b>!",
"Reach for the sky!",
"Hands up!",
"Hands in the air!",
"Hands up, <name_b>!",
"Hands where I can see them!",
"Okay <name_b>, hands up!",
"Okay <name_b><punc> hands up!",
"Hands in the air, <name_b>!"
};

std::string talk_no_faction[10] = {
"I'm unaffiliated.",
"I don't run with a crew.",
"I'm a solo artist, <okay>?",
"I don't kowtow to any group, <okay>?",
"I'm a freelancer.",
"I work alone, <name_g>.",
"I'm a free agent, more money that way.",
"I prefer to work uninhibited by that kind of connection.",
"I haven't found one that's good enough for me.",
"I don't belong to a faction, <name_g>"
};

std::string talk_come_here[10] = {
"Wait up, let's talk!",
"Hey, I <really> want to talk to you!",
"Come on, talk to me!",
"Hey <name_g>, let's talk!",
"<name_g>, we <really> need to talk!",
"Hey, we should talk, <okay>?",
"<name_g>!  Wait up!",
"Wait up, <okay>?",
"Let's talk, <name_g>!",
"Look, <name_g><punc> let's talk!"
};

std::string talk_wait[10] = {
"Hey, where are you?",
"Wait up, <name_g>!",
"<name_g>, wait for me!",
"Hey, wait up, <okay>?",
"You <really> need to wait for me!",
"You <swear> need to wait!",
"<name_g>, where are you?",
"Hey <name_g><punc> Wait for me!",
"Where are you?!",
"Hey, I'm over here!"
};

std::string talk_let_me_pass[10] = {
"Excuse me, let me pass.",
"Hey <name_g>, can I get through?",
"Let me get past you, <name_g>.",
"Let me through, <okay>?",
"Can I get past you, <name_g>?",
"I need to get past you, <name_g>.",
"Move your <swear> ass, <name_b>!",
"Out of my way, <name_b>!",
"Move it, <name_g>!",
"You need to move, <name_g>, <okay>?"
};

// Used to tell player to move to avoid friendly fire
std::string talk_move[10] = {
"Move",
"Move your ass",
"Get out of the way",
"You need to move"
"Hey <name_g>, move",
"<swear> move it",
"Move your <swear> ass",
"Get out of my way, <name_b>,",
"Move to the side",
"Get out of my line of fire"
};

std::string talk_done_mugging[10] = {
"Thanks for the cash, <name_b>!",
"So long, <name_b>!",
"Thanks a lot, <name_g>!",
"Catch you later, <name_g>!",
"See you later, <name_b>!",
"See you in hell, <name_b>!",
"Hasta luego, <name_g>!",
"I'm outta here! <done_mugging>",
"Bye bye, <name_b>!",
"Thanks, <name_g>!"
};

#define NUM_STATIC_TAGS 19


tag_data talk_tags[NUM_STATIC_TAGS] = {
{"<okay>",		&talk_okay},
{"<name_b>",		&talk_bad_names},
{"<name_g>",		&talk_good_names},
{"<swear>",		&talk_swear},
{"<very>",		&talk_very},
{"<really>",		&talk_really},
{"<happy>",		&talk_happy},
{"<sad>",		&talk_sad},
{"<greet>",		&talk_greeting_gen},
{"<ill_die>",		&talk_ill_die},
{"<drop_it>",		&talk_drop_weapon},
{"<hands_up>",		&talk_hands_up},
{"<no_faction>",	&talk_no_faction},
{"<come_here>",		&talk_come_here},
{"<lets_talk>",		&talk_come_here},
{"<wait>",		&talk_wait},
{"<let_me_pass>",	&talk_let_me_pass},
{"<move>",		&talk_move},
{"<done_mugging>",	&talk_done_mugging}
};

#endif
