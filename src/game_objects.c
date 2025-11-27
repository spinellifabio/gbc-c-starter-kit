#include "game_objects.h"
#include "game_state.h"
#include "dialogue.h"
#include "gameplay.h"
#include <string.h>
#include <stdio.h>

// Treasure chest sprite (8x8)
const uint8_t treasure_tiles[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // Empty
    0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0xFF,  // Top of chest
    0x7E,0x42,0x42,0x42,0x42,0x42,0x42,0x7E,  // Middle of chest
    0x7E,0x42,0x5A,0x5A,0x5A,0x5A,0x42,0x7E   // Bottom of chest with lock
};

// Hazard sprite (8x8)
const uint8_t hazard_tiles[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // Empty
    0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00,  // Exclamation mark top
    0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00,  // Exclamation mark middle
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00   // Empty
};

#define MAX_OBJECTS 10

static GameObject objects[MAX_OBJECTS];
static uint8_t objects_initialized = 0;

// Initialize game objects
void init_game_objects(void) {
    if (objects_initialized) return;

    // Clear all objects
    memset(objects, 0, sizeof(objects));

    // Add treasure chest
    objects[0].x = 100;
    objects[0].y = 50;
    objects[0].width = 16;
    objects[0].height = 16;
    objects[0].sprite_tile = TREASURE_TILE;
    objects[0].palette = 1;  // Red palette
    objects[0].type = OBJECT_TREASURE;
    objects[0].active = 1;

    // Add hazard
    objects[1].x = 50;
    objects[1].y = 80;
    objects[1].width = 8;
    objects[1].height = 16;
    objects[1].sprite_tile = HAZARD_TILE;
    objects[1].palette = 2;  // Yellow palette
    objects[1].type = OBJECT_HAZARD;
    objects[1].active = 1;

    // Load sprite data into VRAM
    set_sprite_data(TREASURE_TILE, 4, treasure_tiles);
    set_sprite_data(HAZARD_TILE, 4, hazard_tiles);

    objects_initialized = 1;
}

// Update game objects
void update_game_objects(void) {
    // For now, no updates needed
}

// Draw game objects
void draw_game_objects(uint16_t cam_x, uint16_t cam_y) {
    for (uint8_t i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].active) {
            // Compute screen position relative to camera; hide if off-screen
            int16_t screen_x = (int16_t)objects[i].x - (int16_t)cam_x;
            int16_t screen_y = (int16_t)objects[i].y - (int16_t)cam_y;

            if ((screen_x < -8) || (screen_x > (SCREENWIDTH + 8)) ||
                (screen_y < -16) || (screen_y > (SCREENHEIGHT + 16))) {
                move_sprite(i * 2, 0u, 0u);
                move_sprite(i * 2 + 1, 0u, 0u);
                continue;
            }

            // Draw object (simple 8x8 sprite for now)
            set_sprite_tile(i * 2, objects[i].sprite_tile);
            set_sprite_tile(i * 2 + 1, objects[i].sprite_tile + 1);

            // Set positions (adjust for 16x16 sprites)
            uint8_t draw_x = (uint8_t)screen_x;
            uint8_t draw_y = (uint8_t)screen_y;
            move_sprite(i * 2, draw_x, draw_y);
            move_sprite(i * 2 + 1, (uint8_t)(draw_x + 8u), draw_y);

            // Set palette
            if (objects[i].palette) {
                set_sprite_prop(i * 2, objects[i].palette - 1);
                set_sprite_prop(i * 2 + 1, objects[i].palette - 1);
            }
        }
    }
}

// Check collision with game objects
GameObject* check_object_collision(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    for (uint8_t i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].active) {
            if (x + width > objects[i].x &&
                x < objects[i].x + objects[i].width &&
                y + height > objects[i].y &&
                y < objects[i].y + objects[i].height) {
                return &objects[i];
            }
        }
    }
    return NULL;
}

// Handle losing a life
uint8_t game_lose_life(void) {
    if (game_state.lives > 0) {
        game_state.lives--;
        if (game_state.lives == 0) {
            gameplay_signal_game_over();
        } else {
            // Show remaining lives (rare, but kept if lives increase later)
            char lives_text[32];
            sprintf(lives_text, "Ouch! %u lives left.", (uint8_t)game_state.lives);
            dialogue_show_text(lives_text);
        }
    }
    return game_state.lives;
}

// Handle object interaction
void handle_object_interaction(GameObject* obj) {
    if (!obj || !obj->active) return;

    switch (obj->type) {
        case OBJECT_TREASURE:
            if (!game_state.has_treasure) {
                uint8_t choice = dialogue_show_yes_no("You found a treasure!\nTake it?");
                if (choice == DIALOGUE_RESULT_YES) {
                    game_state.has_treasure = 1;
                    obj->active = 0;  // Remove treasure
                    dialogue_show_text("You got the treasure!\nReturn to start to win!");
                }
            }
            break;

        case OBJECT_HAZARD:
            if (!game_state.encountered_hazard) {
                uint8_t choice = dialogue_show_yes_no("Danger! Continue?");
                if (choice == DIALOGUE_RESULT_YES) {
                    game_state.encountered_hazard = 1;
                    if (game_lose_life() == 0) {
                        // Game over: gameplay loop will exit and screen will show
                    } else {
                        dialogue_show_text("Ouch! Lost a life!");
                    }
                }
            }
            break;

        default:
            break;
    }
}
