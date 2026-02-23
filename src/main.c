/**
 *     $$$$$$\  $$$$$$$\   $$$$$$\         $$$$$$\         $$$$$$\    $$\                          $$\                               $$\   $$\ $$\   $$\
 *    $$  __$$\ $$  __$$\ $$  __$$\       $$  __$$\       $$  __$$\   $$ |                         $$ |                              $$ | $$  |\__|  $$ |
 *    $$ /  \__|$$ |  $$ |$$ /  \__|      $$ /  \__|      $$ /  \__|$$$$$$\    $$$$$$\   $$$$$$\ $$$$$$\    $$$$$$\   $$$$$$\        $$ |$$  / $$\ $$$$$$\
 *    $$ |$$$$\ $$$$$$$\ |$$ |            $$ |            \$$$$$$\  \_$$  _|   \____$$\ $$  __$$\\_$$  _|  $$  __$$\ $$  __$$\       $$$$$  /  $$ |\_$$  _|
 *    $$ |\_$$ |$$  __$$\ $$ |            $$ |             \____$$\   $$ |     $$$$$$$ |$$ |  \__| $$ |    $$$$$$$$ |$$ |  \__|      $$  $$<   $$ |  $$ |
 *    $$ |  $$ |$$ |  $$ |$$ |  $$\       $$ |  $$\       $$\   $$ |  $$ |$$\ $$  __$$ |$$ |       $$ |$$\ $$   ____|$$ |            $$ |\$$\  $$ |  $$ |$$\
 *    \$$$$$$  |$$$$$$$  |\$$$$$$  |      \$$$$$$  |      \$$$$$$  |  \$$$$  |\$$$$$$$ |$$ |       \$$$$  |\$$$$$$$\ $$ |            $$ | \$$\ $$ |  \$$$$  |
 *     \______/ \_______/  \______/        \______/        \______/    \____/  \_______|\__|        \____/  \_______|\__|            \__|  \__|\__|   \____/
 *
 *                                                                                                           v0.1.0 - Code assembled in spare time by Fabio
 */

#include "game_system.h"
#include "gameplay.h"
#include "intro.h"
#include "splash.h"
#include "title_screen.h"
#include "game_state.h"
#include "credits.h"
#include "game_over.h"
#include "win_screen.h"
#include "utils.h"

void main(void) {
    game_system_init();
    game_state_init();

    splash_sequence();
    intro_cut_scene();

    while (1) {
        title_screen();
        game_state_reset();
        GameplayResult result = gameplay_screen();

        /* Fade to black for clean scene transition */
        fade_to_black(12u);

        if (result == GAME_RESULT_GAME_OVER) {
            show_game_over_screen(GAME_OVER_DEFAULT, game_state.score);
        } else {
            show_win_screen(game_state.score, gameplay_get_win_reason());
        }

        credits_scene();
    }
}
