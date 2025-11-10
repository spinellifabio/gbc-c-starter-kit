#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>

#include <stdint.h>

#include "Alex_idle_16x16.h"
#include "Alex_run_16x16.h"
#include "gameplay.h"
#include "tileset.h"
#include "world_defs.h"

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

void gameplay_screen(void) {
    uint16_t world_x = SCREEN_CENTER_X;
    uint16_t world_y = SCREEN_CENTER_Y;
    uint8_t dir = DIR_FRONT;
    uint8_t run_frame = 0u;
    uint8_t anim_tick = 0u;

    ensure_tile_quads();
    reset_viewport_cache();
    update_camera(world_x, world_y);

    while (1) {
        uint8_t keys = joypad();
        uint8_t moving = 0u;
        int16_t nx = (int16_t)world_x;
        int16_t ny = (int16_t)world_y;

        if (keys == J_DOWN) {
            dir = DIR_FRONT;
            ny += MOVE_SPEED;
            moving = 1u;
        } else if (keys == J_UP) {
            dir = DIR_BACK;
            ny -= MOVE_SPEED;
            moving = 1u;
        } else if (keys == J_LEFT) {
            dir = DIR_LEFT;
            nx -= MOVE_SPEED;
            moving = 1u;
        } else if (keys == J_RIGHT) {
            dir = DIR_RIGHT;
            nx += MOVE_SPEED;
            moving = 1u;
        }

        if (moving && can_walk((uint16_t)nx, (uint16_t)(ny + PLAYER_OFFSET_Y))) {
            world_x = (uint16_t)nx;
            world_y = (uint16_t)ny;
        } else {
            moving = 0u;
        }

        if (moving) {
            uint8_t delay = (MOVE_SPEED == 1) ? 8u : (MOVE_SPEED == 2) ? 6u : 4u;
            if (++anim_tick >= delay) {
                anim_tick = 0u;
                run_frame = (uint8_t)((run_frame + 1u) % RUN_FRAMES_PER_DIR);
            }
            uint8_t idx = (uint8_t)(dir * RUN_FRAMES_PER_DIR + run_frame);
            move_metasprite_ex(Alex_run_16x16_metasprites[idx],
                               Alex_idle_16x16_TILE_COUNT, 0u, 0u,
                               SCREEN_CENTER_X + PLAYER_OFFSET_X,
                               SCREEN_CENTER_Y + PLAYER_OFFSET_Y);
        } else {
            move_metasprite_ex(Alex_idle_16x16_metasprites[dir],
                               0u, 0u, 0u,
                               SCREEN_CENTER_X + PLAYER_OFFSET_X,
                               SCREEN_CENTER_Y + PLAYER_OFFSET_Y);
            run_frame = 0u;
            anim_tick = 0u;
        }

        update_camera(world_x, world_y);
        wait_vbl_done();
    }
}
