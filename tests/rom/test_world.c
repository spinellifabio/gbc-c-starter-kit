/* Headless lives/collision checks (see tests/test_rom.py). */
#include "harness.h"
#include "audio.h"
#include "dialogue.h"
#include "game_objects.h"
#include "game_state.h"
#include "gameplay.h"
#include "npc.h"

/* Link seams: keep audio/dialogue/gameplay out of this ROM. */
static uint8_t game_over_calls;
void sfx_play(SfxId id) { (void)id; }
void gameplay_signal_game_over(void) { game_over_calls++; }
void dialogue_show_text(const char *text) { (void)text; }
uint8_t dialogue_show_yes_no(const char *question) { (void)question; return DIALOGUE_RESULT_NO; }

void main(void) {
    GameObject *obj;

    /* lives: decrement, game over exactly once at 0, never below 0 */
    game_state.lives = 2u;
    CHECK(game_lose_life() == 1u && game_over_calls == 0u);
    CHECK(game_lose_life() == 0u && game_over_calls == 1u);
    CHECK(game_lose_life() == 0u && game_over_calls == 1u);

    /* NPC at (50,120), 16x16 hitbox */
    npc_init(1u, 50u, 120u, 0u, 0u, 0u);
    CHECK(npc_check_collision(1u, 55u, 125u, 8u, 8u) == 1u);   /* overlap */
    CHECK(npc_check_collision(1u, 42u, 120u, 8u, 16u) == 0u);  /* touching left edge */
    CHECK(npc_check_collision(1u, 66u, 120u, 8u, 16u) == 0u);  /* touching right edge */
    CHECK(npc_check_collision(1u, 43u, 120u, 8u, 16u) == 1u);  /* 1px inside */
    CHECK(npc_check_collision(0u, 55u, 125u, 8u, 8u) == 0u);   /* inactive slot */
    CHECK(npc_check_collision(MAX_NPCS, 55u, 125u, 8u, 8u) == 0u); /* index out of range */
    CHECK(npc_solid_at(55u, 125u, 8u, 8u) == 1u && npc_solid_at(0u, 0u, 8u, 8u) == 0u);

    /* world objects: treasure at (100,50) 16x16 */
    init_game_objects();
    obj = check_object_collision(105u, 55u, 4u, 4u);
    CHECK(obj != NULL && obj->type == OBJECT_TREASURE);
    obj->active = 0u;
    CHECK(check_object_collision(105u, 55u, 4u, 4u) == NULL);  /* inactive ignored */
    CHECK(check_object_collision(0u, 0u, 4u, 4u) == NULL);

    test_finish();
}
