#ifndef _MORALEDATA_H_
#define _MORALEDATA_H_

#include <string>
#include "morale.h"

std::string morale_data[NUM_MORALE_TYPES] = {
"This is a bug",
"Enjoyed %i",
"Music",
"Marloss Bliss",
"Good Feeling",

"Nicotine Craving",
"Caffeine Craving",
"Alcohol Craving",
"Opiate Craving",
"Speed Craving",
"Cocaine Craving",

"Disliked %i",
"Ate Meat",
"Wet",
"Bad Feeling",
"Killed Innocent",
"Killed Friend",
"Killed Mother",

"Moodswing",
"Read %i",
};

int morale_halflives[NUM_MORALE_TYPES] = {
  0,  // (bug)
  100, // Enjoyed %i
  75, // Music
  250, // Marloss
  75, // Good Feeling / Chem Imbalance

  100, // Nicotine Craving
  100, // Caffeine Craving
  100, // Alcohol Craving
  100, // Opiate Craving
  100, // Speed Craving
  100, // Cocaine Craving
  
  100, // Disliked %i
  100, // Ate Meat (vegetarian)
  75, // Wet
  75, // Bad Feeling / Chem Imbalance
  4000, // Killed Innocent
  10000, // Killed Friend
  2000, // Killed Mother

  200, // Moodswing
  4000 // Book
};


#endif
