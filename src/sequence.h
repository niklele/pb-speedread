
typedef struct word {
  char *text;
  uint8_t center; // pos of highlighted character
  uint16_t pause; // longer if it contains punctuation
} word;

// set the 'center point' which will always be displayed at the same location
void set_center(word *w, uint16_t wlen) {
  w->center = (wlen - 1) / 2;
}

// set the additional delay based on the last character
void set_pause(word *w, char last) {
  // char last = w->text[wlen - 1];
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
static const char *spaces = "    ";

// set the fields of a word struct
static void make_word(word *w, const char *input, uint16_t wlen) {

  set_center(w, wlen);
  uint8_t num_spaces = (w->center <= 4) ? (4 - w->center) : 0;

  w->text = malloc(wlen + num_spaces + 1);
  
  strncpy(w->text, spaces, num_spaces);
  strncpy(w->text + num_spaces, input, wlen);

  set_pause(w, input[wlen-1]);
}

// copy from an input buffer to construct the sequence array
static uint16_t build_sequence(word **sequence, const char *input) {
  uint16_t len = (uint16_t) strlen(input);

  // count words
  uint16_t nwords = 0;
  uint16_t word_ends[100]; // TODO decide size
  for (uint16_t i=0; i<len; ++i) {
    if (input[i] == ' ') {
      word_ends[nwords] = i;
      ++nwords;
    } else if (input[i] == '-') {
      word_ends[nwords] = i; // if this is set to +1 we keep the - but lose the next char
      ++nwords;
    }
    else if (i == len-1) {
      word_ends[nwords] = len;
      ++nwords;
    }
  }

  // build sequence
  *sequence = (word *) malloc(nwords * sizeof(word));
  word *seq = *sequence;

  uint16_t wstart = 0;
  for (uint16_t j=0; j<nwords; ++j) {
    // make new word from wstart to word_ends[j]
    uint16_t wlen = word_ends[j] - wstart;
    make_word(&seq[j], &input[wstart], wlen);
    wstart = word_ends[j] + 1;
  }

  return nwords;
}

static void destroy_sequence(word *seq, uint16_t nwords) {
  for (uint16_t i = 0; i < nwords; ++i) {
    free((void *) seq[i].text);
  }
  free((void *) seq);
}

