#include <pebble.h>
#include "sequence.h"

static Window *window;
static TextLayer *text_layer;
static TextLayer *info_layer;
static AppTimer *text_timer;

static const char* SAMPLE_WORDS = "This is an example sentence to test speed reading. Maybe, if there is punctuation, the sentence will be different?";
static word *seq;
static uint16_t nwords;

static uint32_t base_wpm = 250;
static uint16_t w = 0;
static bool running = false;

static void text_timer_callback(void *data) {
  word *curr = (word *) data;
  text_layer_set_text(text_layer, curr->text);
  uint16_t sleep = (60000/base_wpm) + curr->pause;

  if (w < (nwords-1)) {
    ++w;
    text_timer = app_timer_register(sleep, text_timer_callback, &seq[w]);
  }
  else {
    w = 0;
    running = false;
    app_timer_cancel(text_timer);
  }
}

static void start() {
  text_timer = app_timer_register((60000/base_wpm), text_timer_callback, &seq[w]);
}

static void pause() {
  app_timer_cancel(text_timer);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  running = !running;
  running ? start() : pause();
}

static char info[32];
static void display_info() {
  snprintf(info, 32, "WPM: %u\n", (unsigned int) base_wpm);
  text_layer_set_text(info_layer, info);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (base_wpm < 600) base_wpm += 50;
  display_info();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (base_wpm > 100) base_wpm -= 50;
  display_info();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 5, 72 }, .size = { bounds.size.w, 36 } });
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));

  info_layer = text_layer_create((GRect) { .origin = { 0, 20 }, .size = { bounds.size.w, 32 } });
  text_layer_set_text_alignment(info_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(info_layer));

  text_layer_set_font(info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(info_layer);
}

static void init(void) {
  nwords = build_sequence(&seq, SAMPLE_WORDS);

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  display_info();
}

static void deinit(void) {
  destroy_sequence(seq, nwords);
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
