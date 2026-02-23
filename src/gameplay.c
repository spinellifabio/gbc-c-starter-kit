#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>

#include <stdint.h>
#include <string.h>

#include "Alex_idle_16x16.h"
#include "Alex_run_16x16.h"
#include "gameplay.h"
#include "tileset.h"
#include "world_defs.h"
#include "game_objects.h"
#include "game_state.h"
#include "dialogue.h"
#include "game_over.h"

#define SCREEN_CENTER_X 80u
#define SCREEN_CENTER_Y 72u

#define PLAYER_OFFSET_X (-8)
#define PLAYER_OFFSET_Y 16

#define RUN_FRAMES_PER_DIR 6u

#define DIR_RIGHT 0u
#define DIR_BACK  1u
#define DIR_LEFT  2u
#define DIR_FRONT 3u

#define MOVE_SPEED 2

#define HOME_MIN_X 64u   /* Spawn zone bounding box; landing back here ends game */
#define HOME_MAX_X 96u
#define HOME_MIN_Y 64u
#define HOME_MAX_Y 96u

#define M_WATER 0u
#define M_SAND  1u
#define M_GRASS 2u

#define LOGIC_TILE_COUNT 3u
#define BKG_HW_W 32u
#define BKG_HW_H 32u

#define VIEW_LOGIC_W (BKG_HW_W >> 1)  /* 16 logic columns visible at once */
#define VIEW_LOGIC_H (BKG_HW_H >> 1)  /* 16 logic rows visible at once */

#define PLAYER_HALF_WIDTH 6
#define PLAYER_FEET_HEIGHT 2

static uint16_t cam_x = 0u;
static uint16_t cam_y = 0u;

static const uint8_t island_map[MAP_HEIGHT][MAP_WIDTH] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

static uint8_t tile_quads_ready = 0u;
static uint8_t tile_quads[LOGIC_TILE_COUNT][4];
static uint8_t attr_quads[LOGIC_TILE_COUNT][4];
static uint8_t last_view_tile_x = 0xFFu;
static uint8_t last_view_tile_y = 0xFFu;

static void ensure_tile_quads(void) {
    if (tile_quads_ready) {
        return;
    }
    for (uint8_t logic = 0u; logic < LOGIC_TILE_COUNT; logic++) {
        uint8_t base_x = (uint8_t)(logic * 2u);
        uint8_t top_row = base_x;
        uint8_t bottom_row = (uint8_t)(base_x + tileset_MAP_ATTRIBUTES_WIDTH);

        tile_quads[logic][0] = (uint8_t)(FONT_OFFSET + tileset_map[top_row]);
        tile_quads[logic][1] = (uint8_t)(FONT_OFFSET + tileset_map[top_row + 1u]);
        tile_quads[logic][2] = (uint8_t)(FONT_OFFSET + tileset_map[bottom_row]);
        tile_quads[logic][3] = (uint8_t)(FONT_OFFSET + tileset_map[bottom_row + 1u]);

        attr_quads[logic][0] = tileset_map_attributes[top_row];
        attr_quads[logic][1] = tileset_map_attributes[top_row + 1u];
        attr_quads[logic][2] = tileset_map_attributes[bottom_row];
        attr_quads[logic][3] = tileset_map_attributes[bottom_row + 1u];
    }
    tile_quads_ready = 1u;
}

static void render_viewport(void);

static void reset_viewport_cache(void);

static void update_camera(uint16_t world_x, uint16_t world_y) {
    int16_t new_x = (int16_t)world_x - (int16_t)SCREEN_CENTER_X;
    int16_t new_y = (int16_t)world_y - (int16_t)SCREEN_CENTER_Y;
    if (new_x < 0) {
        new_x = 0;
    }
    if (new_y < 0) {
        new_y = 0;
    }
    uint16_t max_x = MAP_WIDTH * TILE_SIZE - SCREENWIDTH;
    uint16_t max_y = MAP_HEIGHT * TILE_SIZE - SCREENHEIGHT;
    if (new_x > (int16_t)max_x) {
        new_x = (int16_t)max_x;
    }
    if (new_y > (int16_t)max_y) {
        new_y = (int16_t)max_y;
    }
    cam_x = (uint16_t)new_x;
    cam_y = (uint16_t)new_y;
    SCX_REG = (uint8_t)cam_x;
    SCY_REG = (uint8_t)cam_y;
    render_viewport();
}

