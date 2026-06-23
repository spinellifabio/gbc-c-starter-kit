#include "save.h"

#include <gb/gb.h>
#include <stdint.h>
#include <string.h>

#include "game_settings.h"
#include "game_state.h"
#include "lang.h"

/* SRAM base; bank 0 holds all three slots (3 * 32 = 96 bytes). */
#define SRAM_BASE ((uint8_t *)0xA000)

/* Compile-time guard: SaveSlot must be exactly one slot stride. */
typedef char save_slot_size_check[(sizeof(SaveSlot) == SAVE_SLOT_STRIDE) ? 1 : -1];

static const char SAVE_MAGIC[4] = { 'G', 'B', 'C', 'P' };

/* 16-bit sum over the first (SAVE_SLOT_STRIDE-2) bytes of a 32-byte record. */
static uint16_t compute_checksum(const uint8_t *p) {
    uint16_t sum = 0u;
    for (uint8_t i = 0u; i < (SAVE_SLOT_STRIDE - 2u); i++) {
        sum = (uint16_t)(sum + p[i]);
    }
    return sum;
}

/* Copy 32 bytes from slot i in SRAM into dst. */
static void sram_read_slot(uint8_t i, uint8_t *dst) {
    __critical {
        ENABLE_RAM;
        SWITCH_RAM(0);
        memcpy(dst, SRAM_BASE + (uint16_t)(i * SAVE_SLOT_STRIDE), SAVE_SLOT_STRIDE);
        DISABLE_RAM;
    }
}

/* Copy 32 bytes from src into slot i in SRAM. */
static void sram_write_slot(uint8_t i, const uint8_t *src) {
    __critical {
        ENABLE_RAM;
        SWITCH_RAM(0);
        memcpy(SRAM_BASE + (uint16_t)(i * SAVE_SLOT_STRIDE), src, SAVE_SLOT_STRIDE);
        DISABLE_RAM;
    }
}

/* Classify a 32-byte record already copied out of SRAM. */
static SaveStatus classify(const uint8_t *buf) {
    const SaveSlot *s = (const SaveSlot *)buf;
    if (memcmp(s->magic, SAVE_MAGIC, 4) != 0) {
        return SAVE_EMPTY;
    }
    if (s->version != SAVE_FORMAT_VERSION) {
        return SAVE_CORRUPTED;
    }
    if (s->checksum != compute_checksum(buf)) {
        return SAVE_CORRUPTED;
    }
    return SAVE_VALID;
}

void save_init(void) {
    /* Touch SRAM once so the MBC RAM gate is exercised, then leave disabled. */
    __critical {
        ENABLE_RAM;
        SWITCH_RAM(0);
        DISABLE_RAM;
    }
}

SaveStatus save_slot_status(uint8_t i) {
    uint8_t buf[SAVE_SLOT_STRIDE];
    if (i >= SAVE_SLOT_COUNT) return SAVE_EMPTY;
    sram_read_slot(i, buf);
    return classify(buf);
}

SaveStatus save_slot_read(uint8_t i, SaveSlot *out) {
    uint8_t buf[SAVE_SLOT_STRIDE];
    SaveStatus status;
    if (i >= SAVE_SLOT_COUNT) return SAVE_EMPTY;
    sram_read_slot(i, buf);
    status = classify(buf);
    if (status == SAVE_VALID) {
        memcpy(out, buf, SAVE_SLOT_STRIDE);
    }
    return status;
}

uint8_t save_slot_write(uint8_t i, SaveSlot *in) {
    if (i >= SAVE_SLOT_COUNT) return 0u;
    memcpy(in->magic, SAVE_MAGIC, 4);
    in->version = SAVE_FORMAT_VERSION;
    in->slot_id = i;
    in->checksum = compute_checksum((const uint8_t *)in);
    sram_write_slot(i, (const uint8_t *)in);
    return 1u;
}

void save_slot_erase(uint8_t i) {
    uint8_t buf[SAVE_SLOT_STRIDE];
    if (i >= SAVE_SLOT_COUNT) return;
    memset(buf, 0, SAVE_SLOT_STRIDE);
    sram_write_slot(i, buf);
}

void save_from_state(SaveSlot *out, uint8_t slot_id) {
    memset(out, 0, SAVE_SLOT_STRIDE);
    out->slot_id    = slot_id;
    out->language   = (uint8_t)g_settings.language;
    out->difficulty = g_settings.difficulty;
    out->lives      = game_state.lives;
    out->level      = game_state.level;
    out->score      = game_state.score;
    out->flags      = (uint8_t)((game_state.has_treasure ? SAVE_FLAG_TREASURE : 0u) |
                                (game_state.encountered_hazard ? SAVE_FLAG_HAZARD : 0u));
    out->seed       = 0u;
    out->play_count = 1u;  /* reserved field; not yet surfaced in the slot UI */
}

void save_to_state(const SaveSlot *in) {
    g_settings.language   = (Language)in->language;
    g_settings.difficulty = in->difficulty;
    game_state.lives      = in->lives;
    game_state.level      = in->level;
    game_state.score      = in->score;
    game_state.has_treasure      = (uint8_t)((in->flags & SAVE_FLAG_TREASURE) != 0u);
    game_state.encountered_hazard = (uint8_t)((in->flags & SAVE_FLAG_HAZARD) != 0u);
    lang_set(g_settings.language);
}

void save_autosave(uint8_t i) {
    SaveSlot slot;
    if (i >= SAVE_SLOT_COUNT) return;
    save_from_state(&slot, i);
    save_slot_write(i, &slot);
}
