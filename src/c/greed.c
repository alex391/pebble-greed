#include <pebble.h>
#include "game.h"

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

void draw_board()
{
  for (size_t y = 1; y < BOARD_HEIGHT - 1; y++) {
    for (size_t x = 1; x < BOARD_WIDTH - 1; x++) {
      graphics_draw_text(ctx, 
    }
  }
}

static void prv_window_load(Window *window) {
  // Layer *window_layer = window_get_root_layer(window);
  //GRect bounds = layer_get_bounds(window_layer);


}

static void prv_window_unload(Window *window) {
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
