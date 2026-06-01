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

/* Sprite VRAM layout:
 *   0.. 19 : player_idle       (20 tiles)
 *  20.. 99 : player_run        (80 tiles)
 * 100..103 : obj_treasure       (4 tiles)
 * 104..107 : obj_hazard         (4 tiles)
 * 108..127 : cat_idle          (20 tiles, placeholder)
 * 128..147 : cutscene_player   (20 tiles, placeholder)
 * 148..167 : cutscene_npc      (20 tiles, placeholder)
 */
#define TREASURE_TILE        100u
extern const uint8_t treasure_tiles[];

#define HAZARD_TILE          104u
extern const uint8_t hazard_tiles[];

#define CAT_IDLE_TILE        108u
#define CUTSCENE_PLAYER_TILE 128u
#define CUTSCENE_NPC_TILE    148u

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
