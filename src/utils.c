#include "utils.h"

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>
#include <stdint.h>

static uint8_t font_first_tile;

void text_set_font_base(uint8_t first_tile) {
    font_first_tile = first_tile;
}

uint8_t text_font_base(void) {
    return font_first_tile;
}

uint8_t text_char_tile(char c) {
    if ((uint8_t)c < ' ' || (uint8_t)c > '~') {
        c = ' ';
    }
    return (uint8_t)(font_first_tile + (uint8_t)((uint8_t)c - ' '));
}

void wait_any_key(void) {
    waitpad(J_START | J_A | J_B | J_UP | J_DOWN | J_LEFT | J_RIGHT);
    waitpadup();
}

/* CGB is never defined by the build (only -O2 in GBDK_CFLAGS); this project's
 * convention is runtime detection via _cpu (see game_system.c), which is
 * what this uses instead of the old always-false #ifdef CGB. */
void clear_attr_map(void) {
    if (_cpu == CGB_TYPE) {
        uint8_t old_vbk = VBK_REG;
        VBK_REG = 1u;
        fill_bkg_rect(0u, 0u, 20u, 18u, 0u);
        VBK_REG = old_vbk;
    }
}

void clear_screen(void) {
    fill_bkg_rect(0u, 0u, 20u, 18u, text_char_tile(' '));
    clear_attr_map();
}

void int_to_str(uint16_t value, char *buffer, uint8_t min_digits) {
    uint8_t i = 0;
    uint8_t digits = 1;
    uint16_t temp = value / 10;

    while (temp > 0) {
        temp /= 10;
        digits++;
    }

    while (digits < min_digits) {
        buffer[i++] = '0';
        min_digits--;
    }

    uint16_t divisor = 1;
    for (uint8_t j = 1; j < digits; j++) {
        divisor *= 10;
    }

    while (divisor > 0) {
        buffer[i++] = '0' + (value / divisor) % 10;
        divisor /= 10;
    }
    buffer[i] = '\0';
}

void print_centered(const char *text, uint8_t row) {
    uint8_t len = 0;
    const char *p = text;
    while (*p++) len++;

    uint8_t x = (20 - len) / 2;
    gotoxy(x, row);
    while (*text) {
        printf("%c", *text++);
    }
}

/**
 * @brief Initializes a clean scene by disabling LCD, clearing OAM, clearing VRAM,
 * resetting display layers, and restoring default palettes.
 * 
 * Call this at the start of any scene transition to prevent glitching.
 */
void set_solid_bkg(palette_color_t color) {
    if (_cpu == CGB_TYPE) {
        palette_color_t pal[4] = {
            color,               /* color 0: background fill */
            RGB8(140,140,140),   /* color 1: medium gray */
            RGB8(200,200,200),   /* color 2: light gray */
            RGB8(255,255,255),   /* color 3: white — GBDK font ink pixels use index 3 */
        };
        set_bkg_palette(0u, 1u, pal);
    }
}

void scene_init_clean(void) {
    DISPLAY_OFF;

    /* Reset background scroll — gameplay sets camera offsets that persist otherwise */
    SCX_REG = 0u;
    SCY_REG = 0u;

    /* Move window off-screen and hide it — dialogue may have left it visible */
    WX_REG = 7u;
    WY_REG = 144u;
    HIDE_WIN;

    /* Clear background tiles and attribute map */
    clear_screen();

    SHOW_BKG;

    /* Restore default DMG palettes */
    BGP_REG  = 0xE4u;
    OBP0_REG = 0xE4u;
    OBP1_REG = 0xE4u;

    DISPLAY_ON;

    /* Clear the GBDK shadow OAM buffer. Direct writes to hardware OAM (0xFE00) are
     * overwritten by the VBlank DMA transfer, so hide_sprites_range() — which updates
     * the shadow buffer — is the correct way to prevent old sprites from persisting. */
    hide_sprites_range(0u, 40u);
    SHOW_SPRITES;

    wait_vbl_done();  /* Trigger DMA so cleared shadow OAM reaches hardware */
}

/**
 * @brief Fades the display to black over the specified number of frames.
 * 
 * @param frames Number of frames to fade over (12 = ~200ms at 60 FPS)
 * 
 * This function gradually reduces palette brightness by looping through
 * palette entries and reducing their values. Works on both DMG and CGB.
 */
/* DMG palette fade steps: each entry encodes 4 color→shade mappings (2 bits each).
 * 0xE4 = [3,2,1,0] normal  →  0xFF = [3,3,3,3] fully black. */
static const uint8_t dmg_fade_steps[4] = { 0xE4u, 0xE9u, 0xFAu, 0xFFu };

void fade_to_black(uint8_t frames) {
    if (frames == 0u) return;

    for (uint8_t frame = 0u; frame < frames; frame++) {
        uint8_t pal = dmg_fade_steps[(frame * 4u) / frames];
        BGP_REG = pal;
        OBP0_REG = pal;
        OBP1_REG = pal;
        wait_vbl_done();
    }

    /* Ensure fully black at the end */
    BGP_REG = 0xFFu;
    OBP0_REG = 0xFFu;
    OBP1_REG = 0xFFu;
    wait_vbl_done();
}

/**
 * @brief Fades the display from black back to normal over the specified number of frames.
 * 
 * @param frames Number of frames to fade over (12 = ~200ms at 60 FPS)
 * 
 * This function gradually increases palette brightness by restoring palette values.
 * Works on both DMG and CGB.
 */
void fade_from_black(uint8_t frames) {
    if (frames == 0u) return;

    for (uint8_t frame = 0u; frame < frames; frame++) {
        uint8_t pal = dmg_fade_steps[3u - (frame * 4u) / frames];
        BGP_REG = pal;
        OBP0_REG = pal;
        OBP1_REG = pal;
        wait_vbl_done();
    }

    /* Ensure fully restored at the end */
    BGP_REG = 0xE4u;
    OBP0_REG = 0xE4u;
    OBP1_REG = 0xE4u;
    wait_vbl_done();
}
