#ifndef WIN_SCREEN_H
#define WIN_SCREEN_H

#include <gb/gb.h>
#include <stdint.h>

// Win reasons
typedef enum {
    WIN_TREASURE_SECURED = 0,
    WIN_REASON_COUNT  // Keep last to count the number of win reasons
} WinReason;

/**
 * @brief Displays the win screen with the specified reason
 *
 * @param score Final score to display
 * @param reason The reason for winning (determines message)
 */
void show_win_screen(uint16_t score, WinReason reason);

#endif // WIN_SCREEN_H
