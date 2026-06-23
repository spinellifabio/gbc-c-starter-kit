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
#include "save.h"
#include "slot_menu.h"
#include "utils.h"

void main(void) {
    game_system_init();
    game_state_init();

    splash_sequence();
    intro_cut_scene();

    while (1) {
        title_screen();

        /* START → slot menu. Back out returns to the title screen. */
        SlotChoice choice = slot_menu();
        if (choice.action == SLOT_ACTION_CANCEL) {
            continue;
        }

        game_state_reset();
        g_active_slot = choice.slot;

        /* Resume a saved run, or start fresh in the chosen slot. */
        if (choice.action == SLOT_ACTION_CONTINUE) {
            SaveSlot slot;
            if (save_slot_read(g_active_slot, &slot) == SAVE_VALID) {
                save_to_state(&slot);
            }
        }

        GameplayResult result = gameplay_screen();

        /* Fade to black for clean scene transition */
        fade_to_black(12u);

        if (result == GAME_RESULT_GAME_OVER) {
            show_game_over_screen(GAME_OVER_DEFAULT, game_state.score);
        } else {
            /* Autosave progress on completion before showing the win screen. */
            save_autosave(g_active_slot);
            show_win_screen(game_state.score, gameplay_get_win_reason());
        }

        credits_scene();
    }
}
