#include <obs-module.h>

#include <util/config-file.h>
#include <util/platform.h>
#include <util/threading.h>

#include <leptonica/allheaders.h>

#include "images.h"

#define PROJECT_VERSION "1.0.0"

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
    PGINFO_KEYBIND_BUTTON,
    ESC_LOOTING_BUTTON,
    ESC_INVENTORY_BUTTON,
    M_MAP_BUTTON,
    PG_BANNER_IMAGE,

    AREAS_NUM
};
typedef enum area_name area_name_t;

struct apex_game_filter_context
{
    PIX *image;
    PIX *banner_references[BANNER_POSITION_NUM];
    PIX *pg_references[CHARACTERS_NUM];
    obs_source_t *source;
    obs_weak_source_t *target_sources[BANNER_POSITION_NUM];
    uint8_t *video_data;
    uint32_t video_linesize;
    uint32_t width;
    uint32_t height;
    gs_texrender_t *texrender;
    gs_stagesurf_t *stagesurface;
    bool closing;
};
typedef struct apex_game_filter_context apex_game_filter_context_t;

struct area
{
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
};
typedef struct area area_t;

#define PGINFO_KEYBIND_BUTTON_X         90
#define PGINFO_KEYBIND_BUTTON_Y         1034
#define PGINFO_KEYBIND_BUTTON_W         20
#define PGINFO_KEYBIND_BUTTON_H         20

#define ESC_LOOTING_BUTTON_X            530
#define ESC_LOOTING_BUTTON_Y            971
#define ESC_LOOTING_BUTTON_W            37
#define ESC_LOOTING_BUTTON_H            15

#define ESC_INVENTORY_BUTTON_X          67
#define ESC_INVENTORY_BUTTON_Y          1042
#define ESC_INVENTORY_BUTTON_W          37
#define ESC_INVENTORY_BUTTON_H          15

#define M_MAP_BUTTON_X                  62
#define M_MAP_BUTTON_Y                  1036
#define M_MAP_BUTTON_W                  24
#define M_MAP_BUTTON_H                  24

#define PG_BANNER_IMAGE_X               114
#define PG_BANNER_IMAGE_Y               977
#define PG_BANNER_IMAGE_W               20
#define PG_BANNER_IMAGE_H               20

static const area_t areas[AREAS_NUM] =
{
    [PGINFO_KEYBIND_BUTTON] =   { PGINFO_KEYBIND_BUTTON_X,      PGINFO_KEYBIND_BUTTON_Y,    PGINFO_KEYBIND_BUTTON_W,    PGINFO_KEYBIND_BUTTON_H     },
    [ESC_LOOTING_BUTTON] =      { ESC_LOOTING_BUTTON_X,         ESC_LOOTING_BUTTON_Y,       ESC_LOOTING_BUTTON_W,       ESC_LOOTING_BUTTON_H        },
    [ESC_INVENTORY_BUTTON] =    { ESC_INVENTORY_BUTTON_X,       ESC_INVENTORY_BUTTON_Y,     ESC_INVENTORY_BUTTON_W,     ESC_INVENTORY_BUTTON_H      },
    [M_MAP_BUTTON] =            { M_MAP_BUTTON_X,               M_MAP_BUTTON_Y,             M_MAP_BUTTON_W,             M_MAP_BUTTON_H              },
    [PG_BANNER_IMAGE] =         { PG_BANNER_IMAGE_X,            PG_BANNER_IMAGE_Y,          PG_BANNER_IMAGE_W,          PG_BANNER_IMAGE_H           }
};

static void fill_area(PIX *image, uint32_t *raw_image, unsigned width, unsigned height, area_name_t an)
{
    if (pixGetHeight(image) != height || pixGetWidth(image) != width)
        pixSetResolution(image, width, height);

    const area_t *a = &areas[an];

    for (unsigned x = a->x; x < (a->x + a->w); x++) {
        for (unsigned y = a->y; y < (a->y + a->w); y++) {
            uint8_t *rgb = &raw_image[y * width + x];
            uint8_t r = rgb[0];
            uint8_t g = rgb[1];
            uint8_t b = rgb[2];
            pixSetRGBPixel(image, x, y, r, g, b);
        }
    }
}

