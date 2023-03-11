#include <obs-module.h>
#include <obs-source.h>

#include <util/config-file.h>
#include <util/platform.h>
#include <util/threading.h>

#include <leptonica/allheaders.h>

#include "images.h"

#define PROJECT_VERSION "1.2.0"

#define DEBUG_FRAME_INTERVAL        300
#define DEBUG_SAVE_PATH             "C:\\Temp"
#define DEBUG_SAVE_PATH_NAME_LEN    128

#define PSNR_THRESHOLD_VALUE        20

#define write_log(log_level, format, ...) blog(log_level, "[apex-game] " format, ##__VA_ARGS__)

#define bdebug(format, ...) write_log(LOG_DEBUG, format, ##__VA_ARGS__)
#define binfo(format, ...) write_log(LOG_INFO, format, ##__VA_ARGS__)
#define bwarn(format, ...) write_log(LOG_WARNING, format, ##__VA_ARGS__)
#define berr(format, ...) write_log(LOG_ERROR, format, ##__VA_ARGS__)

enum character_name
{
    BLOODHOUND,
    GIBRALTAR,
    LIFELINE,
    PATHFINDER,
    WRAITH,
    BANGALORE,
    CAUSTIC,
    MIRAGE,
    OCTANE,
    WATTSON,
    CRYPTO,
    REVENANT,
    LOBA,
    RAMPART,
    HORIZON,
    FUSE,
    VALKYRIE,
    SEER,
    ASH,
    MADMAGGIE,
    NEWCASTLE,
    VANTAGE,
    CATALYST,

    CHARACTERS_NUM
};
typedef enum character_name character_name_t;

const char *character_name_str[CHARACTERS_NUM] =
{
    "bloodhound",
    "gibraltar",
    "lifeline",
    "pathfinder",
    "wraith",
    "bangalore",
    "caustic",
    "mirage",
    "octane",
    "wattson",
    "crypto",
    "revenant",
    "loba",
    "rampart",
    "horizon",
    "fuse",
    "valkyrie",
    "seer",
    "ash",
    "madmaggie",
    "newcastle",
    "vantage",
    "catalyst",
};

enum banner_position
{
    BANNER_GAME,
    BANNER_LOOTING,
    BANNER_INVENTORY,
    BANNER_MAP,

    BANNER_POSITION_NUM
};
typedef enum banner_position banner_position_t;

enum area_name
{
    MAP_GAME_BUTTON,
    GRENADE_GAME_BUTTON,
    ESC_LOOTING_BUTTON,
    ESC_INVENTORY_BUTTON,
    GRAYBAR_INVENTORY_BUTTON,
    M_MAP_BUTTON,
    PG_BANNER_IMAGE,
    PAD_MAP_BUTTON,
    PAD_LOOTING_BUTTON,
    PAD_INVENTORY_BUTTON,
    PAD_TACTICAL_BUTTON,

    AREAS_NUM
};
typedef enum area_name area_name_t;

const char *area_name_str[] =
{
    "MAP_GAME_BUTTON",
    "GRENADE_GAME_BUTTON",
    "ESC_LOOTING_BUTTON",
    "ESC_INVENTORY_BUTTON",
    "GRAYBAR_INVENTORY_BUTTON",
    "M_MAP_BUTTON",
    "PG_BANNER_IMAGE",
    "PAD_MAP_BUTTON",
    "PAD_LOOTING_BUTTON",
    "PAD_INVENTORY_BUTTON",
    "PAD_TACTICAL_BUTTON"
};

enum input_device
{
    MOUSE_AND_KEYBOARD,
    PLAY_STATION_PAD,

    INPUT_DEVICES_NUM
};

struct area
{
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
};
typedef struct area area_t;

struct apex_game_filter_context
{
    PIX *image;
    PIX *banner_references[AREAS_NUM];
    PIX *pg_references[CHARACTERS_NUM];
    obs_source_t *source;
    obs_weak_source_t *target_sources[BANNER_POSITION_NUM];
    uint8_t *video_data;
    uint32_t video_linesize;
    uint32_t width;
    uint32_t height;
    enum input_device input;
    gs_texrender_t *texrender;
    gs_stagesurf_t *stagesurface;
    bool closing;
    bool debug_mode;
    uint32_t debug_counter;
    const area_t *areas;
};
typedef struct apex_game_filter_context apex_game_filter_context_t;

#define MAP_GAME_BUTTON_X               52
#define MAP_GAME_BUTTON_Y               26
#define MAP_GAME_BUTTON_W               20
#define MAP_GAME_BUTTON_H               20

