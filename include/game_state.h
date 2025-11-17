#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>

// Game state structure
typedef struct {
    uint8_t lives;
    uint16_t score;
    uint8_t level;
} GameState;

// Global game state
extern GameState game_state;

/**
 * @brief Initialize the game state with default values
 */
void game_state_init(void);

/**
 * @brief Reset the game state for a new game
 */
void game_state_reset(void);

/**
 * @brief Add points to the score
 *
 * @param points Number of points to add
 */
void game_add_score(uint16_t points);

/**
 * @brief Lose a life
 *
 * @return uint8_t Remaining lives (0 if game over)
 */
uint8_t game_lose_life(void);

#endif // GAME_STATE_H
