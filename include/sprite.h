#ifndef SPRITE_H
#define SPRITE_H

#include <gb/gb.h>
#include <stdint.h>

// Maximum number of hardware sprites (40 total on GBC, but we'll use 32 to be safe)
#define MAX_SPRITES 32

// Sprite flags (for internal use)
#define SPRITE_FLAG_ACTIVE 0x01
#define SPRITE_FLAG_VISIBLE 0x02

// Sprite structure to track sprite state
typedef struct {
    uint8_t id;         // OAM index
    uint8_t flags;      // Status flags
    uint8_t tile_id;    // Starting tile ID
    uint8_t x;          // X position (0-159)
    uint8_t y;          // Y position (0-143)
    uint8_t width;      // Width in tiles (1-2)
    uint8_t height;     // Height in tiles (1-2)
    uint8_t palette;    // Palette number (0-7)
    uint8_t priority;   // Priority (0=above BG, 1=behind BG)
    uint8_t flip_x;     // Horizontal flip
    uint8_t flip_y;     // Vertical flip
} Sprite;

// Initialize the sprite system
void sprite_init(void);

// Allocate a new sprite (returns NULL if no sprites available)
Sprite* sprite_alloc(void);

// Free a sprite
void sprite_free(Sprite* sprite);

// Update sprite properties in OAM
void sprite_update(Sprite* sprite);

// Update all active sprites in OAM
void sprite_update_all(void);

// Clear all sprites (hide them by setting Y=0)
void sprite_clear_all(void);

// Set sprite position
void sprite_set_pos(Sprite* sprite, uint8_t x, uint8_t y);

// Set sprite visibility
void sprite_set_visible(Sprite* sprite, uint8_t visible);

// Set sprite tiles (for animation)
void sprite_set_tiles(Sprite* sprite, uint8_t tile_id);

// Set sprite flip state
void sprite_set_flip(Sprite* sprite, uint8_t flip_x, uint8_t flip_y);

// Set sprite priority (0=above BG, 1=behind BG)
void sprite_set_priority(Sprite* sprite, uint8_t priority);

// Set sprite palette
void sprite_set_palette(Sprite* sprite, uint8_t palette);

#endif // SPRITE_H
