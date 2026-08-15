#include <gb/gb.h>
#include <gb/metasprites.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lang.h"
#include "splash.h"
#include "utils.h"

static void show_splash(LangStringId id_text, uint16_t duration_frames) {
    WY_REG = 144u;
    HIDE_WIN;
    set_solid_bkg(BKG_COLOR_SPLASH);
    cls();
    clear_attr_map();

    /* Hide any sprite left over from a prior scene (e.g. a reset from
     * gameplay); hide_sprites_range() updates the GBDK shadow OAM, and
     * wait_vbl_done() lets the VBlank DMA transfer flush it to hardware. */
    hide_sprites_range(0u, 40u);
    wait_vbl_done();
    const char *text = lang_str(id_text);
    gotoxy((uint8_t)((20u - strlen(text)) / 2u), 9u);
    printf("%s", text);
    for (uint16_t f = 0; f < duration_frames; f++) {
        wait_vbl_done();
    }
    cls();
    clear_attr_map();
}

void splash_sequence(void) {
    show_splash(STR_SPLASH_STUDIO, 120u);
    show_splash(STR_SPLASH_PRESENTS, 120u);
}