#define GRENADE_GAME_BUTTON_X           1417
#define GRENADE_GAME_BUTTON_Y           1035
#define GRENADE_GAME_BUTTON_W           20
#define GRENADE_GAME_BUTTON_H           20

#define ESC_LOOTING_BUTTON_X_IT         516
#define ESC_LOOTING_BUTTON_X_EN         527
#define ESC_LOOTING_BUTTON_Y            965
#define ESC_LOOTING_BUTTON_W            43
#define ESC_LOOTING_BUTTON_H            26

#define ESC_INVENTORY_BUTTON_X_IT       81
#define ESC_INVENTORY_BUTTON_X_EN       62
#define ESC_INVENTORY_BUTTON_Y          1034
#define ESC_INVENTORY_BUTTON_W          47
#define ESC_INVENTORY_BUTTON_H          30

#define GRAYBAR_INVENTORY_BUTTON_X      149
#define GRAYBAR_INVENTORY_BUTTON_Y      839
#define GRAYBAR_INVENTORY_BUTTON_W      200
#define GRAYBAR_INVENTORY_BUTTON_H      1

#define M_MAP_BUTTON_X                  63
#define M_MAP_BUTTON_Y                  1037
#define M_MAP_BUTTON_W                  22
#define M_MAP_BUTTON_H                  22

#define PG_BANNER_IMAGE_X               110
#define PG_BANNER_IMAGE_Y               970
#define PG_BANNER_IMAGE_W               28
#define PG_BANNER_IMAGE_H               36

#define PAD_MAP_BUTTON_X                55
#define PAD_MAP_BUTTON_Y                1022
#define PAD_MAP_BUTTON_W                52
#define PAD_MAP_BUTTON_H                51

#define PAD_LOOTING_BUTTON_X_IT         525
#define PAD_LOOTING_BUTTON_X_EN         536
#define PAD_LOOTING_BUTTON_Y            968
#define PAD_LOOTING_BUTTON_W            24
#define PAD_LOOTING_BUTTON_H            20

#define PAD_INVENTORY_BUTTON_X_IT       92
#define PAD_INVENTORY_BUTTON_X_EN       73
#define PAD_INVENTORY_BUTTON_Y          1039
#define PAD_INVENTORY_BUTTON_W          24
#define PAD_INVENTORY_BUTTON_H          20

#define PAD_TACTICAL_BUTTON_X_IT        604
#define PAD_TACTICAL_BUTTON_X_EN        604
#define PAD_TACTICAL_BUTTON_Y           1040
#define PAD_TACTICAL_BUTTON_W           22
#define PAD_TACTICAL_BUTTON_H           12

static const area_t areas_en[AREAS_NUM] =
{
    [MAP_GAME_BUTTON] =         { MAP_GAME_BUTTON_X,            MAP_GAME_BUTTON_Y,          MAP_GAME_BUTTON_W,          MAP_GAME_BUTTON_H           },
    [GRENADE_GAME_BUTTON] =     { GRENADE_GAME_BUTTON_X,        GRENADE_GAME_BUTTON_Y,      GRENADE_GAME_BUTTON_W,      GRENADE_GAME_BUTTON_H       },
    [ESC_LOOTING_BUTTON] =      { ESC_LOOTING_BUTTON_X_EN,      ESC_LOOTING_BUTTON_Y,       ESC_LOOTING_BUTTON_W,       ESC_LOOTING_BUTTON_H        },
    [ESC_INVENTORY_BUTTON] =    { ESC_INVENTORY_BUTTON_X_EN,    ESC_INVENTORY_BUTTON_Y,     ESC_INVENTORY_BUTTON_W,     ESC_INVENTORY_BUTTON_H      },
    [GRAYBAR_INVENTORY_BUTTON] ={ GRAYBAR_INVENTORY_BUTTON_X,   GRAYBAR_INVENTORY_BUTTON_Y, GRAYBAR_INVENTORY_BUTTON_W, GRAYBAR_INVENTORY_BUTTON_H  },
    [M_MAP_BUTTON] =            { M_MAP_BUTTON_X,               M_MAP_BUTTON_Y,             M_MAP_BUTTON_W,             M_MAP_BUTTON_H              },
    [PG_BANNER_IMAGE] =         { PG_BANNER_IMAGE_X,            PG_BANNER_IMAGE_Y,          PG_BANNER_IMAGE_W,          PG_BANNER_IMAGE_H           },
    [PAD_MAP_BUTTON] =          { PAD_MAP_BUTTON_X,             PAD_MAP_BUTTON_Y,           PAD_MAP_BUTTON_W,           PAD_MAP_BUTTON_H            },
    [PAD_LOOTING_BUTTON] =      { PAD_LOOTING_BUTTON_X_EN,      PAD_LOOTING_BUTTON_Y,       PAD_LOOTING_BUTTON_W,       PAD_LOOTING_BUTTON_H        },
    [PAD_INVENTORY_BUTTON] =    { PAD_INVENTORY_BUTTON_X_EN,    PAD_INVENTORY_BUTTON_Y,     PAD_INVENTORY_BUTTON_W,     PAD_INVENTORY_BUTTON_H      },
    [PAD_TACTICAL_BUTTON] =     { PAD_TACTICAL_BUTTON_X_IT,     PAD_TACTICAL_BUTTON_Y,      PAD_TACTICAL_BUTTON_W,      PAD_TACTICAL_BUTTON_H       }
};

