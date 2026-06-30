#ifndef PALETTE_H
#define PALETTE_H

#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>

/* Returned by palette_alloc_* / palette_set_* when no slot is available. */
#define PALETTE_NONE    0xFFu

/* palette_free() kind parameter */
#define PALETTE_KIND_BKG 0u
#define PALETTE_KIND_OBJ 1u

/* Clear slot-usage bitmaps. Call once from game_system_init(). */
void palette_init(void);

/*
 * Assign colors to an explicit BG or OBJ hardware palette slot (0-7).
 * On CGB: writes the palette via GBDK. On DMG: no-op for color write.
 * Marks the slot used in the occupancy bitmap.
 * Returns slot on success, PALETTE_NONE if slot > 7 or colors is NULL.
 */
uint8_t palette_set_bkg(uint8_t slot, const palette_color_t* colors);
uint8_t palette_set_obj(uint8_t slot, const palette_color_t* colors);

/*
 * Find the first free BG or OBJ slot and assign colors to it.
 * Returns the allocated slot, or PALETTE_NONE if all 8 are in use.
 */
uint8_t palette_alloc_bkg(const palette_color_t* colors);
uint8_t palette_alloc_obj(const palette_color_t* colors);

/* Release a slot for reuse. kind = PALETTE_KIND_BKG or PALETTE_KIND_OBJ. */
void palette_free(uint8_t kind, uint8_t slot);

/* Set BGP/OBP0/OBP1 to standard grayscale (DMG fallback). */
void palette_dmg_grayscale(void);

#endif /* PALETTE_H */
