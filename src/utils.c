#include "utils.h"

#include <gb/gb.h>
#include <gb/metasprites.h>
#include <stdint.h>

/**
 * @brief Initializes a clean scene by disabling LCD, clearing OAM, clearing VRAM,
 * resetting display layers, and restoring default palettes.
 * 
 * Call this at the start of any scene transition to prevent glitching.
 */
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
