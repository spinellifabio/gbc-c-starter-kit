#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

typedef enum {
    SFX_UI_MOVE   = 0,
    SFX_UI_SELECT = 1,
    SFX_UI_BACK   = 2,
    SFX_STEP      = 3,
    SFX_PICKUP    = 4,
    SFX_ERROR     = 5,
    SFX_COUNT
} SfxId;

void audio_init(void);
void audio_set_enabled(uint8_t enabled);
void sfx_play(SfxId id);

#endif /* AUDIO_H */