static uint8_t point_walkable(int16_t sample_x, int16_t sample_y) {
    if ((sample_x < 0) || (sample_y < 0)) {
        return 0u;
    }
    uint16_t px = (uint16_t)sample_x >> TILE_SHIFT;
    uint16_t py = (uint16_t)sample_y >> TILE_SHIFT;
    if ((px >= MAP_WIDTH) || (py >= MAP_HEIGHT)) {
        return 0u;
    }
    return (island_map[py][px] != M_WATER);
}

static uint8_t can_walk(uint16_t x, uint16_t foot_y) {
    int16_t left = (int16_t)x - PLAYER_HALF_WIDTH;
    int16_t right = (int16_t)x + PLAYER_HALF_WIDTH;
    int16_t ground = (int16_t)foot_y - PLAYER_FEET_HEIGHT;

    if ((left < 0) || (right >= (int16_t)(MAP_WIDTH * TILE_SIZE))) {
        return 0u;
    }
    if ((ground < 0) || (ground >= (int16_t)(MAP_HEIGHT * TILE_SIZE))) {
        return 0u;
    }

    if (!point_walkable(left, ground)) {
        return 0u;
    }
    if (!point_walkable((int16_t)x, ground)) {
        return 0u;
    }
    if (!point_walkable(right, ground)) {
        return 0u;
    }
    return 1u;
}

static void reset_viewport_cache(void) {
    last_view_tile_x = 0xFFu;
    last_view_tile_y = 0xFFu;
}

static void blit_logic_quad(uint8_t logic_x, uint8_t logic_y, uint8_t dest_col, uint8_t dest_row) {
    uint8_t logic = M_WATER;
    if ((logic_x < MAP_WIDTH) && (logic_y < MAP_HEIGHT)) {
        logic = island_map[logic_y][logic_x];
    }
    if (logic >= LOGIC_TILE_COUNT) {
        logic = M_WATER;
    }

    uint8_t tiles[4];
    uint8_t attrs[4];

    tiles[0] = tile_quads[logic][0];
    tiles[1] = tile_quads[logic][1];
    tiles[2] = tile_quads[logic][2];
    tiles[3] = tile_quads[logic][3];

    attrs[0] = attr_quads[logic][0];
    attrs[1] = attr_quads[logic][1];
    attrs[2] = attr_quads[logic][2];
    attrs[3] = attr_quads[logic][3];

    set_bkg_tiles(dest_col, dest_row, 2u, 2u, tiles);
    if (_cpu == CGB_TYPE) {
        VBK_REG = 1u;
        set_bkg_tiles(dest_col, dest_row, 2u, 2u, attrs);
        VBK_REG = 0u;
    }
}

static void draw_full_view(uint8_t tile_x, uint8_t tile_y) {
    for (uint8_t row = 0u; row < VIEW_LOGIC_H; row++) {
        uint8_t logic_y = (uint8_t)(tile_y + row);
        uint8_t dest_row = (uint8_t)(((tile_y + row) & (VIEW_LOGIC_H - 1u)) << 1);
        for (uint8_t col = 0u; col < VIEW_LOGIC_W; col++) {
            uint8_t logic_x = (uint8_t)(tile_x + col);
            uint8_t dest_col = (uint8_t)(((tile_x + col) & (VIEW_LOGIC_W - 1u)) << 1);
            blit_logic_quad(logic_x, logic_y, dest_col, dest_row);
        }
    }
}

static void draw_logic_column(uint8_t current_tile_x, uint8_t current_tile_y, uint8_t logic_x, uint8_t dest_col) {
    (void)current_tile_x;
    for (uint8_t row = 0u; row < VIEW_LOGIC_H; row++) {
        uint8_t logic_y = (uint8_t)(current_tile_y + row);
        uint8_t dest_row = (uint8_t)(((current_tile_y + row) & (VIEW_LOGIC_H - 1u)) << 1);
        blit_logic_quad(logic_x, logic_y, dest_col, dest_row);
    }
}

