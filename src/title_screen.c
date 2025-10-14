#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "game_settings.h"
#include "input.h"
#include "lang.h"
#include "options_screen.h"
#include "title_screen.h"

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
    cls();

    set_bkg_tiles(5u, 6u, (uint8_t)strlen(g_settings.game_name), 1u, (uint8_t *)g_settings.game_name);
    set_bkg_tiles(7u, 7u, (uint8_t)strlen(g_settings.version), 1u, (uint8_t *)g_settings.version);

    while (1) {
        if (++frame_counter >= 30u) {
            frame_counter = 0u;
            visible = (uint8_t)!visible;
            draw_blink_line(4u, 11u, STR_TITLE_HINT_START_PLAY, visible);
            draw_blink_line(4u, 13u, STR_TITLE_HINT_SELECT_OPT, visible);
        }
        uint8_t pressed = get_pressed();
        if (pressed & J_START) {
            break;
        }
        if (pressed & J_SELECT) {
            options_screen();
        }
        wait_vbl_done();
    }
    flush_input();
    cls();
}
