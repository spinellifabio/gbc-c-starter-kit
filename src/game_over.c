#include <gb/gb.h>
#include <stdio.h>
#include "game_over.h"
#include "game_settings.h"
#include "utils.h"

// Game over messages (will be moved to language system later)
static const char game_over_messages[][14] = {  // Longest string is 13 chars + null
    "GAME OVER",     // GAME_OVER_DEFAULT
    "TIME'S UP!",    // GAME_OVER_TIME_UP
    "NO MORE LIVES", // GAME_OVER_LIFE_LOST
    "CAUGHT!"        // GAME_OVER_CAUGHT
};

void show_game_over_screen(GameOverReason reason, uint16_t score) {
    // Clear screen and hide sprites
    clear_screen();
    HIDE_SPRITES;

    // Display game over message
    const char *message = game_over_messages[reason];
    print_centered(message, 6);

    // Display score if not zero
    if (score > 0) {
        char score_str[6]; // Up to 5 digits + null terminator
        int_to_str(score, score_str, 5);

        // Display "SCORE: " text
        const char score_text[] = "SCORE:";
        print_centered(score_text, 8);

        // Display score value
        gotoxy(13, 8);
        for (uint8_t i = 0; i < 5; i++) {
            printf("%c", score_str[i]);
        }
    }

    // Wait a moment before allowing input (60 frames = ~1 second)
    uint8_t wait_frames = 180; // 3 seconds at 60 FPS
    while (wait_frames--) {
        wait_vbl_done();
    }

    // Show continue prompt
    print_centered("PRESS START", 12);

    // Wait for any key
    wait_any_key();
}
