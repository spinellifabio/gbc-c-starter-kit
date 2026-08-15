#ifndef RES_TILES_LOGO_H
#define RES_TILES_LOGO_H

#include <stdint.h>
#include <gbdk/platform.h>

/* Title screen logo/banner — a decorative double-rule with tapered end caps,
 * framing the game name. BG tile range 96-103 (8 of the 32 free tiles
 * between the font, 0-95, and the world tileset, 128-131). Placed via an
 * explicit tile-index map (never a raw char*) per contracts/text_rendering.md. */
#define LOGO_TILE_BASE       96u
#define LOGO_TILE_COUNT      2u
#define LOGO_BANNER_WIDTH    8u

extern const uint8_t logo_tiles[32];
/* Relative tile indices (add LOGO_TILE_BASE) for one banner row: tapered end
 * caps flanking six repeats of the double-rule tile. */
extern const uint8_t logo_banner_map[LOGO_BANNER_WIDTH];

BANKREF_EXTERN(logo)

#endif /* RES_TILES_LOGO_H */
