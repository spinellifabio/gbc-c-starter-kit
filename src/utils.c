#include "utils.h"

#include <gb/gb.h>
#include <stdint.h>

/**
 * @brief Initializes a clean scene by disabling LCD, clearing OAM, clearing VRAM,
 * resetting display layers, and restoring default palettes.
 * 
 * Call this at the start of any scene transition to prevent glitching.
 */
void scene_init_clean(void) {
    /* Disable LCD to prevent VRAM corruption during setup */
    DISPLAY_OFF;
    
    /* Clear OAM (sprite memory) by writing 0xFF to all 160 bytes */
    /* This completely removes all old sprites from the display */
    uint8_t *oam = (uint8_t *)0xFE00u;
    for (uint8_t i = 0u; i < 160u; i++) {
        oam[i] = 0xFFu;
    }
    
    /* Clear background VRAM */
    clear_screen();
    
    /* Show background and sprites */
    SHOW_BKG;
    SHOW_SPRITES;
    
    /* Restore default palettes (DMG/CGB compatible) */
    BGP_REG = 0xE4u;  /* Default DMG palette */
    OBP0_REG = 0xE4u;
    OBP1_REG = 0xE4u;
    
    /* Re-enable LCD */
    DISPLAY_ON;
    
    /* Wait for VBlank to ensure all changes are applied */
    wait_vbl_done();
}

/**
 * @brief Fades the display to black over the specified number of frames.
 * 
 * @param frames Number of frames to fade over (12 = ~200ms at 60 FPS)
 * 
 * This function gradually reduces palette brightness by looping through
 * palette entries and reducing their values. Works on both DMG and CGB.
 */
void fade_to_black(uint8_t frames) {
    uint8_t frame;
    uint8_t step;
    
    if (frames == 0u) return;
    
    /* For each frame, reduce palette brightness */
    for (frame = 0u; frame < frames; frame++) {
        /* Calculate fade step (0 to frames-1) */
        step = (frame * 4u) / frames;  /* 0-4 step range */
        
        /* Apply fade to background palette */
        uint8_t faded_bgp = BGP_REG;
        if (step > 0u) {
            /* Gradually darken: shift palette values down */
            faded_bgp = (faded_bgp >> (step * 2u)) & 0xC0u;
        }
        BGP_REG = faded_bgp;
        
        /* Apply fade to sprite palettes */
        uint8_t faded_obp = OBP0_REG;
        if (step > 0u) {
            faded_obp = (faded_obp >> (step * 2u)) & 0xC0u;
        }
        OBP0_REG = faded_obp;
        OBP1_REG = faded_obp;
        
        /* Wait for VBlank to apply palette change */
        wait_vbl_done();
    }
    
    /* Ensure fully black at the end */
    BGP_REG = 0u;
    OBP0_REG = 0u;
    OBP1_REG = 0u;
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
    uint8_t frame;
    uint8_t step;
    
    if (frames == 0u) return;
    
    /* For each frame, increase palette brightness */
    for (frame = 0u; frame < frames; frame++) {
        /* Calculate fade step (0 to frames-1) */
        step = (frame * 4u) / frames;  /* 0-4 step range */
        
        /* Restore background palette */
        uint8_t restored_bgp = 0xE4u;  /* Default palette */
        if (step < 4u) {
            /* Gradually brighten: shift palette values up */
            restored_bgp = (0xE4u >> (4u - step) * 2u) & 0xFFu;
        }
        BGP_REG = restored_bgp;
        
        /* Restore sprite palettes */
        uint8_t restored_obp = 0xE4u;
        if (step < 4u) {
            restored_obp = (0xE4u >> (4u - step) * 2u) & 0xFFu;
        }
        OBP0_REG = restored_obp;
        OBP1_REG = restored_obp;
        
        /* Wait for VBlank to apply palette change */
        wait_vbl_done();
    }
    
    /* Ensure fully restored at the end */
    BGP_REG = 0xE4u;
    OBP0_REG = 0xE4u;
    OBP1_REG = 0xE4u;
    wait_vbl_done();
}
