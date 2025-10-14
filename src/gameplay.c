#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>

#include <stdint.h>

#include "Alex_idle_16x16.h"
#include "Alex_run_16x16.h"
#include "gameplay.h"
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

static const uint8_t logic_tile_quads[][4] = {
    { FONT_OFFSET + 0u, FONT_OFFSET + 1u, FONT_OFFSET + 0u, FONT_OFFSET + 1u }, /* water */
    { FONT_OFFSET + 2u, FONT_OFFSET + 2u, FONT_OFFSET + 2u, FONT_OFFSET + 2u }, /* sand  */
    { FONT_OFFSET + 3u, FONT_OFFSET + 3u, FONT_OFFSET + 3u, FONT_OFFSET + 3u }  /* grass */
};

static const uint8_t logic_attr_quads[][4] = {
    { 0u, 0u, 0u, 0u }, /* water palette */
    { 0u, 0u, 0u, 0u }, /* sand palette  */
    { 1u, 1u, 1u, 1u }  /* grass palette */
};

static void blit_logic_tile(uint8_t lx, uint8_t ly, uint8_t vram_x, uint8_t vram_y, uint8_t is_cgb) {
    uint8_t logic = island_map[ly][lx];
    if (logic >= (uint8_t)(sizeof(logic_tile_quads) / sizeof(logic_tile_quads[0]))) {
        logic = M_WATER;
    }
    set_bkg_tiles(vram_x, vram_y, 2u, 2u, logic_tile_quads[logic]);
    if (is_cgb) {
        VBK_REG = 1u;
        set_bkg_tiles(vram_x, vram_y, 2u, 2u, logic_attr_quads[logic]);
        VBK_REG = 0u;
    }
}

static void render_full_map(void) {
    const uint8_t is_cgb = (_cpu == CGB_TYPE);
    for (uint8_t y = 0u; y < MAP_HEIGHT; y++) {
        for (uint8_t x = 0u; x < MAP_WIDTH; x++) {
            blit_logic_tile(x, y, (uint8_t)(x * 2u), (uint8_t)(y * 2u), is_cgb);
        }
    }
}

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
}

static uint8_t can_walk(uint16_t x, uint16_t y) {
    uint16_t px = x >> TILE_SHIFT;
    uint16_t py = y >> TILE_SHIFT;
    if ((px >= MAP_WIDTH) || (py >= MAP_HEIGHT)) {
        return 0u;
    }
    return (island_map[py][px] != M_WATER);
}

void gameplay_screen(void) {
    uint16_t world_x = SCREEN_CENTER_X;
    uint16_t world_y = SCREEN_CENTER_Y;
    uint8_t dir = DIR_FRONT;
    uint8_t run_frame = 0u;
    uint8_t anim_tick = 0u;

    render_full_map();

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