static const area_t areas_it[AREAS_NUM] =
{
    [MAP_GAME_BUTTON] =         { MAP_GAME_BUTTON_X,            MAP_GAME_BUTTON_Y,          MAP_GAME_BUTTON_W,          MAP_GAME_BUTTON_H           },
    [GRENADE_GAME_BUTTON] =     { GRENADE_GAME_BUTTON_X,        GRENADE_GAME_BUTTON_Y,      GRENADE_GAME_BUTTON_W,      GRENADE_GAME_BUTTON_H       },
    [ESC_LOOTING_BUTTON] =      { ESC_LOOTING_BUTTON_X_IT,      ESC_LOOTING_BUTTON_Y,       ESC_LOOTING_BUTTON_W,       ESC_LOOTING_BUTTON_H        },
    [ESC_INVENTORY_BUTTON] =    { ESC_INVENTORY_BUTTON_X_IT,    ESC_INVENTORY_BUTTON_Y,     ESC_INVENTORY_BUTTON_W,     ESC_INVENTORY_BUTTON_H      },
    [GRAYBAR_INVENTORY_BUTTON] ={ GRAYBAR_INVENTORY_BUTTON_X,   GRAYBAR_INVENTORY_BUTTON_Y, GRAYBAR_INVENTORY_BUTTON_W, GRAYBAR_INVENTORY_BUTTON_H  },
    [M_MAP_BUTTON] =            { M_MAP_BUTTON_X,               M_MAP_BUTTON_Y,             M_MAP_BUTTON_W,             M_MAP_BUTTON_H              },
    [PG_BANNER_IMAGE] =         { PG_BANNER_IMAGE_X,            PG_BANNER_IMAGE_Y,          PG_BANNER_IMAGE_W,          PG_BANNER_IMAGE_H           },
    [PAD_MAP_BUTTON] =          { PAD_MAP_BUTTON_X,             PAD_MAP_BUTTON_Y,           PAD_MAP_BUTTON_W,           PAD_MAP_BUTTON_H            },
    [PAD_LOOTING_BUTTON] =      { PAD_LOOTING_BUTTON_X_IT,      PAD_LOOTING_BUTTON_Y,       PAD_LOOTING_BUTTON_W,       PAD_LOOTING_BUTTON_H        },
    [PAD_INVENTORY_BUTTON] =    { PAD_INVENTORY_BUTTON_X_IT,    PAD_INVENTORY_BUTTON_Y,     PAD_INVENTORY_BUTTON_W,     PAD_INVENTORY_BUTTON_H      },
    [PAD_TACTICAL_BUTTON] =     { PAD_TACTICAL_BUTTON_X_IT,     PAD_TACTICAL_BUTTON_Y,      PAD_TACTICAL_BUTTON_W,      PAD_TACTICAL_BUTTON_H       }
};

static void debug_step(apex_game_filter_context_t *filter)
{
    filter->debug_counter++;
}

static bool debug_should_save(apex_game_filter_context_t *filter)
{
    if (!filter->debug_mode)
        return false;

    if ((filter->debug_counter % DEBUG_FRAME_INTERVAL) != 0)
        return false;

    return true;
}

static bool debug_should_print(apex_game_filter_context_t *filter)
{
    if (!filter->debug_mode)
        return false;

    if ((filter->debug_counter % DEBUG_FRAME_INTERVAL) != 0)
        return false;

    return true;
}

static void fill_area(PIX *image, uint32_t *raw_image, unsigned width, unsigned height, const area_t *a, int xoff)
{
    if (pixGetHeight(image) != height || pixGetWidth(image) != width)
        pixSetResolution(image, width, height);

    for (unsigned x = a->x + xoff; x < (a->x + xoff + a->w); x++) {
        for (unsigned y = a->y; y < (a->y + a->h); y++) {
            uint8_t *rgb = &raw_image[y * width + x];
            uint8_t r = rgb[0];
            uint8_t g = rgb[1];
            uint8_t b = rgb[2];
            pixSetRGBPixel(image, x, y, r, g, b);
        }
    }
}