static float compare_psnr_value_of_area(PIX *image, PIX *reference, area_name_t an)
{
    const struct area *a = &areas[an];

    BOX *box = boxCreate(a->x, a->y, a->w, a->h);
    PIX *rectangle = pixClipRectangle(image, box, NULL);

    float psnr;
    pixGetPSNR(rectangle, reference, 1, &psnr);

    boxDestroy(&box);
    pixDestroy(&rectangle);

    return psnr;
}

static const char *apex_game_filter_get_name(void *unused)
{
    return "Apex Game";
}

static bool check_banner(apex_game_filter_context_t *filter, area_name_t an, banner_position_t bp)
{
    obs_weak_source_t *source = filter->target_sources[an];
    obs_source_t *s = obs_weak_source_get_source(source);

    fill_area(filter->image, filter->video_data, filter->width, filter->height, an);
    float psnr = compare_psnr_value_of_area(filter->image, filter->banner_references[bp], an);

    bool enable_target_source = psnr > 60;
    obs_source_set_enabled(s, enable_target_source);

    obs_source_release(s);

    return enable_target_source;
}

static void apex_game_filter_offscreen_render(void *data, uint32_t cx, uint32_t cy)
{
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

    gs_texrender_reset(filter->texrender);

    if (!gs_texrender_begin(filter->texrender, filter->width, filter->height))
        return;

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

    check_banner(filter, PGINFO_KEYBIND_BUTTON, BANNER_GAME);
    check_banner(filter, ESC_LOOTING_BUTTON, BANNER_LOOTING);
    check_banner(filter, ESC_INVENTORY_BUTTON, BANNER_INVENTORY);
    check_banner(filter, M_MAP_BUTTON, BANNER_MAP);

    character_name_t pg;
    fill_area(filter->image, filter->video_data, filter->width, filter->height, PG_BANNER_IMAGE);
    for (pg = 0; pg < CHARACTERS_NUM; pg++) {
        float psnr = compare_psnr_value_of_area(filter->image, filter->pg_references[pg], PG_BANNER_IMAGE);
        if (psnr > 60)
            break;
    }
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

    update_source(settings, "game_source", &filter->target_sources[PGINFO_KEYBIND_BUTTON]);
    update_source(settings, "looting_source", &filter->target_sources[ESC_LOOTING_BUTTON]);
    update_source(settings, "inventory_source", &filter->target_sources[ESC_INVENTORY_BUTTON]);
    update_source(settings, "map_source", &filter->target_sources[M_MAP_BUTTON]);
}

static void apex_game_filter_defaults(obs_data_t *settings)
{
}

