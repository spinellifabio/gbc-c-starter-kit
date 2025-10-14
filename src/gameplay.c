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

static uint8_t can_walk(uint16_t x, uint16_t y) {
    uint16_t px = x >> TILE_SHIFT;
    uint16_t py = y >> TILE_SHIFT;
    if ((px >= MAP_WIDTH) || (py >= MAP_HEIGHT)) {
        return 0u;
    }
    return (island_map[py][px] != M_WATER);
}

static void reset_viewport_cache(void) {
    last_view_tile_x = 0xFFu;
    last_view_tile_y = 0xFFu;
}

static void render_viewport(void) {
    uint8_t tile_x = (uint8_t)(cam_x >> TILE_SHIFT);
    uint8_t tile_y = (uint8_t)(cam_y >> TILE_SHIFT);
    if ((tile_x == last_view_tile_x) && (tile_y == last_view_tile_y)) {
        return;
    }
    last_view_tile_x = tile_x;
    last_view_tile_y = tile_y;

    ensure_tile_quads();

    const uint8_t is_cgb = (_cpu == CGB_TYPE);
    uint8_t row_tiles[BKG_HW_W];
    uint8_t row_attrs[BKG_HW_W];

    for (uint8_t hy = 0u; hy < BKG_HW_H; hy++) {
        uint8_t logic_y = (uint8_t)(tile_y + (hy >> 1));
        uint8_t quad_row = (uint8_t)((hy & 1u) << 1);
        for (uint8_t hx = 0u; hx < BKG_HW_W; hx++) {
            uint8_t logic_x = (uint8_t)(tile_x + (hx >> 1));
            uint8_t quad_index = (uint8_t)(quad_row + (hx & 1u));

            uint8_t logic = M_WATER;
            if ((logic_x < MAP_WIDTH) && (logic_y < MAP_HEIGHT)) {
                logic = island_map[logic_y][logic_x];
            }
            if (logic >= LOGIC_TILE_COUNT) {
                logic = M_WATER;
            }

            row_tiles[hx] = tile_quads[logic][quad_index];
            if (is_cgb) {
                row_attrs[hx] = attr_quads[logic][quad_index];
            }
        }
        set_bkg_tiles(0u, hy, BKG_HW_W, 1u, row_tiles);
        if (is_cgb) {
            VBK_REG = 1u;
            set_bkg_tiles(0u, hy, BKG_HW_W, 1u, row_attrs);
            VBK_REG = 0u;
        }
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
