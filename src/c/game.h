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
#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
// two bigger than the actuall playable area to pad with zero around the edges
#define BOARD_WIDTH (14 + 2)
#define BOARD_HEIGHT (12 + 2)

// in milliseconds
#define BLINKING_DELAY 500

uint8_t board[BOARD_HEIGHT][BOARD_WIDTH] = { 0 };

void fill_board();
void movement();
struct movement_vector get_buttons();
size_t get_valid_directions(struct movement_vector *directions);  // directions should have enough space for 8 movement_vectors
bool check_direction(struct movement_vector direction); // true if the player won't hit any zeros going that way
bool movement_vector_in(struct movement_vector needle, struct movement_vector *haystack, size_t size);
void hint();
void reset();
bool is_empty(struct movement_vector);
uint8_t movement_distance(struct movement_vector);
bool movement_vector_equals(struct movement_vector lhs, struct movement_vector rhs);
int32_t random_range(int32_t min, int32_t max);

struct player {
  uint8_t x;
  uint8_t y;
};

struct movement_vector {
  int8_t x;
  int8_t y;
};

#endif
