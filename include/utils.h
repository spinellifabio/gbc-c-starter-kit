#ifndef UTILS_H
#define UTILS_H

#include <gb/gb.h>
#include <gbdk/console.h>
#include <stdio.h>
#include <string.h>
/* Fixed: ensure fixed-width integer types are available for prototypes */
#include <stdint.h>
#include <gb/cgb.h>

// Scene transition utilities
void scene_init_clean(void);

/* Font base-tile derivation (Constitution III — no ad-hoc char->tile writes).
 * font_load() picks the tile the font actually loads at; callers that turn a
 * character into a BG tile index MUST go through text_char_tile() instead of
 * assuming ASCII == tile index. Storage lives in utils.c (single instance);
 * text_font_base() is the only way to read it, so every translation unit
 * observes the same value. */
void text_set_font_base(uint8_t first_tile);
uint8_t text_font_base(void);
uint8_t text_char_tile(char c);

/* Sets CGB background palette 0 to a solid color (no-op on DMG).
 * Call before cls()/clear_screen() in scenes without custom BG art. */
void set_solid_bkg(palette_color_t color);

/* Per-scene solid background colors */
#define BKG_COLOR_SPLASH    RGB8(  0,  0, 80)   /* dark blue   — cinematic */
#define BKG_COLOR_TITLE     RGB8( 24,  0, 64)   /* dark purple — title */
#define BKG_COLOR_OPTIONS   RGB8(  0, 48, 16)   /* dark green  — menu */
#define BKG_COLOR_GAMEOVER  RGB8( 64,  0,  0)   /* dark red    — danger */
#define BKG_COLOR_WIN       RGB8( 48, 40,  0)   /* dark gold   — victory */
#define BKG_COLOR_CREDITS   RGB8(  0, 32, 48)   /* dark teal   — end roll */
void fade_to_black(uint8_t frames);
void fade_from_black(uint8_t frames);

/**
 * @brief Waits for any button press and clears the input buffer
 */
void wait_any_key(void);

/**
 * @brief Clears the background attribute map (CGB palettes/tiles) to prevent bleed between scenes.
 * No-op on DMG.
 */
void clear_attr_map(void);

/**
 * @brief Clears the screen by filling it with space characters.
 * Uses fill_bkg_rect (single DMA-friendly call) instead of 360 individual tile writes.
 */
void clear_screen(void);

/**
 * @brief Converts a number to a string
 *
 * @param value The number to convert
 * @param buffer Buffer to store the result (must be large enough)
 * @param min_digits Minimum number of digits (will be padded with zeros)
 */
void int_to_str(uint16_t value, char *buffer, uint8_t min_digits);

/**
 * @brief Prints text centered on a specific row
 *
 * @param text The text to print
 * @param row The row to print on (0-17)
 */
void print_centered(const char *text, uint8_t row);

#endif // UTILS_H
