#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>

// Game state structure
typedef struct {
    uint8_t lives;
    uint16_t score;
    uint8_t level;
    uint8_t has_treasure;  // Flag for treasure collection
    uint8_t encountered_hazard;  // Flag for hazard encounter
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

/* NOTE: game_add_score() is declared here but not yet implemented.
 * Implement in game_state.c when scoring logic is needed. */
void game_add_score(uint16_t points);

/* NOTE: game_lose_life() is implemented in game_objects.c (it triggers
 * object-layer dialogue). Declaration lives in game_objects.h. */

#endif // GAME_STATE_H
