#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lang.h"
#include "splash.h"

static void show_splash(LangStringId id_text, uint16_t duration_frames) {
    const char *text = lang_str(id_text);
    gotoxy((uint8_t)((20u - strlen(text)) / 2u), 9u);
    printf("%s", text);
    for (uint16_t f = 0; f < duration_frames; f++) {
        wait_vbl_done();
    }
    cls();
}

void splash_sequence(void) {
    show_splash(STR_SPLASH_STUDIO, 120u);
    show_splash(STR_SPLASH_PRESENTS, 120u);
}
