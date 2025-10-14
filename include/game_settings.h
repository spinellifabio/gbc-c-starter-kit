#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include <gb/gb.h>

#include "lang.h"

typedef enum {
    MODE_RELEASE,
    MODE_DEBUG
} GameMode;

typedef struct {
    uint8_t sound_on;
    uint8_t difficulty;
    uint8_t lives;
    GameMode mode;
    Language language;
    const char *game_name;
    const char *version;
} GameSettings;

extern GameSettings g_settings;

#endif /* GAME_SETTINGS_H */
