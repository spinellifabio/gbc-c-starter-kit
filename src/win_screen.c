#include <gb/gb.h>
#include <stdio.h>

#include "utils.h"
#include "win_screen.h"
#include "lang.h"

void show_win_screen(uint16_t score, WinReason reason) {
    (void)reason;  /* Currently only one win reason, but extensible for future */

    /* Fade to black for clean transition */
    fade_to_black(12u);

    /* Initialize clean scene (disable LCD, hide sprites, clear VRAM, reset palettes) */
    scene_init_clean();

    /* Display win title centered vertically at row 8 */
    print_centered(lang_str(STR_WIN_TITLE), 8u);

    /* Display treasure message centered at row 10 */
    print_centered(lang_str(STR_WIN_TREASURE), 10u);

    /* Display score if not zero */
    if (score > 0u) {
        char score_str[6];
        int_to_str(score, score_str, 5u);

        /* Display score label at row 12 */
        print_centered(lang_str(STR_SCORE_LABEL), 12u);

        /* Display score value next to label */
        gotoxy(13u, 12u);
        for (uint8_t i = 0u; i < 5u; i++) {
            printf("%c", score_str[i]);
        }
    }

    /* Fade in from black */
    fade_from_black(12u);

    /* Brief pause so accidental button mashing does not skip the screen */
    uint8_t wait_frames = 180u;  /* ~3 seconds at 60 FPS */
    while (wait_frames--) {
        wait_vbl_done();
    }

    /* Show continue prompt at row 16 (near bottom) */
    print_centered(lang_str(STR_PRESS_START), 16u);
    wait_any_key();
}
