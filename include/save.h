#ifndef SAVE_H
#define SAVE_H

#include <gb/gb.h>
#include <stdint.h>

/* === Save system: 3 battery-backed SRAM slots ===
 * Each slot is a fixed 32-byte record stored contiguously from 0xA000.
 * A slot is VALID only if magic + version match and the 16-bit checksum
 * over the first 30 bytes equals the stored checksum.
 */

#define SAVE_SLOT_COUNT      3u
#define SAVE_FORMAT_VERSION  1u
#define SAVE_SLOT_STRIDE     32u   /* bytes per slot (and sizeof(SaveSlot)) */

/* Progress flag bits (SaveSlot.flags) */
#define SAVE_FLAG_TREASURE   0x01u
#define SAVE_FLAG_HAZARD     0x02u

typedef enum {
    SAVE_EMPTY = 0,    /* no valid record */
    SAVE_VALID,        /* magic + checksum OK */
    SAVE_CORRUPTED     /* magic present but checksum/version mismatch */
} SaveStatus;

typedef struct {
    char     magic[4];     /* "GBCP" */
    uint8_t  version;      /* SAVE_FORMAT_VERSION */
    uint8_t  slot_id;      /* 0..SAVE_SLOT_COUNT-1 */
    uint8_t  language;     /* Language */
    uint8_t  difficulty;   /* g_settings.difficulty */
    uint8_t  lives;        /* game_state.lives */
    uint8_t  level;        /* game_state.level */
    uint16_t score;        /* game_state.score */
    uint8_t  flags;        /* SAVE_FLAG_* progress bits */
    uint16_t play_count;   /* incremented on each write (timestamp stand-in) */
    uint16_t seed;         /* RNG seed for resume */
    uint8_t  reserved[13]; /* pad to SAVE_SLOT_STRIDE-2 for future fields */
    uint16_t checksum;     /* 16-bit sum of bytes [0 .. SAVE_SLOT_STRIDE-3] */
} SaveSlot;

/**
 * @brief Probe SRAM once at boot. Safe to call before any save use.
 *        Leaves SRAM disabled afterward.
 */
void save_init(void);

/**
 * @brief Returns the status of slot i without copying the body.
 */
SaveStatus save_slot_status(uint8_t i);

/**
 * @brief Read slot i into *out. Returns SAVE_VALID/EMPTY/CORRUPTED.
 *        *out is only meaningful when SAVE_VALID.
 */
SaveStatus save_slot_read(uint8_t i, SaveSlot *out);

/**
 * @brief Stamp magic/version/slot_id, compute checksum, and write to SRAM.
 *        Returns 1 on success, 0 if i out of range.
 */
uint8_t save_slot_write(uint8_t i, SaveSlot *in);

/**
 * @brief Zero slot i in SRAM (→ SAVE_EMPTY).
 */
void save_slot_erase(uint8_t i);

/**
 * @brief Populate *out from current g_settings + game_state.
 */
void save_from_state(SaveSlot *out, uint8_t slot_id);

/**
 * @brief Apply a slot's fields to g_settings + game_state (and active language).
 */
void save_to_state(const SaveSlot *in);

/**
 * @brief Snapshot current state into slot i and persist it.
 */
void save_autosave(uint8_t i);

#endif /* SAVE_H */
