
typedef struct word {
  char *text;
  uint8_t center; // pos of highlighted character
  uint8_t pause; // longer if it contains punctuation
} word;

// set the 'center point' which will always be displayed at the same point
void set_center(word *w, uint16_t wlen) {
  w->center = (wlen / 2) - 1; // naive 50% position
}

// set the additional delay based on the last character
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

// set the fields of a word struct
static void make_word(word *w, const char *input, uint16_t wlen) {
  w->text = malloc(wlen + 1);
  strncpy(w->text, input, wlen);
  set_center(w, wlen);
  set_pause(w, wlen);
}

// copy from an input buffer to construct the sequence array
static uint16_t build_sequence(word **sequence, const char *input) {
  uint16_t len = (uint16_t) strlen(input);

  // count words
  uint16_t nwords = 0;
  uint16_t word_ends[50]; // TODO decide size
  for (uint16_t i=0; i<len; ++i) {
    if (input[i] == ' ') {
      word_ends[nwords] = i;
      ++nwords;
    } else if (i == len-1) {
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

