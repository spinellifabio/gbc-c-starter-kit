/* Title screen logo/banner — hand-authored 2bpp BG tiles (see logo.h).
 * Format matches the rest of the codebase's hand-authored tile data
 * (src/game_objects.c treasure/hazard tiles): each row's low and high
 * bitplane bytes are equal, producing a solid-color-3 line (the GBDK font's
 * ink color per utils.c set_solid_bkg). */

#include "logo.h"

BANKREF(logo)

const uint8_t logo_tiles[32] = {
    /* tile 0 (relative index 0): double horizontal rule */
    0x00,0x00, 0x00,0x00, 0xFF,0xFF, 0x00,0x00,
    0x00,0x00, 0xFF,0xFF, 0x00,0x00, 0x00,0x00,
    /* tile 1 (relative index 1): tapered end cap */
    0x00,0x00, 0x00,0x00, 0x18,0x18, 0x3C,0x3C,
    0x3C,0x3C, 0x18,0x18, 0x00,0x00, 0x00,0x00
};

const uint8_t logo_banner_map[LOGO_BANNER_WIDTH] = {
    1u, 0u, 0u, 0u, 0u, 0u, 0u, 1u
};
