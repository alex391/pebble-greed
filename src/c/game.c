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

uint32_t score = 0;
uint8_t board[BOARD_HEIGHT][BOARD_WIDTH] = { 0 };



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

bool movement_vector_equals(struct movement_vector lhs, struct movement_vector rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

void setup() {
  reset();
}

// TODO: i'm pretty sure this function makes no sense here, delete?
void loop() {
  hint();
  movement();
}

void reset() {
  score = 0;
  player.x = random_range(1, BOARD_WIDTH - 1);
  player.y = random_range(1, BOARD_HEIGHT - 1);

  fill_board();
}

// return a random number betweeen min (inclusive) and max (exclusive)
int32_t random_range(int32_t min, int32_t max) {
  return (rand() % max) + min;
}

void fill_board() {
  for (size_t y = 1; y < BOARD_HEIGHT - 1; y++) {
    for (size_t x = 1; x < BOARD_WIDTH - 1; x++) {
      board[y][x] = random_range(1, MAX_NUMBER);
    }
  }
  board[player.y][player.x] = 0;
}

void movement() {
  struct movement_vector valid_directions[8] = { 0 };
  size_t valid_directions_count = get_valid_directions(valid_directions);
  if (valid_directions_count == 0) {
    char game_over_buff[28] = { 0 };  // big enough for "Game over! Score: 999 99.9%\0"
    float percentage = (float)score * 100.0f / (float)((BOARD_HEIGHT - 2) * (BOARD_WIDTH - 2));
    snprintf(game_over_buff, sizeof(game_over_buff), "Game over! Score: %" PRIu32 "%.1f%%", score, percentage);
    // TODO: draw game_over_buff onto the screen, and then wait to be reset
 
    reset();
  }
  struct movement_vector buttons;
  buttons = get_buttons();
  if (is_empty(buttons)) {
    return;
  }
  struct movement_vector held_buttons = { 0 };
  bool redraw = true;
  do {
    // todo: this probably makes not a lot of sense here
    held_buttons = get_buttons();
    // make it easier to go diagonally.
    if (buttons.x == 0 && held_buttons.x != 0) {
      buttons.x = held_buttons.x;
      redraw = true;
    }
    if (buttons.y == 0 && held_buttons.y != 0) {
      buttons.y = held_buttons.y;
      redraw = true;
    }
    if (redraw && movement_vector_in(buttons, valid_directions, valid_directions_count)) {
      // todo: draw the direction the player is moving here
      // todo: re-draw the board here
      redraw = false;
    }
    // todo: delay(polling_delay);
  } while (!(is_empty(held_buttons)));  // wait for release

  uint8_t movement_dist = movement_distance(buttons);
  for (; movement_dist> 0; movement_dist--) {
    player.x += buttons.x;
    player.y += buttons.y;
    board[player.y][player.x] = 0;
    score++;
  }
  board[player.y][player.x] = 0;
}

struct movement_vector get_buttons() {
  struct movement_vector v = { 0 };
  // todo: how to set v here
  return v;
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
  uint8_t movement_dist= movement_distance(direction);
  if (movement_dist== 0) {
    return false;
  }
  struct player temp_player = player;
  for (; movement_dist> 0; movement_dist--) {
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
