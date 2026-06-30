#include "palette.h"

#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>

/* =========================================================
 * WRAM state  (2 bytes — data-model.md)
 * ========================================================= */
static uint8_t bkg_slots_used; /* bitmask: bit N = BG palette slot N in use  */
static uint8_t obj_slots_used; /* bitmask: bit N = OBJ palette slot N in use */

/* ── Helpers ─────────────────────────────────────────────── */

static uint8_t slot_bit(uint8_t slot) {
    return (uint8_t)(1u << slot);
}

/* =========================================================
 * Public API
 * ========================================================= */

void palette_init(void) {
    bkg_slots_used = 0u;
    obj_slots_used = 0u;
}

uint8_t palette_set_bkg(uint8_t slot, const palette_color_t* colors) {
    if (slot > 7u) return PALETTE_NONE;
    if (_cpu == CGB_TYPE && colors) {
        set_bkg_palette(slot, 1u, colors);
    }
    bkg_slots_used |= slot_bit(slot);
    return slot;
}

uint8_t palette_set_obj(uint8_t slot, const palette_color_t* colors) {
    if (slot > 7u) return PALETTE_NONE;
    if (_cpu == CGB_TYPE && colors) {
        set_sprite_palette(slot, 1u, colors);
    }
    obj_slots_used |= slot_bit(slot);
    return slot;
}

uint8_t palette_alloc_bkg(const palette_color_t* colors) {
    uint8_t s;
    for (s = 0u; s < 8u; s++) {
        if (!(bkg_slots_used & slot_bit(s))) {
            return palette_set_bkg(s, colors);
        }
    }
    return PALETTE_NONE;
}

uint8_t palette_alloc_obj(const palette_color_t* colors) {
    uint8_t s;
    for (s = 0u; s < 8u; s++) {
        if (!(obj_slots_used & slot_bit(s))) {
            return palette_set_obj(s, colors);
        }
    }
    return PALETTE_NONE;
}

void palette_free(uint8_t kind, uint8_t slot) {
    if (slot > 7u) return;
    if (kind == PALETTE_KIND_BKG) {
        bkg_slots_used &= (uint8_t)~slot_bit(slot);
    } else {
        obj_slots_used &= (uint8_t)~slot_bit(slot);
    }
}

void palette_dmg_grayscale(void) {
    BGP_REG  = 0xE4u;
    OBP0_REG = 0xE4u;
    OBP1_REG = 0xE4u;
}