static void *apex_game_filter_create(obs_data_t *settings, obs_source_t *source)
{
    binfo("creating new filter");

    apex_game_filter_context_t *context = bzalloc(sizeof(apex_game_filter_context_t));

    context->source = source;
    context->texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

    context->image = pixCreate(1920, 1080, 32);

    context->banner_references[PGINFO_KEYBIND_BUTTON] = pixReadMemBmp(ref_game_bmp, ref_game_bmp_size);
    context->banner_references[ESC_LOOTING_BUTTON] = pixReadMemBmp(ref_looting_bmp, ref_looting_bmp_size);
    context->banner_references[ESC_INVENTORY_BUTTON] = pixReadMemBmp(ref_inventory_bmp, ref_inventory_bmp_size);
    context->banner_references[M_MAP_BUTTON] = pixReadMemBmp(ref_map_bmp, ref_map_bmp_size);

    context->pg_references[BLOODHOUND] = pixReadMemBmp(game_bloodhound_bmp, game_bloodhound_bmp_size);
    context->pg_references[GIBRALTAR] = pixReadMemBmp(game_gibraltar_bmp, game_gibraltar_bmp_size);
    context->pg_references[LIFELINE] = pixReadMemBmp(game_lifeline_bmp, game_lifeline_bmp_size);
    context->pg_references[PATHFINDER] = pixReadMemBmp(game_pathfinder_bmp, game_pathfinder_bmp_size);
    context->pg_references[WRAITH] = pixReadMemBmp(game_wraith_bmp, game_wraith_bmp_size);
    context->pg_references[BANGALORE] = pixReadMemBmp(game_bangalore_bmp, game_bangalore_bmp_size);
    context->pg_references[CAUSTIC] = pixReadMemBmp(game_caustic_bmp, game_caustic_bmp_size);
    context->pg_references[MIRAGE] = pixReadMemBmp(game_mirage_bmp, game_mirage_bmp_size);
    context->pg_references[OCTANE] = pixReadMemBmp(game_octane_bmp, game_octane_bmp_size);
    context->pg_references[WATTSON] = pixReadMemBmp(game_wattson_bmp, game_wattson_bmp_size);
    context->pg_references[CRYPTO] = pixReadMemBmp(game_crypto_bmp, game_crypto_bmp_size);
    context->pg_references[REVENANT] = pixReadMemBmp(game_revenant_bmp, game_revenant_bmp_size);
    context->pg_references[LOBA] = pixReadMemBmp(game_loba_bmp, game_loba_bmp_size);
    context->pg_references[RAMPART] = pixReadMemBmp(game_rampart_bmp, game_rampart_bmp_size);
    context->pg_references[HORIZON] = pixReadMemBmp(game_horizon_bmp, game_horizon_bmp_size);
    context->pg_references[FUSE] = pixReadMemBmp(game_fuse_bmp, game_fuse_bmp_size);
    context->pg_references[VALKYRIE] = pixReadMemBmp(game_valkyrie_bmp, game_valkyrie_bmp_size);
    context->pg_references[SEER] = pixReadMemBmp(game_seer_bmp, game_seer_bmp_size);

    apex_game_filter_update(context, settings);

    obs_add_main_render_callback(apex_game_filter_offscreen_render, context);

    return context;
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

    apex_game_filter_context_t *context = data;

    context->closing = true;

    pixDestroy(&context->image);

    pixDestroy(&context->banner_references[PGINFO_KEYBIND_BUTTON]);
    pixDestroy(&context->banner_references[ESC_LOOTING_BUTTON]);
    pixDestroy(&context->banner_references[ESC_INVENTORY_BUTTON]);
    pixDestroy(&context->banner_references[M_MAP_BUTTON]);

    obs_remove_main_render_callback(apex_game_filter_offscreen_render, context);

    release_source(context->target_sources[PGINFO_KEYBIND_BUTTON]);
    release_source(context->target_sources[ESC_LOOTING_BUTTON]);
    release_source(context->target_sources[ESC_INVENTORY_BUTTON]);
    release_source(context->target_sources[M_MAP_BUTTON]);

    obs_enter_graphics();

    gs_stagesurface_unmap(context->stagesurface);
    gs_stagesurface_destroy(context->stagesurface);
    gs_texrender_destroy(context->texrender);

    obs_leave_graphics();

    bfree(context);
}

static void apex_game_filter_tick(void *data, float seconds)
{
    apex_game_filter_context_t *context = data;

    if (context->closing)
        return;

    obs_source_t *parent = obs_filter_get_parent(context->source);

    if (!parent)
        return;

    uint32_t width = obs_source_get_width(parent);
    uint32_t height = obs_source_get_height(parent);

    width += (width & 1);
    height += (height & 1);

    if (context->width != width || context->height != height) {
        bdebug("new size, width: %d, height: %d", width, height);

        context->width = width;
        context->height = height;
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
    apex_game_filter_context_t *s = data;
    obs_property_t *p;

    obs_properties_t *props = obs_properties_create();

    p = obs_properties_add_list(props, "game_source", "Game Overlay Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    p = obs_properties_add_list(props, "inventory_source", "Game Inventory Overlay Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    p = obs_properties_add_list(props, "looting_source", "Game Looting Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    p = obs_properties_add_list(props, "map_source", "Game Map Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    return props;
}

static void apex_game_filter_render(void *data, gs_effect_t *effect)
{
    UNUSED_PARAMETER(effect);

    apex_game_filter_context_t *context = data;

    obs_source_skip_video_filter(context->source);
}

static void apex_game_filter_filter_remove(void *data, obs_source_t *parent)
{
    apex_game_filter_context_t *context = data;

    context->closing = true;

    obs_remove_main_render_callback(apex_game_filter_offscreen_render, context);
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
