#include <gb/gb.h>
#include <stdio.h>
#include "game_over.h"
#include "game_settings.h"
#include "utils.h"
#include "lang.h"

// Game over messages (will be moved to language system later)
static const char game_over_messages[][14] = {  // Longest string is 13 chars + null
    "GAME OVER",     // GAME_OVER_DEFAULT
    "TIME'S UP!",    // GAME_OVER_TIME_UP
    "NO MORE LIVES", // GAME_OVER_LIFE_LOST
    "CAUGHT!"        // GAME_OVER_CAUGHT
};

void show_game_over_screen(GameOverReason reason, uint16_t score) {
    /* Fade to black for clean transition */
    fade_to_black(12u);

    /* Initialize clean scene (disable LCD, hide sprites, clear VRAM, reset palettes) */
    scene_init_clean();

    /* Display game over title centered vertically at row 8 */
    const char *message = game_over_messages[reason];
    print_centered(message, 8u);

    /* Display score if not zero */
    if (score > 0u) {
        char score_str[6];  /* Up to 5 digits + null terminator */
        int_to_str(score, score_str, 5u);

        /* Display score label at row 10 */
        print_centered(lang_str(STR_SCORE_LABEL), 10u);

        /* Display score value next to label */
        gotoxy(13u, 10u);
        for (uint8_t i = 0u; i < 5u; i++) {
            printf("%c", score_str[i]);
        }
    }

    /* Fade in from black */
    fade_from_black(12u);

    /* Wait a moment before allowing input (180 frames = ~3 seconds at 60 FPS) */
    uint8_t wait_frames = 180u;
    while (wait_frames--) {
        wait_vbl_done();
    }

    /* Show continue prompt at row 16 (near bottom) */
    print_centered(lang_str(STR_PRESS_START), 16u);

    /* Wait for any key */
    wait_any_key();
}
