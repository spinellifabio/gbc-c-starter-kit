#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <stdint.h>
#include "win_screen.h"

typedef enum {
    GAME_RESULT_WIN = 0u,
    GAME_RESULT_GAME_OVER
} GameplayResult;

GameplayResult gameplay_screen(void);
void gameplay_signal_game_over(void);

/**
 * @brief Retrieves the reason for the win condition
 * @return WinReason enum value indicating why the player won
 */
WinReason gameplay_get_win_reason(void);

#endif /* GAMEPLAY_H */
