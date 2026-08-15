#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "bgm.h"
#include "game_settings.h"
#include "input.h"
#include "lang.h"
#include "options_screen.h"
#include "title_screen.h"
#include "utils.h"
#include "logo.h"

static void draw_blink_line(uint8_t x, uint8_t y, LangStringId id, uint8_t visible) {
    gotoxy(x, y);
    printf("                ");
    if (visible) {
        gotoxy(x, y);
        printf("%s", lang_str(id));
    }
}

void title_screen(void) {
    uint8_t frame_counter = 0u;
    uint8_t visible = 1u;
    WY_REG = 144u;
    HIDE_WIN;
    set_solid_bkg(BKG_COLOR_TITLE);
    bgm_play_title_theme();
    cls();
    clear_attr_map();

    /* Logo banner: explicit tile-index map (never a raw char*) — see
     * contracts/text_rendering.md. The previous code wrote g_settings.game_name
     * straight into set_bkg_tiles(), treating ASCII bytes as BG tile indices;
     * since the font loads at first_tile 0, not 32, that produced a garbled
     * off-by-'@' render (research.md R0). */
    uint8_t banner_row[LOGO_BANNER_WIDTH];
    for (uint8_t i = 0u; i < LOGO_BANNER_WIDTH; i++) {
        banner_row[i] = (uint8_t)(LOGO_TILE_BASE + logo_banner_map[i]);
    }
    set_bkg_tiles((uint8_t)((20u - LOGO_BANNER_WIDTH) / 2u), 4u, LOGO_BANNER_WIDTH, 1u, banner_row);

    print_centered(g_settings.game_name, 6u);
    print_centered(g_settings.version, 7u);

    while (1) {
        if (++frame_counter >= 30u) {
            frame_counter = 0u;
            visible = (uint8_t)!visible;
            draw_blink_line(4u, 11u, STR_TITLE_HINT_START_PLAY, visible);
            draw_blink_line(4u, 13u, STR_TITLE_HINT_SELECT_OPT, visible);
        }
        uint8_t pressed = get_pressed();
        if (pressed & J_START) {
            sfx_play(SFX_UI_SELECT);
            break;
        }
        if (pressed & J_SELECT) {
            sfx_play(SFX_UI_SELECT);
            options_screen();
        }
        wait_vbl_done();
    }
    bgm_stop();
    flush_input();
    cls();
    clear_attr_map();
}