static void draw_logic_row(uint8_t current_tile_x, uint8_t current_tile_y, uint8_t logic_y, uint8_t dest_row) {
    (void)current_tile_y;
    for (uint8_t col = 0u; col < VIEW_LOGIC_W; col++) {
        uint8_t logic_x = (uint8_t)(current_tile_x + col);
        uint8_t dest_col = (uint8_t)(((current_tile_x + col) & (VIEW_LOGIC_W - 1u)) << 1);
        blit_logic_quad(logic_x, logic_y, dest_col, dest_row);
    }
}

static void render_viewport(void) {
    uint8_t tile_x = (uint8_t)(cam_x >> TILE_SHIFT);
    uint8_t tile_y = (uint8_t)(cam_y >> TILE_SHIFT);

    ensure_tile_quads();

    if (last_view_tile_x == 0xFFu) {
        draw_full_view(tile_x, tile_y);
        last_view_tile_x = tile_x;
        last_view_tile_y = tile_y;
        return;
    }

    int8_t delta_x = (int8_t)tile_x - (int8_t)last_view_tile_x;
    int8_t delta_y = (int8_t)tile_y - (int8_t)last_view_tile_y;

    while (delta_x > 0) {
        uint8_t logic_x = (uint8_t)(last_view_tile_x + VIEW_LOGIC_W);
        uint8_t dest_col = (uint8_t)(((last_view_tile_x + VIEW_LOGIC_W) & (VIEW_LOGIC_W - 1u)) << 1);
        draw_logic_column(tile_x, tile_y, logic_x, dest_col);
        last_view_tile_x++;
        delta_x--;
    }
    while (delta_x < 0) {
        last_view_tile_x--;
        delta_x++;
        uint8_t logic_x = last_view_tile_x;
        uint8_t dest_col = (uint8_t)((last_view_tile_x & (VIEW_LOGIC_W - 1u)) << 1);
        draw_logic_column(tile_x, tile_y, logic_x, dest_col);
    }

    while (delta_y > 0) {
        uint8_t logic_y = (uint8_t)(last_view_tile_y + VIEW_LOGIC_H);
        uint8_t dest_row = (uint8_t)(((last_view_tile_y + VIEW_LOGIC_H) & (VIEW_LOGIC_H - 1u)) << 1);
        draw_logic_row(tile_x, tile_y, logic_y, dest_row);
        last_view_tile_y++;
        delta_y--;
    }
    while (delta_y < 0) {
        last_view_tile_y--;
        delta_y++;
        uint8_t logic_y = last_view_tile_y;
        uint8_t dest_row = (uint8_t)((last_view_tile_y & (VIEW_LOGIC_H - 1u)) << 1);
        draw_logic_row(tile_x, tile_y, logic_y, dest_row);
    }
}

// Player state
typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t dir;
    uint8_t frame;
    uint8_t moving;
} Player;

static Player player;
static uint8_t game_active = 1;
static GameplayResult game_result = GAME_RESULT_WIN;
static WinReason game_result_reason = WIN_TREASURE_SECURED;
static GameObject* last_obj_interacted = 0;
static const uint8_t run_frame_lut[RUN_FRAMES_PER_DIR * 2] = {0,0,1,1,2,2,3,3,4,4,5,5};

void gameplay_signal_game_over(void) {
    game_result = GAME_RESULT_GAME_OVER;
    game_active = 0;
}

void reset_gameplay(void) {
    // Reset player position
    player.x = 80;
    player.y = 80;
    player.dir = DIR_FRONT;
    player.frame = 0;
    player.moving = 0;

    // Reset camera
    cam_x = 0;
    cam_y = 0;
    reset_viewport_cache();

    // Reset game objects
    init_game_objects();

    // Reset game state flags
    game_state.has_treasure = 0;
    game_state.encountered_hazard = 0;
    last_obj_interacted = 0;

    // Reset game active flag
    game_active = 1;
    game_result = GAME_RESULT_WIN;
}

