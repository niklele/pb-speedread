#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static AppTimer *timer;

// #define COMMA_PAUSE 5
// #define COLON_PAUSE 5
// #define SEMICOLON_PAUSE 7
// #define PERIOD_PAUSE 10
// #define EXCLAMATION_PAUSE 10
// #define QUESTION_PAUSE 10

typedef struct word {
  char *text;
  uint8_t center; // pos of highlighted character
  uint8_t pause; // longer if it contains punctuation
} word;

// needs to end in a space
static const char* SAMPLE_WORDS = "This is an example sentence to test speed reading. Maybe, if there is punctuation, the sentence will be different? ";
static word *seq;
static uint16_t nwords;

void set_center(word *w, uint16_t wlen) {
  w->center = (wlen / 2) - 1; // naive 50% position
}

void set_pause(word *w, uint16_t wlen) {
  char last = w->text[wlen - 1];
  switch (last) {
    case ',':
    case ':':
      w->pause = 100;
      break;
    case ';':
      w->pause = 150;
      break;
    case '.':
    case '!':
    case '?':
      w->pause = 200;
      break;
    default:
      w->pause = 0;
      break;
  }
}

static uint16_t sequence_input(word **sequence, const char *input) {
  uint16_t len = (uint16_t) strlen(input);

  uint16_t nwords = 0;
  uint16_t word_ends[50]; // TODO decide size
  for (uint16_t i=0; i<len; ++i) {
    if (input[i] == ' ') {
      word_ends[nwords] = i;
      ++nwords;
    }
  }

  *sequence = (word *) malloc(nwords * sizeof(word));
  word *seq = *sequence;

  uint16_t wstart = 0;
  for (uint16_t j=0; j<nwords; ++j) {
    // make new word from wstart to word_ends[j]
    uint16_t wlen = word_ends[j] - wstart;
    seq[j].text = malloc(wlen + 1);
    strncpy(seq[j].text, &input[wstart], wlen);

    set_center(&seq[j], wlen);
    set_pause(&seq[j], wlen);

    wstart = word_ends[j] + 1;
  }

  return nwords;
}

// static void update_text() {
//   for (int i = 0; i < nwords; ++i)
//   {
//     text_layer_set_text(text_layer, seq[i].text);
//     uint16_t sleep = 500 + seq[i].pause;

//     psleep(sleep);
//   }
  
// }

#define SPEED 300
static uint16_t w = 0;
static void timer_callback(void *data) {

  text_layer_set_text(text_layer, seq[w].text);
  uint16_t sleep = SPEED + seq[w].pause;

  if (w < nwords) {
    ++w;
    timer = app_timer_register(sleep, timer_callback, NULL);
  }
  else {
    app_timer_cancel(timer);
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // text_layer_set_text(text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // text_layer_set_text(text_layer, "Down");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  // text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  nwords = sequence_input(&seq, SAMPLE_WORDS);

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  timer = app_timer_register(500, timer_callback, NULL);
}

static void deinit(void) {
  for (int i = 0; i < nwords; ++i) {
    free((void *) seq[i].text);
  }
  free((void *) seq);

  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
