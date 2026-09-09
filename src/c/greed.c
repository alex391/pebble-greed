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
#include <pebble.h>
#include <limits.h>
#include "greed.h"
#include "game.h"

#define CHARACTER_WIDTH 13
#define CHARACTER_HEIGHT 16

#define BUTTON_TIMEOUT_MS 500

static Window *s_window;
static Layer *s_canvas_layer;
static GContext *context;
static char gameover_text[40] = { 0 };
static bool gameover = false;

AppTimer *button_timer = NULL;

// helper for logging, with a line number
void debug_log(const char *message, int32_t line) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%s: line %d", message, line);
}

// The idea here is to just print a message to the console and then just do
// nothing forever because something irricoverably bad happened
void log_and_spin(const char *message, int32_t line) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "%s: %d", message, line);
  while(true) {
    psleep(INT_MAX);
  }
}

void unclick(void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "unclick");
  button_timer = NULL;
  if (gameover) {
    gameover = false;
    layer_mark_dirty(s_canvas_layer);
    set_buttons(0, 0);
    return;
  }
  struct movement_vector current_buttons = get_buttons();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "buttons registered: %d, %d", current_buttons.x, current_buttons.y);
  movement();
  layer_mark_dirty(s_canvas_layer);
  set_buttons(0, 0);
}

void set_click_timer() {
  if (button_timer == NULL) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "setting timer");
    button_timer = app_timer_register(BUTTON_TIMEOUT_MS, unclick, NULL);
  }
  else {
    bool rescheduled = app_timer_reschedule(button_timer, BUTTON_TIMEOUT_MS);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "rescheduled");
    if (!rescheduled) {
      // ...not really sure how we got here
      debug_log("button_timer not NULL, but timer not rescheduled!", __LINE__);
      // just try again?
      button_timer = NULL;
      set_click_timer();
    }
  }

}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Select clicked");
  set_click_timer();
  combine_buttons(RIGHT);
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Up clicked");
  set_click_timer();
  combine_buttons(UP);

}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Down clicked");
  set_click_timer();
  combine_buttons(DOWN);
}

static void prv_back_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Back clicked");
  set_click_timer();
  combine_buttons(LEFT);
}


static void prv_click_config_provider(void *context) {

  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, prv_back_click_handler);
}



GColor get_color(int8_t board_value) {
  // colors to pick from
 const GColor colors[MAX_NUMBER + 1] = {
    GColorBlack, // the color of 0 is black - so it'll be invisible
    GColorBlue,
    GColorCyan,
    GColorGreen,
    GColorSpringBud,
    GColorYellow,
    GColorOrange,
    GColorRed,
    GColorFolly
  };

  if (board_value >= 0 && board_value < MAX_NUMBER + 1) {
    return COLOR_FALLBACK(colors[board_value], board_value == 0? GColorBlack: GColorWhite);
  }
  debug_log("Invalid board_value in get_color", __LINE__);
  return GColorWhite;
}

GPoint board_coordinate_to_gpoint(int32_t x, int32_t y) {
  x = x * CHARACTER_WIDTH;
  y = y * CHARACTER_HEIGHT;
  if (x > INT16_MAX || x < INT16_MIN || y > INT16_MAX || y < INT16_MIN) {
    debug_log("board_coordinate_to_gpoint called with cooridinate that ended up outside int16_t range!", __LINE__);
  }

  return (GPoint) { .x = x, .y = y };
}

// The font has a top margin of 6px, just fudge it away for the sake of drawing text
GRect fudge_top_margin(GRect rect) {
  rect.origin.y -= 6;
  return rect;
}

// Adjust the rectangle to match the fudge above
GRect fudge_rectangle(GRect rect) {
  rect.origin.y -= 1;
  rect.size.w -= 1;
  return rect;
}

void draw_board(GContext *ctx, const GFont font)
{
  for (int32_t y = 1; y < BOARD_HEIGHT - 1; y++) {
    for (int32_t x = 1; x < BOARD_WIDTH - 1; x++) {
      int8_t board_value = board_get(x, y);
      if (board_value < 0) {
        debug_log("Tried to draw the board out of bounds!", __LINE__);
        continue; // just skip it
      }

      GRect text_bounds = { .origin = board_coordinate_to_gpoint(x - 1, y - 1), .size = { .w = CHARACTER_WIDTH, .h = CHARACTER_HEIGHT } };
      char text_buffer[2] = { board_value + '0', '\0' };
      graphics_context_set_text_color(ctx, get_color(board_value));
      graphics_draw_text(ctx, text_buffer, font, fudge_top_margin(text_bounds), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }
  }
}

void draw_player(GContext *ctx, struct player player) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Drawing player at %d, %d", player.x, player.y);
  GPoint text_gpoint = board_coordinate_to_gpoint(player.x - 1, player.y - 1);
  GRect text_bounds = { .origin = text_gpoint, .size = { .w = CHARACTER_WIDTH, .h = CHARACTER_HEIGHT } };
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, fudge_rectangle(text_bounds), 0, GCornerNone);
}


void draw_text()
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Drawing text: %s", gameover_text);
  GFont text_font = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
  GRect text_bounds = { .origin = { 0 }, .size = { .w = PBL_DISPLAY_WIDTH, .h = PBL_DISPLAY_HEIGHT } };
  graphics_context_set_fill_color(context, GColorBlack);    
  graphics_fill_rect(context, text_bounds, 0, GCornerNone);
  graphics_context_set_text_color(context, GColorWhite);
  graphics_draw_text(context, gameover_text, text_font, text_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

// Source - https://stackoverflow.com/a/41885173
// Posted by chqrlie, modified by community. See post 'Timeline' for change history
// Retrieved 2026-09-09, License - CC BY-SA 4.0
char *safe_strcpy(char* restrict dest, size_t size, const char* restrict src) {
    if (size > 0) {
        size_t i;
        for (i = 0; i < size - 1 && src[i]; i++) {
             dest[i] = src[i];
        }
        dest[i] = '\0';
    }
    return dest;
}


void set_gameover_text(const char *text) {
  safe_strcpy(gameover_text, sizeof(gameover_text), text);
}

void set_gameover(bool value) {
  gameover = value;
}


static void prv_window_load(Window *window) {

}

static void prv_window_unload(Window *window) {
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  context = ctx;
  GFont numbers_font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);
  draw_board(ctx, numbers_font);
  draw_player(ctx, get_player());
  if (gameover) {
    draw_text();
  }
}


static void prv_init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });

  const bool animated = true;
  window_set_background_color(s_window, GColorBlack);
  window_stack_push(s_window, animated);

  setup();
  
  
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_get_root_layer(s_window), s_canvas_layer);
  layer_mark_dirty(s_canvas_layer);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done loading the window");


}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