static float compare_psnr_value_of_area_with_offset(PIX *image, PIX *reference, const area_t *a, int xoff)
{
    BOX *box = boxCreate(a->x + xoff, a->y, a->w, a->h);
    PIX *rectangle = pixClipRectangle(image, box, NULL);

    float psnr;
    pixGetPSNR(rectangle, reference, 1, &psnr);

    boxDestroy(&box);
    pixDestroy(&rectangle);

    return psnr;
}

static float compare_psnr_value_of_area(PIX *image, PIX *reference, const area_t *a)
{
    return compare_psnr_value_of_area_with_offset(image, reference, a, 0);
}

static void save_ref_image(apex_game_filter_context_t *filter, area_name_t an)
{
    char filename[DEBUG_SAVE_PATH_NAME_LEN];

    const char *name = area_name_str[an];

    snprintf(filename, DEBUG_SAVE_PATH_NAME_LEN, "%s\\ref_%s.png", DEBUG_SAVE_PATH, name);

    pixWrite(filename, filter->banner_references[an], IFF_PNG);
}

static void save_image(apex_game_filter_context_t *filter, area_name_t an)
{
    char filename[DEBUG_SAVE_PATH_NAME_LEN];

    const area_t *a = &(filter->areas[an]);
    const char *name = area_name_str[an];

    BOX *box = boxCreate(a->x, a->y, a->w, a->h);
    PIX *rectangle = pixClipRectangle(filter->image, box, NULL);

    snprintf(filename, DEBUG_SAVE_PATH_NAME_LEN, "%s\\image_%s.png", DEBUG_SAVE_PATH, name);

    pixWrite(filename, rectangle, IFF_PNG);

    boxDestroy(&box);
    pixDestroy(&rectangle);
}

static const char *apex_game_filter_get_name(void *unused)
{
    return "Apex Game";
}

static void set_source_status(obs_weak_source_t *source, bool status)
{
    obs_source_t *s = obs_weak_source_get_source(source);

    obs_source_set_enabled(s, status);

    obs_source_release(s);
}

static bool get_area_status_withoffset(apex_game_filter_context_t *filter, area_name_t an, int xoff)
{
    const area_t *a = &(filter->areas[an]);

    fill_area(filter->image, filter->video_data, filter->width, filter->height, a, xoff);
    float psnr = compare_psnr_value_of_area_with_offset(filter->image, filter->banner_references[an], a, xoff);

    bool match = psnr > PSNR_THRESHOLD_VALUE;

    if (debug_should_print(filter))
        binfo("%s: %f", area_name_str[an], psnr);

    if (debug_should_save(filter)) {
        save_image(filter, an);
        save_ref_image(filter, an);
    }

    return match;
}

static bool get_area_status(apex_game_filter_context_t *filter, area_name_t an)
{
    return get_area_status_withoffset(filter, an, 0);
}

static void check_banner(apex_game_filter_context_t *filter, area_name_t an, banner_position_t bp)
{
    bool enable_target_source = get_area_status(filter, an);

    set_source_status(filter->target_sources[bp], enable_target_source);
}

static character_name_t get_pg_showed(apex_game_filter_context_t *filter)
{
    character_name_t pg;

    fill_area(filter->image, filter->video_data, filter->width, filter->height, &(filter->areas[PG_BANNER_IMAGE]), 0);

    if (debug_should_save(filter)) {
        save_image(filter, PG_BANNER_IMAGE);
        save_ref_image(filter, PG_BANNER_IMAGE);
    }

    for (pg = 0; pg < CHARACTERS_NUM; pg++) {
        float psnr = compare_psnr_value_of_area(filter->image, filter->pg_references[pg], &(filter->areas[PG_BANNER_IMAGE]));

        if (debug_should_print(filter))
            binfo("%s: %f", character_name_str[pg], psnr);

        if (psnr > PSNR_THRESHOLD_VALUE)
            break;
    }

    return pg;
}

