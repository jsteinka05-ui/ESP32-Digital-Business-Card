#include "transitions.h"
#include "slides.h"
#include "ili9488.h"

enum {
    TILE_SIZE = 8,
    TILE_COLUMNS = ILI9488_WIDTH / TILE_SIZE,
    TILE_ROWS = ILI9488_HEIGHT / TILE_SIZE,
    TILE_COUNT = TILE_COLUMNS * TILE_ROWS,
    TILES_PER_STEP = 16,
    WIPE_ROWS = 8
};

_Static_assert(ILI9488_WIDTH % TILE_SIZE == 0 && ILI9488_HEIGHT % TILE_SIZE == 0,
               "Panel dimensions must fit whole tiles");

typedef enum {
    TRANSITION_IDLE,
    TRANSITION_WIPE,
    TRANSITION_REVEAL
} transition_phase_t;

static const int64_t STEP_US = 10000;
static transition_phase_t phase;
static size_t target_slide;
static int wipe_y;
static unsigned tile_position;
static int64_t next_step_us;
// One ordering array reused for every transition
static uint16_t tile_order[TILE_COUNT];

static void shuffle_tiles(uint32_t seed)
{
    for (unsigned i = 0; i < TILE_COUNT; i++) {
        tile_order[i] = i;
    }
    // A local shuffle keeps animation order independent of wireless state
    uint32_t random = seed ? seed : 0x6D2B79F5;
    for (unsigned i = TILE_COUNT - 1; i > 0; i--) {
        random ^= random << 13;
        random ^= random >> 17;
        random ^= random << 5;
        unsigned other = random % (i + 1);
        uint16_t saved = tile_order[i];
        tile_order[i] = tile_order[other];
        tile_order[other] = saved;
    }
}

bool transitions_is_active(void)
{
    return phase != TRANSITION_IDLE;
}

void transitions_cancel(void)
{
    phase = TRANSITION_IDLE;
}

esp_err_t transitions_start(size_t slide, int64_t now_us)
{
    if (transitions_is_active()) {
        return ESP_ERR_INVALID_STATE;
    }
    if (slide >= slides_count()) {
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t result = slides_validate();
    if (result != ESP_OK) {
        return result;
    }
    target_slide = slide;
    wipe_y = 0;
    tile_position = 0;
    next_step_us = now_us;
    shuffle_tiles((uint32_t)now_us ^ (uint32_t)((uint64_t)now_us >> 32));
    phase = TRANSITION_WIPE;
    return ESP_OK;
}

esp_err_t transitions_update(int64_t now_us)
{
    if (!transitions_is_active() || now_us < next_step_us) {
        return ESP_OK;
    }
    // Never catch up with an unbounded burst after a delayed loop
    next_step_us = now_us + STEP_US;
    if (phase == TRANSITION_WIPE) {
        esp_err_t result = ili9488_fill_rect(0, wipe_y, ILI9488_WIDTH, WIPE_ROWS, 0, 0, 0);
        if (result != ESP_OK) {
            transitions_cancel();
            return result;
        }
        wipe_y += WIPE_ROWS;
        if (wipe_y == ILI9488_HEIGHT) {
            phase = TRANSITION_REVEAL;
        }
        return ESP_OK;
    }

    uint8_t pixels[TILE_SIZE * TILE_SIZE * 3];
    for (unsigned count = 0; count < TILES_PER_STEP && tile_position < TILE_COUNT; count++) {
        unsigned tile = tile_order[tile_position];
        int x = (tile % TILE_COLUMNS) * TILE_SIZE;
        int y = (tile / TILE_COLUMNS) * TILE_SIZE;
        esp_err_t result = slides_read_region(target_slide, x, y, TILE_SIZE, TILE_SIZE,
                                              pixels, sizeof(pixels));
        if (result == ESP_OK) {
            result = ili9488_draw_rgb888(x, y, TILE_SIZE, TILE_SIZE, pixels, sizeof(pixels));
        }
        if (result != ESP_OK) {
            transitions_cancel();
            return result;
        }
        tile_position++;
    }
    if (tile_position == TILE_COUNT) {
        phase = TRANSITION_IDLE;
    }
    return ESP_OK;
}
