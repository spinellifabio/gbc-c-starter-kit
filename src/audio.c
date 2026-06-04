#include "audio.h"

#include <gb/gb.h>
#include <stdint.h>

typedef struct {
    uint8_t nr21;  /* Pulse2: duty (bits 7-6) + length (bits 5-0) */
    uint8_t nr22;  /* Pulse2: envelope init vol (bits 7-4), dir (bit 3), steps (bits 2-0) */
    uint8_t nr23;  /* Pulse2: frequency low byte */
    uint8_t nr24;  /* Pulse2: trigger (bit 7) + freq high (bits 2-0) */
} SfxPreset;

/* Frequencies from examples/sound/sound.c LUT (GB freq register values):
 *   C4 = index 48 = 1923 = 0x783  → nr23=0x83, nr24_high=7
 *   A4 = index 57 = 1978 = 0x7BA  → nr23=0xBA, nr24_high=7
 *   F3 = index 41 = 1860 = 0x744  → nr23=0x44, nr24_high=7
 *   D3 = index 38 = 1825 = 0x721  → nr23=0x21, nr24_high=7
 *   A5 = index 69 = 2013 = 0x7DD  → nr23=0xDD, nr24_high=7
 *   G2 = index 31 = 1694 = 0x69E  → nr23=0x9E, nr24_high=6
 */
static const SfxPreset sfx_table[SFX_COUNT] = {
    /* SFX_UI_MOVE:   C4, 50% duty, vol 8, fast decay (1 envelope step) */
    { 0x80u, 0x81u, 0x83u, 0x87u },
    /* SFX_UI_SELECT: A4, 75% duty, vol 12, fast decay */
    { 0xC0u, 0xC1u, 0xBAu, 0x87u },
    /* SFX_UI_BACK:   F3, 25% duty, vol 6, fast decay */
    { 0x40u, 0x61u, 0x44u, 0x87u },
    /* SFX_STEP:      D3, 12.5% duty, vol 5, fast decay — soft low click */
    { 0x00u, 0x51u, 0x21u, 0x87u },
    /* SFX_PICKUP:    A5, 75% duty, vol 10, fast decay — bright chirp */
    { 0xC0u, 0xA1u, 0xDDu, 0x87u },
    /* SFX_ERROR:     G2, 50% duty, vol 8, slow decay (7 steps) — low buzz */
    { 0x80u, 0x87u, 0x9Eu, 0x86u },
};

void audio_init(void) {
    NR52_REG = 0x80u;  /* APU on */
    NR50_REG = 0x77u;  /* max volume both speakers */
    NR51_REG = 0xFFu;  /* route all channels to both outputs */
}

void audio_set_enabled(uint8_t enabled) {
    if (enabled) {
        audio_init();
    } else {
        NR52_REG = 0x00u;  /* APU power off; all channel registers reset to 0 */
    }
}

void sfx_play(SfxId id) {
    if (!(NR52_REG & 0x80u)) return;
    const SfxPreset *p = &sfx_table[id];
    NR21_REG = p->nr21;
    NR22_REG = p->nr22;
    NR23_REG = p->nr23;
    NR24_REG = p->nr24;
}