static void match_mk(apex_game_filter_context_t *filter)
{
    /*
     * if the area of interest matches the reference image we are
     * 100% sure that we can move to that scene
     */
    check_banner(filter, ESC_LOOTING_BUTTON, BANNER_LOOTING);
    check_banner(filter, M_MAP_BUTTON, BANNER_MAP);

    /*
     * if inventory ESC button is found a further check must performed if inventory tab
     * is selected, otherwise in the other tabs player banner is not showed
     */
    if (get_area_status(filter, ESC_INVENTORY_BUTTON)) {
        if (get_area_status(filter, GRAYBAR_INVENTORY_BUTTON))
            set_source_status(filter->target_sources[BANNER_INVENTORY], true);
        else
            set_source_status(filter->target_sources[BANNER_INVENTORY], false);
    } else {
        set_source_status(filter->target_sources[BANNER_INVENTORY], false);
    }

    /*
     * in game matching is a little bit more difficult since when pg info button was removed
     * first we try to identify the pg in the bottom left part of the screen, this should cover the
     * majority of occurreciens.
     * in some situations the pg is not recognizable (ie. when player receives damage the pg image
     * pulses with a red color making recognition unreliable), therefore we use the M button top
     * left or the G under grenades slot.
     */
    bool enable_banner_game = false;

    character_name_t pg = get_pg_showed(filter);

    if (pg != CHARACTERS_NUM) {
        enable_banner_game = true;
    } else {
        bool map_game = get_area_status(filter, MAP_GAME_BUTTON);
        bool grenade_game = get_area_status(filter, GRENADE_GAME_BUTTON);

        enable_banner_game = map_game || grenade_game;
    }

    set_source_status(filter->target_sources[BANNER_GAME], enable_banner_game);
}

static void match_ps4pad(apex_game_filter_context_t *filter)
{
    /*
     * if the area of interest matches the reference image we are
     * 100% sure that we can move to that scene
     */
    check_banner(filter, PAD_MAP_BUTTON, BANNER_MAP);

    /*
     * there's a funny behaviour if you use m&k and pad at the same time,
     * the absolute position of the button moves by 8 pixels wheter or not
     * you move mouse, handle this case by trying to recognize the reference
     * image twice considering that offset
     */
    bool pad_looting = get_area_status(filter, PAD_LOOTING_BUTTON);
    bool pad_looting_offset = get_area_status_withoffset(filter, PAD_LOOTING_BUTTON, 8);
    bool activate_looting = pad_looting || pad_looting_offset;

    set_source_status(filter->target_sources[BANNER_LOOTING], activate_looting);

    /*
     * inventory for pad is difficult, the absolute position of the pg HUD moves when
     * analog joystick is moved making recognition of this HUD not perfect
     * as for now recognize only the inventory button, a much complex analysis is
     * necessary to move the source in the correct position
     */
    bool pad_inventory = get_area_status(filter, PAD_INVENTORY_BUTTON);
    bool pad_inventory_offset = get_area_status_withoffset(filter, PAD_INVENTORY_BUTTON, -8);
    bool activate_inventory = pad_inventory || pad_inventory_offset;

    set_source_status(filter->target_sources[BANNER_INVENTORY], activate_inventory);

    /*
     * in game matching is a little bit more difficult since when pg info button was removed
     * first we try to identify the pg in the bottom left part of the screen, this should cover the
     * majority of occurreciens.
     * in some situations the pg is not recognizable (ie. when player receives damage the pg image
     * pulses with a red color making recognition unreliable), therefore we use the L1 button
     * of the tactical ability
     */
    bool enable_banner_game = false;

    character_name_t pg = get_pg_showed(filter);

    if (pg != CHARACTERS_NUM) {
        enable_banner_game = true;
    } else {
        enable_banner_game = get_area_status(filter, PAD_TACTICAL_BUTTON);
    }

    set_source_status(filter->target_sources[BANNER_GAME], enable_banner_game);
}

