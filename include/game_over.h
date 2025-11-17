#ifndef GAME_OVER_H
#define GAME_OVER_H

#include <gb/gb.h>

// Game over reasons
typedef enum {
    GAME_OVER_DEFAULT = 0,
    GAME_OVER_TIME_UP,
    GAME_OVER_LIFE_LOST,
    GAME_OVER_CAUGHT,
    GAME_OVER_COUNT  // Keep last to count the number of game over types
} GameOverReason;

/**
 * @brief Displays the game over screen with the specified reason
 *
 * @param reason The reason for game over (determines message)
 * @param score Final score to display
 */
void show_game_over_screen(GameOverReason reason, uint16_t score);

#endif // GAME_OVER_H
