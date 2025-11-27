#ifndef GAMEPLAY_H
#define GAMEPLAY_H

typedef enum {
    GAME_RESULT_WIN = 0u,
    GAME_RESULT_GAME_OVER
} GameplayResult;

GameplayResult gameplay_screen(void);
void gameplay_signal_game_over(void);

#endif /* GAMEPLAY_H */
