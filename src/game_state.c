#include "game_state.h"

// Global game state
GameState game_state;

// Slot the current run is bound to (default 0; overwritten by the slot menu).
uint8_t g_active_slot = 0u;

void game_state_init(void) {
    // Initialize with default values
    game_state.lives = 1;          // Single attempt
    game_state.score = 0;          // Starting score
    game_state.level = 1;          // Starting level
    game_state.has_treasure = 0;   // No treasure collected yet
    game_state.encountered_hazard = 0; // No hazard encountered yet
}

void game_state_reset(void) {
    // Reset to initial values (same as init in this case)
    game_state_init();
}

// Note: The score-related functions are already implemented in game_system.c
