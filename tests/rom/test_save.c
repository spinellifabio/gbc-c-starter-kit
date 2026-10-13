/* Headless save-system checks (see tests/test_rom.py). */
#include <string.h>

#include "harness.h"

#include "game_settings.h"
#include "game_state.h"
#include "save.h"

static void fill(SaveSlot *s) {
    memset(s, 0, sizeof(SaveSlot));
    s->language   = LANG_IT;
    s->difficulty = 2u;
    s->lives      = 3u;
    s->level      = 4u;
    s->score      = 0x1234u;
    s->flags      = SAVE_FLAG_TREASURE;
}

void main(void) {
    SaveSlot in, out;
    uint8_t i;

    for (i = 0u; i < SAVE_SLOT_COUNT; i++) save_slot_erase(i);

    /* erased slots are empty; out-of-range index is rejected */
    CHECK(save_slot_status(0u) == SAVE_EMPTY);
    CHECK(save_slot_write(SAVE_SLOT_COUNT, &in) == 0u);

    /* write -> read round-trips every field and stamps slot_id */
    fill(&in);
    CHECK(save_slot_write(1u, &in) == 1u);
    CHECK(save_slot_read(1u, &out) == SAVE_VALID);
    CHECK(out.slot_id == 1u && out.lives == 3u && out.level == 4u &&
          out.score == 0x1234u && out.flags == SAVE_FLAG_TREASURE &&
          out.language == LANG_IT && out.difficulty == 2u);

    /* slots are independent */
    CHECK(save_slot_status(0u) == SAVE_EMPTY);

    /* a flipped payload byte breaks the checksum */
    ENABLE_RAM; SWITCH_RAM(0);
    SRAM[SAVE_SLOT_STRIDE + 10u] ^= 0x01u;  /* slot 1, score low byte */
    DISABLE_RAM;
    CHECK(save_slot_status(1u) == SAVE_CORRUPTED);

    /* unknown version is corrupted, not valid */
    fill(&in);
    save_slot_write(2u, &in);
    ENABLE_RAM; SWITCH_RAM(0);
    SRAM[2u * SAVE_SLOT_STRIDE + 4u] = (uint8_t)(SAVE_FORMAT_VERSION + 1u);
    DISABLE_RAM;
    CHECK(save_slot_status(2u) == SAVE_CORRUPTED);

    /* erase returns a slot to empty */
    save_slot_erase(2u);
    CHECK(save_slot_status(2u) == SAVE_EMPTY);

    /* game state -> slot -> game state keeps lives/score/flags */
    game_state.lives = 2u; game_state.score = 300u; game_state.level = 5u;
    game_state.has_treasure = 0u; game_state.encountered_hazard = 1u;
    save_autosave(0u);
    game_state_reset();
    CHECK(save_slot_read(0u, &out) == SAVE_VALID);
    save_to_state(&out);
    CHECK(game_state.lives == 2u && game_state.score == 300u && game_state.level == 5u &&
          game_state.has_treasure == 0u && game_state.encountered_hazard == 1u);

    test_finish();
}