void handle_player_movement(void) {
    uint8_t joy = joypad();
    uint8_t moved = 0;

    // Handle movement
    if (joy & J_LEFT) {
        player.dir = DIR_LEFT;
        if (can_walk((uint16_t)(player.x - MOVE_SPEED), player.y + PLAYER_OFFSET_Y)) {
            player.x -= MOVE_SPEED;
            moved = 1;
        }
    } else if (joy & J_RIGHT) {
        player.dir = DIR_RIGHT;
        if (can_walk((uint16_t)(player.x + MOVE_SPEED), player.y + PLAYER_OFFSET_Y)) {
            player.x += MOVE_SPEED;
            moved = 1;
        }
    } else if (joy & J_UP) {
        player.dir = DIR_BACK;
        if (can_walk(player.x, (uint16_t)(player.y - MOVE_SPEED + PLAYER_OFFSET_Y))) {
            player.y -= MOVE_SPEED;
            moved = 1;
        }
    } else if (joy & J_DOWN) {
        player.dir = DIR_FRONT;
        if (can_walk(player.x, (uint16_t)(player.y + MOVE_SPEED + PLAYER_OFFSET_Y))) {
            player.y += MOVE_SPEED;
            moved = 1;
        }
    }

    // Update animation
    if (moved) {
        player.frame = (player.frame + 1) % (RUN_FRAMES_PER_DIR * 2);
    } else {
        player.frame = 0;
    }
    player.moving = moved;

    // Check for object collisions
    GameObject* obj = check_object_collision(player.x - 4, player.y - 8, 8, 16);
    if (obj) {
        if (obj != last_obj_interacted) {
            handle_object_interaction(obj);
            last_obj_interacted = obj;
        }
    } else {
        last_obj_interacted = 0;
    }

    // Check win condition (return to home zone with treasure)
    if (game_state.has_treasure &&
        (player.x >= HOME_MIN_X) && (player.x <= HOME_MAX_X) &&
        (player.y >= HOME_MIN_Y) && (player.y <= HOME_MAX_Y)) {
        game_result = GAME_RESULT_WIN;
        game_active = 0;
    }
}

void draw_player(void) {
    uint8_t tile;

    if (player.moving) {
        // Use running animation frames; frame_lut avoids costly div/mod on Z80
        uint8_t frame_idx = run_frame_lut[player.frame]; /* 0..5 */
        tile = (uint8_t)((player.dir * RUN_FRAMES_PER_DIR + frame_idx) << 2);
    } else {
        // Standing still
        tile = (uint8_t)((player.dir * RUN_FRAMES_PER_DIR) << 2);
    }

    /* OAM layout: indices 0-19 reserved for game objects (10 objects × 2 sprites),
     * player occupies indices 20-23 to avoid collision. */
    set_sprite_tile(20, tile);
    set_sprite_tile(21, tile + 1);
    set_sprite_tile(22, tile + 2);
    set_sprite_tile(23, tile + 3);

    // Position sprites (adjust for camera)
    int16_t screen_x = (int16_t)player.x - (int16_t)cam_x;
    int16_t screen_y = (int16_t)player.y - (int16_t)cam_y - 8;

    move_sprite(20, screen_x, screen_y);
    move_sprite(21, screen_x + 8, screen_y);
    move_sprite(22, screen_x, screen_y + 8);
    move_sprite(23, screen_x + 8, screen_y + 8);
}



GameplayResult gameplay_screen(void) {
    // Initialize game state
    reset_gameplay();

    // Initialize graphics
    ensure_tile_quads();
    reset_viewport_cache();

    // Show instructions
    dialogue_show_text("Find the treasure!\nBut beware of hazards!");

    // Main game loop
    while (game_active) {
        // Handle input and update game state
        handle_player_movement();

        // Update camera to follow player (also renders the viewport internally)
        update_camera(player.x, player.y);

        // Update game objects
        update_game_objects();

        // Draw game objects
        draw_game_objects(cam_x, cam_y);

        // Draw player
        draw_player();

        // Wait for VBLANK to ensure smooth animation
        wait_vbl_done();
    }

    return game_result;
}

/**
 * @brief Retrieves the reason for the win condition
 * @return WinReason enum value indicating why the player won
 */
WinReason gameplay_get_win_reason(void) {
    return game_result_reason;
}
