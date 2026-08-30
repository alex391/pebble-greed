#include <pebble.h>
#include "game.h"

#define CHARACTER_WIDTH 13
#define CHARACTER_HEIGHT 17
#define CHARACTER_KERNING 3 /* this might not get used */

static Window *s_window;

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

// helper for logging, with a line number
void debug_log(const char *message, int32_t line) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%s: line %d", message, line);
}

// The idea here is to just print a message to the console and then just do
// nothing forever because something irricoverably bad happened
void log_and_spin(const char *message, int32_t line) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "%s: %d", message, line);
  while(true);
}

void draw_board(GContext *ctx, const GFont font)
{
  for (int32_t y = 1; y < BOARD_HEIGHT - 1; y++) {
    for (int32_t x = 1; x < BOARD_WIDTH - 1; x++) {
      volatile int8_t board_value = board_get(x, y);
      if (board_value < 0) {
        debug_log("Tried to draw the board out of bounds!", __LINE__);
        continue; // just skip it
      }
      GRect text_bounds = { .origin = { .x = (x - 1) * CHARACTER_WIDTH, .y = (y - 1) * CHARACTER_HEIGHT }, .size = { .w = CHARACTER_WIDTH, .h = CHARACTER_HEIGHT } };
      char text_buffer[2] = { board_value + '0', '\0' };
      graphics_draw_text(ctx, text_buffer, font, text_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }
  }
}


static void prv_window_load(Window *window) {
}

static void prv_window_unload(Window *window) {
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GFont numbers_font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS); // TODO this font's kinda small
  graphics_context_set_text_color(ctx, GColorBlack);

  draw_board(ctx, numbers_font);
}


static void prv_init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });

  const bool animated = true;
  window_stack_push(s_window, animated);

  setup();
  
  static Layer *s_canvas_layer;
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_get_root_layer(s_window), s_canvas_layer);
  layer_mark_dirty(s_canvas_layer);


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