static void apex_game_filter_offscreen_render(void *data, uint32_t cx, uint32_t cy)
{
    UNUSED_PARAMETER(cx);
    UNUSED_PARAMETER(cy);

    apex_game_filter_context_t *filter = data;

    if (filter->closing)
        return;

    if (!obs_source_enabled(filter->source))
        return;

    obs_source_t *parent = obs_filter_get_parent(filter->source);
    if (!parent)
        return;

    if (!filter->width || !filter->height)
        return;

    if (filter->width != 1920 || filter->height != 1080)
        return;

    gs_texrender_reset(filter->texrender);

    if (!gs_texrender_begin(filter->texrender, filter->width, filter->height))
        return;

    if (debug_should_print(filter))
        binfo("frame: %d", filter->debug_counter);

    struct vec4 background;

    vec4_zero(&background);

    gs_clear(GS_CLEAR_COLOR, &background, 0.0f, 0);
    gs_ortho(0.0f, (float)filter->width, 0.0f, (float)filter->height, -100.0f, 100.0f);

    gs_blend_state_push();
    gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);

    obs_source_video_render(parent);

    gs_blend_state_pop();
    gs_texrender_end(filter->texrender);

    uint32_t surface_width = gs_stagesurface_get_width(filter->stagesurface);
    uint32_t surface_height = gs_stagesurface_get_height(filter->stagesurface);

    if (surface_width != filter->width || surface_height != filter->height) {
        gs_stagesurface_destroy(filter->stagesurface);
        filter->stagesurface = NULL;
    }

    if (filter->video_data) {
        gs_stagesurface_unmap(filter->stagesurface);
        filter->video_data = NULL;
    }

    if (!filter->stagesurface)
        filter->stagesurface = gs_stagesurface_create(filter->width, filter->height, GS_RGBA);

    gs_stage_texture(filter->stagesurface, gs_texrender_get_texture(filter->texrender));

    if (!gs_stagesurface_map(filter->stagesurface, &filter->video_data, &filter->video_linesize))
        return;

    if (filter->input == MOUSE_AND_KEYBOARD)
        match_mk(filter);
    else if (filter->input == PLAY_STATION_PAD)
        match_ps4pad(filter);

    debug_step(filter);
}

static void update_source(obs_data_t *settings, const char *set_name, obs_weak_source_t **s)
{
    const char *set_source_name = obs_data_get_string(settings, set_name);

    if (!strlen(set_source_name)) {
        if (*s) {
            obs_weak_source_release(*s);
            *s = NULL;
        }
    } else {
        obs_source_t *cur_source = obs_weak_source_get_source(*s);

        if (cur_source)
            obs_source_release(cur_source);

        if (!cur_source || strcmp(set_source_name, obs_source_get_name(cur_source)) != 0) {
            if (*s) {
                obs_weak_source_release(*s);
                *s = NULL;
            }

            cur_source = obs_get_source_by_name(set_source_name);
            if (cur_source) {
                *s = obs_source_get_weak_source(cur_source);
                obs_source_release(cur_source);
            }
        }
    }
}

static void apex_game_filter_update(void *data, obs_data_t *settings)
{
    apex_game_filter_context_t *filter = data;

    binfo("update");

    update_source(settings, "game_source", &filter->target_sources[BANNER_GAME]);
    update_source(settings, "looting_source", &filter->target_sources[BANNER_LOOTING]);
    update_source(settings, "inventory_source", &filter->target_sources[BANNER_INVENTORY]);
    update_source(settings, "map_source", &filter->target_sources[BANNER_MAP]);

    filter->debug_mode = obs_data_get_bool(settings, "debug_mode");

    const char *game_lang = obs_data_get_string(settings, "game_lang");

    if (strcmp(game_lang, "it") == 0)
        filter->areas = areas_it;
    else if (strcmp(game_lang, "en") == 0)
        filter->areas = areas_en;
    else
        filter->areas = areas_en;

    const char *game_input = obs_data_get_string(settings, "game_input");

    if (strcmp(game_input, "mk") == 0)
        filter->input = MOUSE_AND_KEYBOARD;
    else if (strcmp(game_input, "ps-pad") == 0)
        filter->input = PLAY_STATION_PAD;
    else
        filter->input = MOUSE_AND_KEYBOARD;
}

static void apex_game_filter_defaults(obs_data_t *settings)
{
    UNUSED_PARAMETER(settings);
}

