#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

#include <gb/gb.h>
#include <stdint.h>

// Game object types
typedef enum {
    OBJECT_NONE = 0,
    OBJECT_TREASURE,
    OBJECT_HAZARD
} GameObjectType;

// Game object structure
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    uint8_t sprite_tile;
    uint8_t palette;
    GameObjectType type;
    uint8_t active;
} GameObject;

// Treasure chest sprite (8x8)
#define TREASURE_TILE 64
extern const uint8_t treasure_tiles[];

// Hazard sprite (8x8)
#define HAZARD_TILE 68
extern const uint8_t hazard_tiles[];

// Initialize game objects
void init_game_objects(void);

// Update game objects
void update_game_objects(void);

// Draw game objects
void draw_game_objects(uint16_t cam_x, uint16_t cam_y);

// Check collision with game objects
GameObject* check_object_collision(uint8_t x, uint8_t y, uint8_t width, uint8_t height);

// Handle object interaction
void handle_object_interaction(GameObject* obj);

// Handle losing a life
uint8_t game_lose_life(void);

#endif // GAME_OBJECTS_H
