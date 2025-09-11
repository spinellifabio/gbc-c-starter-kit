#ifndef LANG_H
#define LANG_H

#include <gb/gb.h>
#include <stdio.h>

// Supported languages
typedef enum {
    LANG_EN = 0,
    LANG_IT,
    LANG_COUNT
} Language;

// Initialize language system (default = English)
void lang_init(Language default_lang);

// Change current language
void lang_set(Language lang);

// Get active language
Language lang_get(void);

// Fetch dialog text for a given page index
// Returns pointer to string (NULL if out of range)
const char* lang_get_dialog(uint8_t page);

#endif