static void *apex_game_filter_create(obs_data_t *settings, obs_source_t *source)
{
    binfo("creating new filter");

    apex_game_filter_context_t *filter = bzalloc(sizeof(apex_game_filter_context_t));

    filter->source = source;
    filter->texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

    filter->image = pixCreate(1920, 1080, 32);

    filter->banner_references[MAP_GAME_BUTTON] = pixReadMemBmp(ref_game_map_bmp, ref_game_map_bmp_size);
    filter->banner_references[GRENADE_GAME_BUTTON] = pixReadMemBmp(ref_game_grenade_bmp, ref_game_grenade_bmp_size);
    filter->banner_references[ESC_LOOTING_BUTTON] = pixReadMemBmp(ref_looting_bmp, ref_looting_bmp_size);
    filter->banner_references[ESC_INVENTORY_BUTTON] = pixReadMemBmp(ref_inventory_bmp, ref_inventory_bmp_size);
    filter->banner_references[GRAYBAR_INVENTORY_BUTTON] = pixReadMemBmp(ref_graybar_inventory_bmp, ref_graybar_inventory_bmp_size);
    filter->banner_references[M_MAP_BUTTON] = pixReadMemBmp(ref_map_bmp, ref_map_bmp_size);
    filter->banner_references[PAD_MAP_BUTTON] = pixReadMemBmp(ref_pad_map_bmp, ref_pad_map_bmp_size);
    filter->banner_references[PAD_LOOTING_BUTTON] = pixReadMemBmp(ref_pad_looting_bmp, ref_pad_looting_bmp_size);
    filter->banner_references[PAD_INVENTORY_BUTTON] = pixReadMemBmp(ref_pad_inventory_bmp, ref_pad_inventory_bmp_size);
    filter->banner_references[PAD_TACTICAL_BUTTON] = pixReadMemBmp(ref_pad_tactical_bmp, ref_pad_tactical_bmp_size);

    filter->pg_references[BLOODHOUND] = pixReadMemBmp(game_bloodhound_bmp, game_bloodhound_bmp_size);
    filter->pg_references[GIBRALTAR] = pixReadMemBmp(game_gibraltar_bmp, game_gibraltar_bmp_size);
    filter->pg_references[LIFELINE] = pixReadMemBmp(game_lifeline_bmp, game_lifeline_bmp_size);
    filter->pg_references[PATHFINDER] = pixReadMemBmp(game_pathfinder_bmp, game_pathfinder_bmp_size);
    filter->pg_references[WRAITH] = pixReadMemBmp(game_wraith_bmp, game_wraith_bmp_size);
    filter->pg_references[BANGALORE] = pixReadMemBmp(game_bangalore_bmp, game_bangalore_bmp_size);
    filter->pg_references[CAUSTIC] = pixReadMemBmp(game_caustic_bmp, game_caustic_bmp_size);
    filter->pg_references[MIRAGE] = pixReadMemBmp(game_mirage_bmp, game_mirage_bmp_size);
    filter->pg_references[OCTANE] = pixReadMemBmp(game_octane_bmp, game_octane_bmp_size);
    filter->pg_references[WATTSON] = pixReadMemBmp(game_wattson_bmp, game_wattson_bmp_size);
    filter->pg_references[CRYPTO] = pixReadMemBmp(game_crypto_bmp, game_crypto_bmp_size);
    filter->pg_references[REVENANT] = pixReadMemBmp(game_revenant_bmp, game_revenant_bmp_size);
    filter->pg_references[LOBA] = pixReadMemBmp(game_loba_bmp, game_loba_bmp_size);
    filter->pg_references[RAMPART] = pixReadMemBmp(game_rampart_bmp, game_rampart_bmp_size);
    filter->pg_references[HORIZON] = pixReadMemBmp(game_horizon_bmp, game_horizon_bmp_size);
    filter->pg_references[FUSE] = pixReadMemBmp(game_fuse_bmp, game_fuse_bmp_size);
    filter->pg_references[VALKYRIE] = pixReadMemBmp(game_valkyrie_bmp, game_valkyrie_bmp_size);
    filter->pg_references[SEER] = pixReadMemBmp(game_seer_bmp, game_seer_bmp_size);
    filter->pg_references[ASH] = pixReadMemBmp(game_ash_bmp, game_ash_bmp_size);
    filter->pg_references[MADMAGGIE] = pixReadMemBmp(game_madmaggie_bmp, game_madmaggie_bmp_size);
    filter->pg_references[NEWCASTLE] = pixReadMemBmp(game_newcastle_bmp, game_newcastle_bmp_size);
    filter->pg_references[VANTAGE] = pixReadMemBmp(game_vantage_bmp, game_vantage_bmp_size);
    filter->pg_references[CATALYST] = pixReadMemBmp(game_catalyst_bmp, game_catalyst_bmp_size);

    filter->debug_mode = false;
    filter->debug_counter = 0;

    apex_game_filter_update(filter, settings);

    obs_add_main_render_callback(apex_game_filter_offscreen_render, filter);

    return filter;
}

static release_source(obs_weak_source_t *weak_source)
{
    if (weak_source)
    {
        obs_source_t *source = obs_weak_source_get_source(weak_source);
        obs_source_set_enabled(source, true);
        obs_source_release(source);
        obs_weak_source_release(weak_source);
    }
}

