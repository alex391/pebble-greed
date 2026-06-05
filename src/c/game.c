/*
copyright (c) 2024, 2026 alex leute

permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "software"), to deal
in the software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the software, and to permit persons to whom the software is
furnished to do so, subject to the following conditions:

the above copyright notice and this permission notice shall be included in all
copies or substantial portions of the software.

the software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement. in no event shall the
authors or copyright holders be liable for any claim, damages or other
liability, whether in an action of contract, tort or otherwise, arising from,
out of or in connection with the software or the use or other dealings in the
software.
*/

/*
  this file contains all of the actual game logic - so everythig that isn't
  drawing to the screen or getting button inputs

  this has been translated from an aurduino c++ version for wio terminal
  https://gist.github.com/alex391/c13f53c876c2ca99fafaacd4404522a4
*/


// TODO: Uh, undo lowercasing of whole file! (how did I even do that without noticing? vim moment)
// just switch to snake_case I think?

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

// two bigger than the actuall playable area to pad with zero around the edges
#define board_width (19 + 2)
#define board_height (12 + 2)

// in milliseconds
#define blinking_delay 500

#define debug false

void fillboard();
void movement();
struct movementvector getbuttons();
size_t getvaliddirections(struct movementvector *directions);  // directions should have enough space for 8 movementvectors
bool checkdirection(struct movementvector direction); // true if the player won't hit any zeros going that way
bool movementvectorin(struct movementvector needle, struct movementvector *haystack, size_t size);
void hint();
void reset();
bool isempty(struct movementvector this);
uint8_t movementdistance(struct movementvector this);
bool movementvectorequals(struct movementvector lhs, struct movementvector rhs);
int32_t randomrange(int32_t min, int32_t max);

uint32_t score = 0;

uint8_t board[board_height][board_width] = { 0 };

struct player {
  uint8_t x;
  uint8_t y;
};

struct player player = { 1, 1 };

struct movementvector {
  int8_t x;
  int8_t y;

};

bool isempty(struct movementvector this) {
  return this.x == 0 && this.y == 0;
}

uint8_t movementdistance(struct movementvector this) {
 return board[player.y + this.y][player.x + this.x];
}

bool movementvectorequals(struct movementvector lhs, struct movementvector rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

void setup() {
  reset();
}

// todo: i'm pretty sure this function makes no sense here, delete?
void loop() {
  hint();
  movement();
}

void reset() {
  score = 0;
  player.x = randomrange(1, board_width - 1);
  player.y = randomrange(1, board_height - 1);

  fillboard();
}

// return a random number betweeen min (inclusive) and max (exclusive)
int32_t randomrange(int32_t min, int32_t max) {
  return (rand() % max) + min;
}

void fillboard() {
  for (size_t y = 1; y < board_height - 1; y++) {
    for (size_t x = 1; x < board_width - 1; x++) {
      board[y][x] = randomrange(1, 8);
    }
  }
  board[player.y][player.x] = 0;
}

void movement() {
  struct movementvector valid_directions[8] = { 0 };
  size_t valid_directions_count = getvaliddirections(valid_directions);
  if (valid_directions_count == 0) {
    char game_over_buff[28] = { 0 };  // big enough for "game over! score: 999 99.9%\0"
    float percentage = (float)score * 100.0f / (float)((board_height - 2) * (board_width - 2));
    snprintf(game_over_buff, sizeof(game_over_buff), "game over! score: %" PRIu32 "%.1f%%", score, percentage);
    // todo: draw game_over_buff onto the screen, and then wait to be reset
    reset();
  }
  struct movementvector buttons;
  buttons = getbuttons();
  if (isempty(buttons)) {
    return;
  }
  struct movementvector held_buttons = { 0 };
  bool redraw = true;
  do {
    // todo: this probably makes not a lot of sense here
    held_buttons = getbuttons();
    // make it easier to go diagonally.
    if (buttons.x == 0 && held_buttons.x != 0) {
      buttons.x = held_buttons.x;
      redraw = true;
    }
    if (buttons.y == 0 && held_buttons.y != 0) {
      buttons.y = held_buttons.y;
      redraw = true;
    }
    if (redraw && movementvectorin(buttons, valid_directions, valid_directions_count)) {
      // todo: draw the direction the player is moving here
      // todo: re-draw the board here
      redraw = false;
    }
    // todo: delay(polling_delay);
  } while (!(isempty(held_buttons)));  // wait for release

  uint8_t movement_distance = movementdistance(buttons);
  for (; movement_distance > 0; movement_distance--) {
    player.x += buttons.x;
    player.y += buttons.y;
    board[player.y][player.x] = 0;
    score++;
  }
  board[player.y][player.x] = 0;
}

struct movementvector getbuttons() {
  struct movementvector v = { 0 };
  // todo: how to set v here
  return v;
}

// play greed... greedily!
struct movementvector greedybuttons(struct movementvector *directions, size_t size) {
  uint8_t max = 0;
  struct movementvector max_direction = { 0 };
  for (size_t i = 0; i < size; i++) {
    if (movementdistance(directions[i]) > max) {
      max = movementdistance(directions[i]);
      max_direction = directions[i];
    }
  }
  return max_direction;
}

// leaving it up to the caller to make sure directions enough space for 8
// directions in it
// returns the number of directions that are valid
size_t getvaliddirections(struct movementvector *directions) {
  size_t directions_index = 0;
  for (int8_t x = -1; x <= 1; x++) {
    for (int8_t y = -1; y <= 1; y++) {
      if (x == 0 && y == 0) {
        continue;
      }
      struct movementvector v = { x, y };
      if (checkdirection(v)) {
        directions[directions_index++] = v;
      }
    }
  }
  return directions_index;
}

bool checkdirection(struct movementvector direction) {
  uint8_t movement_distance = movementdistance(direction);
  if (movement_distance == 0) {
    return false;
  }
  struct player temp_player = player;
  for (; movement_distance > 0; movement_distance--) {
    temp_player.x += direction.x;
    temp_player.y += direction.y;
    if (board[temp_player.y][temp_player.x] == 0) {
      return false;
    }
  }
  return true;
}

bool movementvectorin(struct movementvector needle, struct movementvector *haystack, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (movementvectorequals(needle, haystack[i])) {
      return true;
    }
  }
  return false;
}

// todo: this function is supposed to show you all of the directions you're
// allowed to go in, and how far they'd take you. since it's all screen drawing,
// it doesn't really belong here
void hint() {
}
