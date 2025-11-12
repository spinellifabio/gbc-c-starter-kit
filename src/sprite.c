#include "../include/sprite.h"
#include <string.h>

// Array to track all sprites
static Sprite sprites[MAX_SPRITES];
static uint8_t next_sprite_id = 0;

// Initialize the sprite system
void sprite_init(void) {
    // Clear sprite array
    memset(sprites, 0, sizeof(sprites));
    
    // Initialize all sprites as inactive
    for (uint8_t i = 0; i < MAX_SPRITES; i++) {
        sprites[i].id = i;
        sprites[i].flags = 0;
        sprites[i].x = 0;
        sprites[i].y = 0;
        sprites[i].tile_id = 0;
        sprites[i].width = 1;
        sprites[i].height = 1;
        sprites[i].palette = 0;
        sprites[i].priority = 0;
        sprites[i].flip_x = 0;
        sprites[i].flip_y = 0;
    }
    
    // Clear OAM (hide all sprites by setting Y=0)
    for (uint8_t i = 0; i < 40; i++) {
        move_sprite(i, 0, 0);
    }
    
    next_sprite_id = 0;
}

// Find the next available sprite
Sprite* sprite_alloc(void) {
    // Simple round-robin allocation to distribute wear
    for (uint8_t i = 0; i < MAX_SPRITES; i++) {
        uint8_t idx = (next_sprite_id + i) % MAX_SPRITES;
        if (!(sprites[idx].flags & SPRITE_FLAG_ACTIVE)) {
            sprites[idx].flags = SPRITE_FLAG_ACTIVE | SPRITE_FLAG_VISIBLE;
            next_sprite_id = (idx + 1) % MAX_SPRITES;
            return &sprites[idx];
        }
    }
    return NULL; // No free sprites
}

// Free a sprite
void sprite_free(Sprite* sprite) {
    if (sprite && (sprite->flags & SPRITE_FLAG_ACTIVE)) {
        // Hide the sprite
        move_sprite(sprite->id, 0, 0);
        sprite->flags = 0;
    }
}

// Update sprite properties in OAM
void sprite_update(Sprite* sprite) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    
    uint8_t flags = 0;
    if (sprite->flip_x) flags |= S_FLIPX;
    if (sprite->flip_y) flags |= S_FLIPY;
    if (sprite->priority) flags |= S_PRIORITY;
    
    // Set sprite attributes (tile, palette, flags)
    set_sprite_prop(sprite->id, flags | sprite->palette);
    
    // Set sprite position
    if (sprite->flags & SPRITE_FLAG_VISIBLE) {
        move_sprite(sprite->id, sprite->x, sprite->y);
    } else {
        move_sprite(sprite->id, 0, 0);
    }
    
    // Handle multi-tile sprites (2x2 max)
    if (sprite->width > 1 || sprite->height > 1) {
        uint8_t tile = sprite->tile_id;
        uint8_t base_x = sprite->x;
        uint8_t base_y = sprite->y;
        
        for (uint8_t y = 0; y < sprite->height; y++) {
            for (uint8_t x = 0; x < sprite->width; x++) {
                if (x == 0 && y == 0) continue; // Skip the base sprite
                
                uint8_t sub_id = sprite->id + y * 2 + x;
                if (sub_id < 40) { // Don't exceed OAM limit
                    set_sprite_prop(sub_id, flags | sprite->palette);
                    move_sprite(sub_id, base_x + x * 8, base_y + y * 8);
                    set_sprite_tile(sub_id, tile + y * 2 + x);
                }
            }
        }
    }
}

// Update all active sprites in OAM
void sprite_update_all(void) {
    for (uint8_t i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i].flags & SPRITE_FLAG_ACTIVE) {
            sprite_update(&sprites[i]);
        }
    }
}

// Clear all sprites (hide them by setting Y=0)
void sprite_clear_all(void) {
    for (uint8_t i = 0; i < 40; i++) {
        move_sprite(i, 0, 0);
    }
}

// Set sprite position
void sprite_set_pos(Sprite* sprite, uint8_t x, uint8_t y) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    sprite->x = x;
    sprite->y = y;
    sprite_update(sprite);
}

// Set sprite visibility
void sprite_set_visible(Sprite* sprite, uint8_t visible) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    
    if (visible) {
        sprite->flags |= SPRITE_FLAG_VISIBLE;
    } else {
        sprite->flags &= ~SPRITE_FLAG_VISIBLE;
    }
    
    // Update immediately
    if (visible) {
        move_sprite(sprite->id, sprite->x, sprite->y);
    } else {
        move_sprite(sprite->id, 0, 0);
    }
}

// Set sprite tiles (for animation)
void sprite_set_tiles(Sprite* sprite, uint8_t tile_id) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    
    sprite->tile_id = tile_id;
    set_sprite_tile(sprite->id, tile_id);
    
    // Update multi-tile sprites
    if (sprite->width > 1 || sprite->height > 1) {
        for (uint8_t y = 0; y < sprite->height; y++) {
            for (uint8_t x = 0; x < sprite->width; x++) {
                if (x == 0 && y == 0) continue; // Skip the base sprite
                
                uint8_t sub_id = sprite->id + y * 2 + x;
                if (sub_id < 40) { // Don't exceed OAM limit
                    set_sprite_tile(sub_id, tile_id + y * 2 + x);
                }
            }
        }
    }
}

// Set sprite flip state
void sprite_set_flip(Sprite* sprite, uint8_t flip_x, uint8_t flip_y) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    
    sprite->flip_x = flip_x ? 1 : 0;
    sprite->flip_y = flip_y ? 1 : 0;
    sprite_update(sprite);
}

// Set sprite priority (0=above BG, 1=behind BG)
void sprite_set_priority(Sprite* sprite, uint8_t priority) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    
    sprite->priority = priority ? 1 : 0;
    sprite_update(sprite);
}

// Set sprite palette
void sprite_set_palette(Sprite* sprite, uint8_t palette) {
    if (!sprite || !(sprite->flags & SPRITE_FLAG_ACTIVE)) return;
    
    sprite->palette = palette & 0x07; // Limit to 3 bits (0-7)
    sprite_update(sprite);
}