static void apex_game_filter_destroy(void *data)
{
    binfo("destroing filter");

    apex_game_filter_context_t *filter = data;

    filter->closing = true;

    pixDestroy(&filter->image);

    for (enum area_name an = 0; an < AREAS_NUM; an++)
        pixDestroy(&filter->banner_references[an]);

    for (character_name_t pg = 0; pg < CHARACTERS_NUM; pg++)
        pixDestroy(&filter->pg_references[pg]);

    obs_remove_main_render_callback(apex_game_filter_offscreen_render, filter);

    release_source(filter->target_sources[BANNER_GAME]);
    release_source(filter->target_sources[BANNER_LOOTING]);
    release_source(filter->target_sources[BANNER_INVENTORY]);
    release_source(filter->target_sources[BANNER_MAP]);

    obs_enter_graphics();

    gs_stagesurface_unmap(filter->stagesurface);
    gs_stagesurface_destroy(filter->stagesurface);
    gs_texrender_destroy(filter->texrender);

    obs_leave_graphics();

    bfree(filter);
}

static void apex_game_filter_tick(void *data, float seconds)
{
    UNUSED_PARAMETER(seconds);

    apex_game_filter_context_t *filter = data;

    if (filter->closing)
        return;

    obs_source_t *parent = obs_filter_get_parent(filter->source);

    if (!parent)
        return;

    uint32_t width = obs_source_get_width(parent);
    uint32_t height = obs_source_get_height(parent);

    width += (width & 1);
    height += (height & 1);

    if (filter->width != width || filter->height != height) {
        bdebug("new size, width: %d, height: %d", width, height);

        filter->width = width;
        filter->height = height;
    }
}

static bool list_add_sources(void *data, obs_source_t *source)
{
    obs_property_t *p = data;
    const uint32_t flags = obs_source_get_output_flags(source);
    const char *source_name = obs_source_get_name(source);

    bool is_composite_source = (flags & OBS_SOURCE_COMPOSITE) != 0;
    bool is_video = (flags & OBS_SOURCE_VIDEO) != 0;

    if (is_composite_source || is_video)
        obs_property_list_add_string(p, source_name, source_name);

    return true;
}

static obs_properties_t *apex_game_filter_properties(void *data)
{
    apex_game_filter_context_t *filter = data;
    obs_property_t *p;

    obs_properties_t *props = obs_properties_create();

    obs_properties_t *group_1 = obs_properties_create();
    obs_properties_t *group_2 = obs_properties_create();

    obs_properties_add_group(props, "game_settin", "Game settings", OBS_GROUP_NORMAL, group_1);

    p = obs_properties_add_list(group_1, "game_input", "Input device", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(p, "Mouse and Keyboard", "mk");
    obs_property_list_add_string(p, "PlayStation Pad", "ps-pad");

    p = obs_properties_add_list(group_1, "game_lang", "Game Language", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(p, "Italiano", "it");
    obs_property_list_add_string(p, "English", "en");

    obs_properties_add_group(props, "sources", "Sources configuration", OBS_GROUP_NORMAL, group_2);

    p = obs_properties_add_list(group_2, "game_source", "Game Overlay Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    p = obs_properties_add_list(group_2, "inventory_source", "Game Inventory Overlay Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    p = obs_properties_add_list(group_2, "looting_source", "Game Looting Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    p = obs_properties_add_list(group_2, "map_source", "Game Map Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    obs_properties_add_bool(props, "debug_mode", "Enable debug messages");

    return props;
}

static void apex_game_filter_render(void *data, gs_effect_t *effect)
{
    UNUSED_PARAMETER(effect);

    apex_game_filter_context_t *filter = data;

    obs_source_skip_video_filter(filter->source);
}

static void apex_game_filter_filter_remove(void *data, obs_source_t *parent)
{
    UNUSED_PARAMETER(parent);

    apex_game_filter_context_t *filter = data;

    filter->closing = true;

    obs_remove_main_render_callback(apex_game_filter_offscreen_render, filter);
}

struct obs_source_info apex_game_filter_info = {
    .id = "apex_game_filter",
    .type = OBS_SOURCE_TYPE_FILTER,
    .output_flags = OBS_SOURCE_VIDEO,
    .get_name = apex_game_filter_get_name,
    .create = apex_game_filter_create,
    .destroy = apex_game_filter_destroy,
    .update = apex_game_filter_update,
    .load = apex_game_filter_update,
    .get_defaults = apex_game_filter_defaults,
    .video_render = apex_game_filter_render,
    .video_tick = apex_game_filter_tick,
    .get_properties = apex_game_filter_properties,
    .filter_remove = apex_game_filter_filter_remove,
};

OBS_DECLARE_MODULE()

MODULE_EXPORT const char *obs_module_description(void)
{
    return "Apex Game Filter";
}

bool obs_module_load(void)
{
    binfo("loaded version %s", PROJECT_VERSION);

    obs_register_source(&apex_game_filter_info);

    return true;
}

void obs_module_post_load(void)
{
    binfo("module post loading");
}

void obs_module_unload(void)
{
    binfo("module unloading");
}
