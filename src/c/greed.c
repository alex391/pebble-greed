#include <pebble.h>
#include "game.h"

#define CHARACTER_WIDTH 13
#define CHARACTER_HEIGHT 16

#define BUTTON_TIMEOUT_MS 500

static Window *s_window;
static Layer *s_canvas_layer;

AppTimer *button_timer = NULL;

void unclick(void *data) {
  button_timer = NULL;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "unclick");
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
      APP_LOG(APP_LOG_LEVEL_DEBUG, "button_timer not NULL, but timer not rescheduled!");
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
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Back clicked"); // TODO hown to make this go
  set_click_timer();
  combine_buttons(LEFT);
}


static void prv_click_config_provider(void *context) {

  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, prv_back_click_handler);
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

void draw_player(GContext *ctx, struct player player, bool blink_on, const GFont font) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Drawing player at %d, %d", player.x, player.y);
  GPoint text_gpoint = board_coordinate_to_gpoint(player.x - 1, player.y - 1);
  GRect text_bounds = { .origin = text_gpoint, .size = { .w = CHARACTER_WIDTH, .h = CHARACTER_HEIGHT } };
  const char *at = "9"; // TODO this is the wrong character
  if (blink_on) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, fudge_rectangle(text_bounds), 0, GCornerNone);
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, at, font, fudge_top_margin(text_bounds), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
  else {
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, at, font, fudge_top_margin(text_bounds), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }

}


static void prv_window_load(Window *window) {

}

static void prv_window_unload(Window *window) {
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GFont numbers_font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);
  //GFont player_font = 
  draw_board(ctx, numbers_font);
  draw_player(ctx, get_player(), true, numbers_font);
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
