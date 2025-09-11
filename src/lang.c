#include <gb/gb.h>
#include <stdio.h>

#include "lang.h"

// Current active language
static Language current_lang = LANG_EN;

// Example dialogs (RPG-style cutscene)
// Pages must end with NULL as terminator
static const char* dialogs_en[] = {
    "Welcome hero...",
    "The journey\nbegins now!",
    "Don't forget\nto press A!",
    NULL
};

static const char* dialogs_it[] = {
    "Benvenuto eroe...",
    "L'avventura\ninizia ora!",
    "Non dimenticare\ndi premere A!",
    NULL
};

// Array of pointers to each language set
static const char** dialogs_all[LANG_COUNT] = {
    dialogs_en,
    dialogs_it
};

void lang_init(Language default_lang) {
    if (default_lang < LANG_COUNT) {
        current_lang = default_lang;
    } else {
        current_lang = LANG_EN;
    }
}

void lang_set(Language lang) {
    if (lang < LANG_COUNT) {
        current_lang = lang;
    }
}

Language lang_get(void) {
    return current_lang;
}


const char* lang_get_dialog(uint8_t page) {
    const char** active_dialogs = dialogs_all[current_lang];
    if (!active_dialogs) return NULL;
    return active_dialogs[page];
}
