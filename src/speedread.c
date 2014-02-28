#include <pebble.h>
#include "sequence.h"

static Window *window;
static TextLayer *text_layer;
static TextLayer *info_layer;
static AppTimer *text_timer;

static GFont small_font;
static GFont reg_font;

// static const char* SAMPLE_WORDS = "This is an example sentence to test speed reading. Maybe, if there is punctua-tion, the sentence will be different?";

static const char* SAMPLE_WORDS = "But I must explain to you how all this mistaken idea of denouncing pleasure and praising pain was born and I will give you a complete account of the system, and expound the actual teachings of the great explorer of the truth, the master-builder of human happiness. No one rejects, dislikes, or avoids pleasure itself, because it is pleasure, but because those who do not know how to pursue pleasure rationally encounter consequences that are extremely painful. Nor again is there anyone who loves or pursues or desires to obtain pain of itself, because it is pain, but because occasionally circumstances occur in which toil and pain can procure him some great pleasure. To take a trivial example, which of us ever undertakes laborious physical exercise, except to obtain some advantage from it? But who has any right to find fault with a man who chooses to enjoy a pleasure that has no annoying consequences, or one who avoids a pain that produces no resultant pleasure?";

// static const char *SAMPLE_WORDS = "aaaaaaaaaa bbbbbbbbbb cccccccccc dddddddddd eeeeeeeeee ffffffffff gggggggggg hhhhhhhhhh iiiiiiiiii jjjjjjjjjj kkkkkkkkkk llllllllll mmmmmmmmmm nnnnnnnnnn oooooooooo pppppppppp qqqqqqqqqq rrrrrrrrrr ssssssssss tttttttttt uuuuuuuuuu vvvvvvvvvv wwwwwwwwww xxxxxxxxxx yyyyyyyyyy zzzzzzzzzz 0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666 7777777777 8888888888 9999999999";

// static const char *SAMPLE_WORDS = "here are some? short! words! with which I can do! some tests";

static word *seq;
static uint16_t nwords;

static uint32_t base_wpm = 250;
static uint16_t w = 0;
static bool running = false;

static void text_timer_callback(void *data) {
  word *curr = (word *) data;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", curr->text);

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

  text_layer = text_layer_create((GRect) { .origin = { 1, 72 }, .size = { bounds.size.w, 36 } });
  text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  // text_layer_set_font(text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DROID_SANS_MONO_24)));
  // text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_font(text_layer, reg_font);

  info_layer = text_layer_create((GRect) { .origin = { 0, 10 }, .size = { bounds.size.w, 32 } });
  text_layer_set_text_alignment(info_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(info_layer));

  text_layer_set_font(info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  // text_layer_set_font(info_layer, small_font);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(info_layer);
}

static void init(void) {
  nwords = build_sequence(&seq, SAMPLE_WORDS);

  reg_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SOURCE_CODE_PRO_MEDIUM_24));
  small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SOURCE_CODE_PRO_EXTRALIGHT_24));

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
