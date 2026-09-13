/*
Copyright (c) 2024, 2026 Alex Leute

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
  this file contains all of the actual game logic - so everythig that isn't
  drawing to the screen or getting button inputs

  this has been translated from an aurduino c++ version for wio terminal
  https://gist.github.com/alex391/c13f53c876c2ca99fafaacd4404522a4
*/
#include <pebble.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "game.h"
#include "greed.h"

uint32_t score = 0;
uint8_t board[BOARD_HEIGHT][BOARD_WIDTH] = { 0 };

// The different plaltforms have different usable screen area, so mask off the
// parts that aren't usable:
uint8_t board_mask[BOARD_HEIGHT][BOARD_WIDTH] =
#ifdef PBL_PLATFORM_GABBRO
  {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
  };
#else
  { 0 }; // This shoudn't happen
#endif


struct movement_vector buttons = { 0 };

struct player player = { 1, 1 };


bool is_empty(struct movement_vector this) {
  return this.x == 0 && this.y == 0;
}

uint8_t movement_distance(struct movement_vector this) {
 return board[player.y + this.y][player.x + this.x];
}

// Get from the board, returns -1 if x or y are out of bounds
int8_t board_get(int32_t x, int32_t y) {
  if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT) {
    return board[y][x];
  }
  return -1;
}

struct player get_player() {
  return player;
}

bool movement_vector_equals(struct movement_vector lhs, struct movement_vector rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

void setup() {
  reset();
}

void place_player() {
  do {
    player.x = random_range(0, BOARD_WIDTH + 1); // + 1 because max is exclusive
    player.y = random_range(0, BOARD_HEIGHT + 1);
  }
  while(board_mask[player.y][player.x] == 0);
}

void reset() {
  score = 0;\
  place_player();
  fill_board();
  board[player.y][player.x] = 0;

}

// return a random number betweeen min (inclusive) and max (exclusive)
int32_t random_range(int32_t min, int32_t max) {
  return (rand() % max) + min;
}

void apply_board_mask() {
  for (size_t y = 1; y < BOARD_HEIGHT; y++) {
    for (size_t x = 1; x < BOARD_WIDTH; x++) {
      board[y][x] = board_mask[y][x] * board[y][x];
    }
  }
}

void fill_board() {
  for (size_t y = 1; y < BOARD_HEIGHT; y++) {
    for (size_t x = 1; x < BOARD_WIDTH; x++) {
      board[y][x] = random_range(1, MAX_NUMBER);
    }
  }

  apply_board_mask();
}

void movement(GContext *ctx) {
  if (is_empty(buttons)) {
    return;
  }

  struct movement_vector valid_directions[8] = { 0 };
  size_t valid_directions_count = get_valid_directions(valid_directions);


  if (!movement_vector_in(buttons, valid_directions, valid_directions_count)) {
    return;
  }

  uint8_t movement_dist = movement_distance(buttons);
  for (; movement_dist > 0; movement_dist--) {
    player.x += buttons.x;
    player.y += buttons.y;
    board[player.y][player.x] = 0;
    score++;
  }
  board[player.y][player.x] = 0;

  valid_directions_count = get_valid_directions(valid_directions);
  if (valid_directions_count == 0) {
    char game_over_buff[40] = { 0 };  // big enough for "Game over! Score: 999 99.9%\0"
    float percentage = (float)score * 100.0f / (float)((BOARD_HEIGHT - 2) * (BOARD_WIDTH - 2));
    int32_t percentage_whole_part = (int32_t)percentage;
    int32_t percentage_fraction_part = (int32_t)((percentage - (float)percentage_whole_part) * 10.0f); 
    snprintf(game_over_buff, sizeof(game_over_buff), "Game over! Score: %" PRIu32 " %" PRIi32 ".%" PRIi32 "%%", score, percentage_whole_part, percentage_fraction_part);
    // TODO: draw game_over_buff onto the screen, and then wait to be reset
    set_gameover_text(game_over_buff);
    set_gameover(true);
    reset();
  }
}

struct movement_vector get_buttons() {
  return buttons;
}


void set_buttons(int8_t x, int8_t y) {
  if (x < -1 || x > 1 || y < -1 || y > 1) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting buttons to out of range %d, %d", x, y);
  }
  buttons.x = x;
  buttons.y = y;
}

// return min of a and b
int8_t int8_t_min(int8_t a, int8_t b) {
  if (a < b) {
    return a;
  }
  return b;
}

// return max of a and b
int8_t int8_t_max(int8_t a, int8_t b) {
  if (a > b) {
    return a;
  }
  return b;
}

// return a value that's between lower_bounds and upper_bounds, inclusive,
// that's closeset to x
int8_t int8_t_move_into_range(int8_t lower_bounds, int8_t x, int8_t upper_bounds) {
  int8_t above_lower = int8_t_max(lower_bounds, x);
  return int8_t_min(upper_bounds, above_lower);
}

void combine_buttons(enum direction direction) {
  struct movement_vector buttons_copy = buttons;
  switch (direction) {
    case UP:
      // If it's confusing to you that negitive y is up, just immagine the
      // enemy's gate on the top of the board
      buttons_copy.y--;
      break;
    case DOWN:
      buttons_copy.y++;
      break;
    case LEFT:
      buttons_copy.x--;
      break;
    case RIGHT:
      buttons_copy.x++;
      break;
  }
  buttons.x = int8_t_move_into_range(-1, buttons_copy.x, 1);
  buttons.y = int8_t_move_into_range(-1, buttons_copy.y, 1);

}



// play greed... greedily!
struct movement_vector greedybuttons(struct movement_vector *directions, size_t size) {
  uint8_t max = 0;
  struct movement_vector max_direction = { 0 };
  for (size_t i = 0; i < size; i++) {
    if (movement_distance(directions[i]) > max) {
      max = movement_distance(directions[i]);
      max_direction = directions[i];
    }
  }
  return max_direction;
}

// leaving it up to the caller to make sure directions enough space for 8
// directions in it
// returns the number of directions that are valid
size_t get_valid_directions(struct movement_vector *directions) {
  size_t directions_index = 0;
  for (int8_t x = -1; x <= 1; x++) {
    for (int8_t y = -1; y <= 1; y++) {
      if (x == 0 && y == 0) {
        continue;
      }
      struct movement_vector v = { x, y };
      if (check_direction(v)) {
        directions[directions_index++] = v;
      }
    }
  }
  return directions_index;
}

bool check_direction(struct movement_vector direction) {
  uint8_t movement_dist = movement_distance(direction);
  if (movement_dist == 0) {
    return false;
  }
  struct player temp_player = player;
  for (; movement_dist > 0; movement_dist--) {
    temp_player.x += direction.x;
    temp_player.y += direction.y;
    if (board[temp_player.y][temp_player.x] == 0) {
      return false;
    }
  }
  return true;
}

bool movement_vector_in(struct movement_vector needle, struct movement_vector *haystack, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (movement_vector_equals(needle, haystack[i])) {
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

