#include "bgm.h"
#include "audio.h"

#include <gb/gb.h>
#include <stdint.h>

/* Frequencies from examples/sound/sound.c LUT:
 *   C3=36→1798=0x706, D3=38→1825=0x721, E3=40→1849=0x739
 *   G3=43→1881=0x759, A3=45→1899=0x76B, C4=48→1923=0x783
 */
static const BgmNote demo_song[] = {
    { 0x706u, 12u },  /* C3 */
    { 0x739u, 12u },  /* E3 */
    { 0x759u, 12u },  /* G3 */
    { 0x783u, 12u },  /* C4 */
    { 0x783u, 12u },  /* C4 */
    { 0x759u, 12u },  /* G3 */
    { 0x739u, 12u },  /* E3 */
    { 0x706u, 24u },  /* C3 long */
    { 0x000u, 12u },  /* rest */
    BGM_NOTE_END
};

static const BgmNote *bgm_song   = 0;
static uint8_t        bgm_idx    = 0u;
static uint8_t        bgm_ticks  = 0u;
static uint8_t        bgm_paused = 0u;

static void silence_pulse1(void) {
    NR12_REG = 0x00u;
    NR14_REG = 0x80u;
}

static void play_note(const BgmNote *n) {
    if (n->freq == 0u) {
        silence_pulse1();
        return;
    }
    NR10_REG = 0x00u;                             /* no sweep */
    NR11_REG = 0x80u;                             /* 50% duty, no length */
    NR12_REG = 0xF0u;                             /* vol 15, no envelope */
    NR13_REG = (uint8_t)(n->freq & 0xFFu);
    NR14_REG = (uint8_t)(0x80u | ((n->freq >> 8u) & 0x07u));  /* trigger */
}

void bgm_play(const BgmNote *song) {
    if (!(NR52_REG & 0x80u)) return;
    bgm_song   = song;
    bgm_idx    = 0u;
    bgm_paused = 0u;
    play_note(&song[0]);
    bgm_ticks = song[0].duration;
}

void bgm_play_title_theme(void) {
    bgm_play(demo_song);
}

void bgm_stop(void) {
    bgm_song = 0;
    silence_pulse1();
}

/* Silence the channel too — otherwise a note with no length/envelope decay
 * (see play_note()) keeps sounding indefinitely while paused, producing a
 * stuck drone on any screen that pauses BGM (e.g. options). */
void bgm_pause(void)  { bgm_paused = 1u; silence_pulse1(); }
void bgm_resume(void) { bgm_paused = 0u; }

void bgm_vbl_update(void) {
    if (!bgm_song || bgm_paused || !(NR52_REG & 0x80u)) return;

    if (bgm_ticks > 0u) { bgm_ticks--; return; }

    bgm_idx++;
    const BgmNote *note = &bgm_song[bgm_idx];
    if (note->duration == 0u) { bgm_idx = 0u; note = bgm_song; }
    play_note(note);
    bgm_ticks = note->duration;
}
