#include <obs-module.h>
#include <obs-source.h>

#include <util/config-file.h>
#include <util/platform.h>
#include <util/threading.h>

#include <leptonica/allheaders.h>

#include "images.h"

#define PROJECT_VERSION "1.5.0"

#define DEBUG_FRAME_INTERVAL        300
#define DEBUG_SAVE_PATH             "C:\\Temp"
#define DEBUG_SAVE_PATH_NAME_LEN    128

#define PSNR_THRESHOLD_VALUE        16.5f

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
    BALLISTIC,

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
    "ballistic",
};

enum banner_position
{
    BANNER_GAME,
    BANNER_LOOTING,
    BANNER_INVENTORY,
    BANNER_MAP,
    BANNER_SPECTATE,

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
    SPECTATE_IMAGE_RED,
    SPECTATE_IMAGE_GREEN,
    SPECTATE_IMAGE_ORANGE,
    SPECTATE_IMAGE_BLUE,

    AREAS_NUM
};
typedef enum area_name area_name_t;

const char *area_name_str[AREAS_NUM] =
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
    "PAD_TACTICAL_BUTTON",
    "SPECTATE_IMAGE_RED",
    "SPECTATE_IMAGE_GREEN",
    "SPECTATE_IMAGE_ORANGE",
    "SPECTATE_IMAGE_BLUE",
};

enum input_device
{
    MOUSE_AND_KEYBOARD,
    PLAY_STATION_PAD,

    INPUT_DEVICES_NUM
};

enum display_resolution
{
    DISPLAY_1080P,
    DISPLAY_2K,

    DISPLAY_RESOLUTIONS
};

enum game_language
{
    LANGUAGE_IT,
    LANGUAGE_EN,
    LANGUAGE_ZH,

    LANGUAGES
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
    PIX *banner_references[DISPLAY_RESOLUTIONS][AREAS_NUM];
    PIX *pg_references[DISPLAY_RESOLUTIONS][CHARACTERS_NUM];
    obs_source_t *source;
    obs_weak_source_t *target_sources[BANNER_POSITION_NUM];
    uint8_t *video_data;
    uint32_t video_linesize;
    uint32_t width;
    uint32_t height;
    enum display_resolution display;
    enum input_device input;
    enum game_language language;
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
#define ESC_LOOTING_BUTTON_X_ZH         545
#define ESC_LOOTING_BUTTON_Y            965
#define ESC_LOOTING_BUTTON_W            43
#define ESC_LOOTING_BUTTON_H            26

#define ESC_INVENTORY_BUTTON_X_IT       81
#define ESC_INVENTORY_BUTTON_X_EN       62
#define ESC_INVENTORY_BUTTON_X_ZH       56
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

#define PAD_LOOTING_BUTTON_X_IT         533
#define PAD_LOOTING_BUTTON_X_EN         544
#define PAD_LOOTING_BUTTON_X_ZH         563
#define PAD_LOOTING_BUTTON_Y            968
#define PAD_LOOTING_BUTTON_W            24
#define PAD_LOOTING_BUTTON_H            20

#define PAD_INVENTORY_BUTTON_X_IT       92
#define PAD_INVENTORY_BUTTON_X_EN       73
#define PAD_INVENTORY_BUTTON_X_ZH       67
#define PAD_INVENTORY_BUTTON_Y          1039
#define PAD_INVENTORY_BUTTON_W          24
#define PAD_INVENTORY_BUTTON_H          20

#define PAD_TACTICAL_BUTTON_X           604
#define PAD_TACTICAL_BUTTON_Y           1040
#define PAD_TACTICAL_BUTTON_W           22
#define PAD_TACTICAL_BUTTON_H           12

#define SPECTATE_IMAGE_X                1130
#define SPECTATE_IMAGE_Y                1000
#define SPECTATE_IMAGE_W                16
#define SPECTATE_IMAGE_H                22

#define MAP_GAME_BUTTON_2K_X            69
#define MAP_GAME_BUTTON_2K_Y            35
#define MAP_GAME_BUTTON_2K_W            27
#define MAP_GAME_BUTTON_2K_H            26

#define GRENADE_GAME_BUTTON_2K_X        1889
#define GRENADE_GAME_BUTTON_2K_Y        1380
#define GRENADE_GAME_BUTTON_2K_W        27
#define GRENADE_GAME_BUTTON_2K_H        27

#define ESC_LOOTING_BUTTON_2K_X_IT      686
#define ESC_LOOTING_BUTTON_2K_X_EN      701
#define ESC_LOOTING_BUTTON_2K_X_ZH      736
#define ESC_LOOTING_BUTTON_2K_Y         1286
#define ESC_LOOTING_BUTTON_2K_W         59
#define ESC_LOOTING_BUTTON_2K_H         36

#define ESC_INVENTORY_BUTTON_2K_X_IT    109
#define ESC_INVENTORY_BUTTON_2K_X_EN    83
#define ESC_INVENTORY_BUTTON_2K_X_ZH    76
#define ESC_INVENTORY_BUTTON_2K_Y       1379
#define ESC_INVENTORY_BUTTON_2K_W       62
#define ESC_INVENTORY_BUTTON_2K_H       38

#define GRAYBAR_INVENTORY_BUTTON_2K_X   219
#define GRAYBAR_INVENTORY_BUTTON_2K_Y   1117
#define GRAYBAR_INVENTORY_BUTTON_2K_W   300
#define GRAYBAR_INVENTORY_BUTTON_2K_H   1

#define M_MAP_BUTTON_2K_X               83
#define M_MAP_BUTTON_2K_Y               1382
#define M_MAP_BUTTON_2K_W               31
#define M_MAP_BUTTON_2K_H               31

#define PG_BANNER_IMAGE_2K_X            146
#define PG_BANNER_IMAGE_2K_Y            1300
#define PG_BANNER_IMAGE_2K_W            30
#define PG_BANNER_IMAGE_2K_H            40

#define PAD_MAP_BUTTON_2K_X             73
#define PAD_MAP_BUTTON_2K_Y             1370
#define PAD_MAP_BUTTON_2K_W             68
#define PAD_MAP_BUTTON_2K_H             55

#define PAD_LOOTING_BUTTON_2K_X_IT      711
#define PAD_LOOTING_BUTTON_2K_X_EN      726
#define PAD_LOOTING_BUTTON_2K_X_ZH      740
#define PAD_LOOTING_BUTTON_2K_Y         1291
#define PAD_LOOTING_BUTTON_2K_W         29
#define PAD_LOOTING_BUTTON_2K_H         26

#define PAD_INVENTORY_BUTTON_2K_X_IT    126
#define PAD_INVENTORY_BUTTON_2K_X_EN    100
#define PAD_INVENTORY_BUTTON_2K_X_ZH    93
#define PAD_INVENTORY_BUTTON_2K_Y       1385
#define PAD_INVENTORY_BUTTON_2K_W       26
#define PAD_INVENTORY_BUTTON_2K_H       26

#define PAD_TACTICAL_BUTTON_2K_X        807
#define PAD_TACTICAL_BUTTON_2K_Y        1385
#define PAD_TACTICAL_BUTTON_2K_W        26
#define PAD_TACTICAL_BUTTON_2K_H        18

#define SPECTATE_IMAGE_2K_X             1508
#define SPECTATE_IMAGE_2K_Y             1336
#define SPECTATE_IMAGE_2K_W             22
#define SPECTATE_IMAGE_2K_H             22

static const area_t areas_1080p_en[AREAS_NUM] =
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
    [PAD_TACTICAL_BUTTON] =     { PAD_TACTICAL_BUTTON_X,        PAD_TACTICAL_BUTTON_Y,      PAD_TACTICAL_BUTTON_W,      PAD_TACTICAL_BUTTON_H       },
    [SPECTATE_IMAGE_RED] =      { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },
    [SPECTATE_IMAGE_GREEN] =    { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },
    [SPECTATE_IMAGE_ORANGE] =   { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },
    [SPECTATE_IMAGE_BLUE] =     { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },
};

static const area_t areas_1080p_it[AREAS_NUM] =
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
    [PAD_TACTICAL_BUTTON] =     { PAD_TACTICAL_BUTTON_X,        PAD_TACTICAL_BUTTON_Y,      PAD_TACTICAL_BUTTON_W,      PAD_TACTICAL_BUTTON_H       },
    [SPECTATE_IMAGE_RED] =      { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },
    [SPECTATE_IMAGE_GREEN] =    { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },
    [SPECTATE_IMAGE_ORANGE] =   { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },
    [SPECTATE_IMAGE_BLUE] =     { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },
};

static const area_t areas_1080p_zh[AREAS_NUM] =
{
    [MAP_GAME_BUTTON] =         { MAP_GAME_BUTTON_X,            MAP_GAME_BUTTON_Y,          MAP_GAME_BUTTON_W,          MAP_GAME_BUTTON_H           },
    [GRENADE_GAME_BUTTON] =     { GRENADE_GAME_BUTTON_X,        GRENADE_GAME_BUTTON_Y,      GRENADE_GAME_BUTTON_W,      GRENADE_GAME_BUTTON_H       },
    [ESC_LOOTING_BUTTON] =      { ESC_LOOTING_BUTTON_X_ZH,      ESC_LOOTING_BUTTON_Y,       ESC_LOOTING_BUTTON_W,       ESC_LOOTING_BUTTON_H        },
    [ESC_INVENTORY_BUTTON] =    { ESC_INVENTORY_BUTTON_X_ZH,    ESC_INVENTORY_BUTTON_Y,     ESC_INVENTORY_BUTTON_W,     ESC_INVENTORY_BUTTON_H      },
    [GRAYBAR_INVENTORY_BUTTON] ={ GRAYBAR_INVENTORY_BUTTON_X,   GRAYBAR_INVENTORY_BUTTON_Y, GRAYBAR_INVENTORY_BUTTON_W, GRAYBAR_INVENTORY_BUTTON_H  },
    [M_MAP_BUTTON] =            { M_MAP_BUTTON_X,               M_MAP_BUTTON_Y,             M_MAP_BUTTON_W,             M_MAP_BUTTON_H              },
    [PG_BANNER_IMAGE] =         { PG_BANNER_IMAGE_X,            PG_BANNER_IMAGE_Y,          PG_BANNER_IMAGE_W,          PG_BANNER_IMAGE_H           },
    [PAD_MAP_BUTTON] =          { PAD_MAP_BUTTON_X,             PAD_MAP_BUTTON_Y,           PAD_MAP_BUTTON_W,           PAD_MAP_BUTTON_H            },
    [PAD_LOOTING_BUTTON] =      { PAD_LOOTING_BUTTON_X_ZH,      PAD_LOOTING_BUTTON_Y,       PAD_LOOTING_BUTTON_W,       PAD_LOOTING_BUTTON_H        },
    [PAD_INVENTORY_BUTTON] =    { PAD_INVENTORY_BUTTON_X_ZH,    PAD_INVENTORY_BUTTON_Y,     PAD_INVENTORY_BUTTON_W,     PAD_INVENTORY_BUTTON_H      },
    [PAD_TACTICAL_BUTTON] =     { PAD_TACTICAL_BUTTON_X,        PAD_TACTICAL_BUTTON_Y,      PAD_TACTICAL_BUTTON_W,      PAD_TACTICAL_BUTTON_H       },
    [SPECTATE_IMAGE_RED] =      { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },
    [SPECTATE_IMAGE_GREEN] =    { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },
    [SPECTATE_IMAGE_ORANGE] =   { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },
    [SPECTATE_IMAGE_BLUE] =     { SPECTATE_IMAGE_X,             SPECTATE_IMAGE_Y,           SPECTATE_IMAGE_W,           SPECTATE_IMAGE_H            },

};

static const area_t areas_2k_en[AREAS_NUM] =
{
    [MAP_GAME_BUTTON] =         { MAP_GAME_BUTTON_2K_X,             MAP_GAME_BUTTON_2K_Y,           MAP_GAME_BUTTON_2K_W,           MAP_GAME_BUTTON_2K_H            },
    [GRENADE_GAME_BUTTON] =     { GRENADE_GAME_BUTTON_2K_X,         GRENADE_GAME_BUTTON_2K_Y,       GRENADE_GAME_BUTTON_2K_W,       GRENADE_GAME_BUTTON_2K_H        },
    [ESC_LOOTING_BUTTON] =      { ESC_LOOTING_BUTTON_2K_X_EN,       ESC_LOOTING_BUTTON_2K_Y,        ESC_LOOTING_BUTTON_2K_W,        ESC_LOOTING_BUTTON_2K_H         },
    [ESC_INVENTORY_BUTTON] =    { ESC_INVENTORY_BUTTON_2K_X_EN,     ESC_INVENTORY_BUTTON_2K_Y,      ESC_INVENTORY_BUTTON_2K_W,      ESC_INVENTORY_BUTTON_2K_H       },
    [GRAYBAR_INVENTORY_BUTTON] ={ GRAYBAR_INVENTORY_BUTTON_2K_X,    GRAYBAR_INVENTORY_BUTTON_2K_Y,  GRAYBAR_INVENTORY_BUTTON_2K_W,  GRAYBAR_INVENTORY_BUTTON_2K_H   },
    [M_MAP_BUTTON] =            { M_MAP_BUTTON_2K_X,                M_MAP_BUTTON_2K_Y,              M_MAP_BUTTON_2K_W,              M_MAP_BUTTON_2K_H               },
    [PG_BANNER_IMAGE] =         { PG_BANNER_IMAGE_2K_X,             PG_BANNER_IMAGE_2K_Y,           PG_BANNER_IMAGE_2K_W,           PG_BANNER_IMAGE_2K_H            },
    [PAD_MAP_BUTTON] =          { PAD_MAP_BUTTON_2K_X,              PAD_MAP_BUTTON_2K_Y,            PAD_MAP_BUTTON_2K_W,            PAD_MAP_BUTTON_2K_H             },
    [PAD_LOOTING_BUTTON] =      { PAD_LOOTING_BUTTON_2K_X_EN,       PAD_LOOTING_BUTTON_2K_Y,        PAD_LOOTING_BUTTON_2K_W,        PAD_LOOTING_BUTTON_2K_H         },
    [PAD_INVENTORY_BUTTON] =    { PAD_INVENTORY_BUTTON_2K_X_EN,     PAD_INVENTORY_BUTTON_2K_Y,      PAD_INVENTORY_BUTTON_2K_W,      PAD_INVENTORY_BUTTON_2K_H       },
    [PAD_TACTICAL_BUTTON] =     { PAD_TACTICAL_BUTTON_2K_X,         PAD_TACTICAL_BUTTON_2K_Y,       PAD_TACTICAL_BUTTON_2K_W,       PAD_TACTICAL_BUTTON_2K_H        },
    [SPECTATE_IMAGE_RED] =      { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
    [SPECTATE_IMAGE_GREEN] =    { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
    [SPECTATE_IMAGE_ORANGE] =   { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
    [SPECTATE_IMAGE_BLUE] =     { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
};

static const area_t areas_2k_it[AREAS_NUM] =
{
    [MAP_GAME_BUTTON] =         { MAP_GAME_BUTTON_2K_X,             MAP_GAME_BUTTON_2K_Y,           MAP_GAME_BUTTON_2K_W,           MAP_GAME_BUTTON_2K_H            },
    [GRENADE_GAME_BUTTON] =     { GRENADE_GAME_BUTTON_2K_X,         GRENADE_GAME_BUTTON_2K_Y,       GRENADE_GAME_BUTTON_2K_W,       GRENADE_GAME_BUTTON_2K_H        },
    [ESC_LOOTING_BUTTON] =      { ESC_LOOTING_BUTTON_2K_X_IT,       ESC_LOOTING_BUTTON_2K_Y,        ESC_LOOTING_BUTTON_2K_W,        ESC_LOOTING_BUTTON_2K_H         },
    [ESC_INVENTORY_BUTTON] =    { ESC_INVENTORY_BUTTON_2K_X_IT,     ESC_INVENTORY_BUTTON_2K_Y,      ESC_INVENTORY_BUTTON_2K_W,      ESC_INVENTORY_BUTTON_2K_H       },
    [GRAYBAR_INVENTORY_BUTTON] ={ GRAYBAR_INVENTORY_BUTTON_2K_X,    GRAYBAR_INVENTORY_BUTTON_2K_Y,  GRAYBAR_INVENTORY_BUTTON_2K_W,  GRAYBAR_INVENTORY_BUTTON_2K_H   },
    [M_MAP_BUTTON] =            { M_MAP_BUTTON_2K_X,                M_MAP_BUTTON_2K_Y,              M_MAP_BUTTON_2K_W,              M_MAP_BUTTON_2K_H               },
    [PG_BANNER_IMAGE] =         { PG_BANNER_IMAGE_2K_X,             PG_BANNER_IMAGE_2K_Y,           PG_BANNER_IMAGE_2K_W,           PG_BANNER_IMAGE_2K_H            },
    [PAD_MAP_BUTTON] =          { PAD_MAP_BUTTON_2K_X,              PAD_MAP_BUTTON_2K_Y,            PAD_MAP_BUTTON_2K_W,            PAD_MAP_BUTTON_2K_H             },
    [PAD_LOOTING_BUTTON] =      { PAD_LOOTING_BUTTON_2K_X_IT,       PAD_LOOTING_BUTTON_2K_Y,        PAD_LOOTING_BUTTON_2K_W,        PAD_LOOTING_BUTTON_2K_H         },
    [PAD_INVENTORY_BUTTON] =    { PAD_INVENTORY_BUTTON_2K_X_IT,     PAD_INVENTORY_BUTTON_2K_Y,      PAD_INVENTORY_BUTTON_2K_W,      PAD_INVENTORY_BUTTON_2K_H       },
    [PAD_TACTICAL_BUTTON] =     { PAD_TACTICAL_BUTTON_2K_X,         PAD_TACTICAL_BUTTON_2K_Y,       PAD_TACTICAL_BUTTON_2K_W,       PAD_TACTICAL_BUTTON_2K_H        },
    [SPECTATE_IMAGE_RED] =      { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
    [SPECTATE_IMAGE_GREEN] =    { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
    [SPECTATE_IMAGE_ORANGE] =   { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
    [SPECTATE_IMAGE_BLUE] =     { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
};

static const area_t areas_2k_zh[AREAS_NUM] =
{
    [MAP_GAME_BUTTON] =         { MAP_GAME_BUTTON_2K_X,             MAP_GAME_BUTTON_2K_Y,           MAP_GAME_BUTTON_2K_W,           MAP_GAME_BUTTON_2K_H            },
    [GRENADE_GAME_BUTTON] =     { GRENADE_GAME_BUTTON_2K_X,         GRENADE_GAME_BUTTON_2K_Y,       GRENADE_GAME_BUTTON_2K_W,       GRENADE_GAME_BUTTON_2K_H        },
    [ESC_LOOTING_BUTTON] =      { ESC_LOOTING_BUTTON_2K_X_ZH,       ESC_LOOTING_BUTTON_2K_Y,        ESC_LOOTING_BUTTON_2K_W,        ESC_LOOTING_BUTTON_2K_H         },
    [ESC_INVENTORY_BUTTON] =    { ESC_INVENTORY_BUTTON_2K_X_ZH,     ESC_INVENTORY_BUTTON_2K_Y,      ESC_INVENTORY_BUTTON_2K_W,      ESC_INVENTORY_BUTTON_2K_H       },
    [GRAYBAR_INVENTORY_BUTTON] ={ GRAYBAR_INVENTORY_BUTTON_2K_X,    GRAYBAR_INVENTORY_BUTTON_2K_Y,  GRAYBAR_INVENTORY_BUTTON_2K_W,  GRAYBAR_INVENTORY_BUTTON_2K_H   },
    [M_MAP_BUTTON] =            { M_MAP_BUTTON_2K_X,                M_MAP_BUTTON_2K_Y,              M_MAP_BUTTON_2K_W,              M_MAP_BUTTON_2K_H               },
    [PG_BANNER_IMAGE] =         { PG_BANNER_IMAGE_2K_X,             PG_BANNER_IMAGE_2K_Y,           PG_BANNER_IMAGE_2K_W,           PG_BANNER_IMAGE_2K_H            },
    [PAD_MAP_BUTTON] =          { PAD_MAP_BUTTON_2K_X,              PAD_MAP_BUTTON_2K_Y,            PAD_MAP_BUTTON_2K_W,            PAD_MAP_BUTTON_2K_H             },
    [PAD_LOOTING_BUTTON] =      { PAD_LOOTING_BUTTON_2K_X_ZH,       PAD_LOOTING_BUTTON_2K_Y,        PAD_LOOTING_BUTTON_2K_W,        PAD_LOOTING_BUTTON_2K_H         },
    [PAD_INVENTORY_BUTTON] =    { PAD_INVENTORY_BUTTON_2K_X_ZH,     PAD_INVENTORY_BUTTON_2K_Y,      PAD_INVENTORY_BUTTON_2K_W,      PAD_INVENTORY_BUTTON_2K_H       },
    [PAD_TACTICAL_BUTTON] =     { PAD_TACTICAL_BUTTON_2K_X,         PAD_TACTICAL_BUTTON_2K_Y,       PAD_TACTICAL_BUTTON_2K_W,       PAD_TACTICAL_BUTTON_2K_H        },
    [SPECTATE_IMAGE_RED] =      { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
    [SPECTATE_IMAGE_GREEN] =    { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
    [SPECTATE_IMAGE_ORANGE] =   { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
    [SPECTATE_IMAGE_BLUE] =     { SPECTATE_IMAGE_2K_X,              SPECTATE_IMAGE_2K_Y,            SPECTATE_IMAGE_2K_W,            SPECTATE_IMAGE_2K_H             },
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

    pixWrite(filename, filter->banner_references[filter->display][an], IFF_PNG);
}

static void save_image_area(apex_game_filter_context_t *filter, const area_t *a, const char *n)
{
    char filename[DEBUG_SAVE_PATH_NAME_LEN];

    BOX *box = boxCreate(a->x, a->y, a->w, a->h);
    PIX *rectangle = pixClipRectangle(filter->image, box, NULL);

    snprintf(filename, DEBUG_SAVE_PATH_NAME_LEN, "%s\\image_%s.png", DEBUG_SAVE_PATH, n);

    pixWrite(filename, rectangle, IFF_PNG);

    boxDestroy(&box);
    pixDestroy(&rectangle);
}

static void save_image(apex_game_filter_context_t *filter, area_name_t an)
{
    const area_t *a = &(filter->areas[an]);
    const char *n = area_name_str[an];

    save_image_area(filter, a, n);
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
    float psnr = compare_psnr_value_of_area_with_offset(filter->image, filter->banner_references[filter->display][an], a, xoff);

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
        float psnr = compare_psnr_value_of_area(filter->image, filter->pg_references[filter->display][pg], &(filter->areas[PG_BANNER_IMAGE]));

        if (debug_should_print(filter))
            binfo("%s: %f", character_name_str[pg], psnr);

        if (psnr > PSNR_THRESHOLD_VALUE)
            break;
    }

    return pg;
}

#define BOX_START_X         290
#define BOX_START_Y         790
#define BOX_WIDTH           260
#define BOX_HEIGHT          185

#define BOX_START_2K_X      370
#define BOX_START_2K_Y      1030
#define BOX_WIDTH_2K        375
#define BOX_HEIGHT_2K       275

#define MIN_LINE_LENGTH     75
#define MIN_LINE_LENGTH_2K  100

#define GRAY_LINE_BANNER_DEFAULT_Y          926
#define GRAY_LINE_BANNER_DEFAULT_X_END      450
#define GRAY_LINE_BANNER_DEFAULT_DIFF       87

#define GRAY_LINE_BANNER_2K_DEFAULT_Y       1234
#define GRAY_LINE_BANNER_2K_DEFAULT_X_END   617
#define GRAY_LINE_BANNER_2K_DEFAULT_DIFF    116

#define GRAY_POINT          150
#define GRAY_MAX_DIFF       15
#define GRAY_MIN            (GRAY_POINT - GRAY_MAX_DIFF)
#define GRAY_MAX            (GRAY_POINT + GRAY_MAX_DIFF)
#define GRAY_COMP_MAX_DIFF  8

struct gray_line_searcher_ref
{
    uint32_t box_start_x;
    uint32_t box_start_y;
    uint32_t box_witdh;
    uint32_t box_height;
    uint32_t min_line_length;
    uint32_t default_grayline_y;
    uint32_t default_grayline_x_end;
    uint32_t default_grayline_diff;
};

const struct gray_line_searcher_ref line_searches[DISPLAY_RESOLUTIONS] =
{
    [DISPLAY_1080P] =   { BOX_START_X,      BOX_START_Y,    BOX_WIDTH,      BOX_HEIGHT,     MIN_LINE_LENGTH,    GRAY_LINE_BANNER_DEFAULT_Y,     GRAY_LINE_BANNER_DEFAULT_X_END,     GRAY_LINE_BANNER_DEFAULT_DIFF       },
    [DISPLAY_2K] =      { BOX_START_2K_X,   BOX_START_2K_Y, BOX_WIDTH_2K,   BOX_HEIGHT_2K,  MIN_LINE_LENGTH_2K, GRAY_LINE_BANNER_2K_DEFAULT_Y,  GRAY_LINE_BANNER_2K_DEFAULT_X_END,  GRAY_LINE_BANNER_2K_DEFAULT_DIFF    },
};

static bool check_rgb(int r, int g, int b)
{
    if (abs(r - g) > GRAY_COMP_MAX_DIFF ||
        abs(g - b) > GRAY_COMP_MAX_DIFF ||
        abs(r - b) > GRAY_COMP_MAX_DIFF)
        return false;

    if (r < GRAY_MIN || r > GRAY_MAX)
        return false;

    if (g < GRAY_MIN || g > GRAY_MAX)
        return false;

    if (b < GRAY_MIN || b > GRAY_MAX)
        return false;

    return true;
}

struct gray_line
{
    int pixel_count;
    int end_x;
    int y;
    bool found;
};

static bool find_banner_gray_lines(apex_game_filter_context_t *filter, struct gray_line lines[2])
{
    uint32_t x, y, check_x, r, g, b, count, line;
    const struct gray_line_searcher_ref *ls = &line_searches[filter->display];

    area_t a =
    {
        .x = ls->box_start_x,
        .y = ls->box_start_y,
        .w = ls->box_witdh,
        .h = ls->box_height
    };

    lines[0].found = false;
    lines[1].found = false;

    fill_area(filter->image, filter->video_data, filter->width, filter->height, &a, 0);

    x = ls->box_start_x;
    y = ls->box_start_y + ls->box_height;

    line = 0;

    while (y > ls->box_start_y) {
        pixGetRGBPixel(filter->image, x, y, &r, &g, &b);

        if (check_rgb(r, g, b)) {
            count = 0;
            for (check_x = ls->box_start_x; check_x < (ls->box_start_x + ls->box_witdh); check_x++) {
                pixGetRGBPixel(filter->image, check_x, y, &r, &g, &b);

                if (!check_rgb(r, g, b))
                    break;

                count++;
            }

            if (count > ls->min_line_length) {
                if (debug_should_print(filter))
                    binfo("found line y:%d, length: %d", y, count);

                bool line_first = line == 0;
                bool line_dist_ok = !line_first && (lines[line-1].y - y) > 5;

                if (line_first || line_dist_ok) {
                    lines[line].pixel_count = count;
                    lines[line].end_x = check_x;
                    lines[line].y = y;
                    lines[line].found = true;

                    line++;
                }
            }

            if (line == 2)
                break;
        }

        y--;
    }

    if (debug_should_print(filter)) {
        binfo("line 1: %d %d %d %d", lines[0].found, lines[0].pixel_count, lines[0].end_x, lines[0].y);
        binfo("line 2: %d %d %d %d", lines[1].found, lines[1].pixel_count, lines[1].end_x, lines[1].y);
        binfo("line distance: %d", lines[0].y - lines[1].y);

        save_image_area(filter, &a, "GRAYBAR_BAR");
    }

    if (line == 2) {
        int line_diff = lines[0].y - lines[1].y;
        if (line_diff == ls->default_grayline_diff)
            return true;
    }

    return false;
}

int match_offsets[DISPLAY_RESOLUTIONS][AREAS_NUM] =
{
    [DISPLAY_1080P] =
    {
        [M_MAP_BUTTON] =            -2,
        [PAD_MAP_BUTTON] =          -2,
        [PAD_LOOTING_BUTTON] =      -8,
        [PAD_INVENTORY_BUTTON] =    -8,
        [ESC_INVENTORY_BUTTON] =    -8,
        [ESC_LOOTING_BUTTON] =      -8,
    },
    [DISPLAY_2K] =
    {
        [M_MAP_BUTTON] =            -3,
        [PAD_MAP_BUTTON] =          -3,
        [PAD_LOOTING_BUTTON] =      -11,
        [PAD_INVENTORY_BUTTON] =    -11,
        [ESC_INVENTORY_BUTTON] =    -11,
        [ESC_LOOTING_BUTTON] =      -11,
    },
};

static void match_mk(apex_game_filter_context_t *filter)
{
    /*
     * if the area of interest matches the reference image we are
     * 100% sure that we can move to that scene
     */
    bool esc_looting_button = get_area_status(filter, ESC_LOOTING_BUTTON);
    bool esc_looting_button_offset = get_area_status_withoffset(filter, ESC_LOOTING_BUTTON, match_offsets[filter->display][ESC_LOOTING_BUTTON]);
    bool activate_looting = esc_looting_button || esc_looting_button_offset;

    set_source_status(filter->target_sources[BANNER_LOOTING], activate_looting);

    /*
     * checking map in control game mode moves the M button a little bit
     * with respect to all other game modes
     */
    bool map_button = get_area_status(filter, M_MAP_BUTTON);
    bool map_button_offset = get_area_status_withoffset(filter, M_MAP_BUTTON, match_offsets[filter->display][M_MAP_BUTTON]);
    bool activate_map = map_button || map_button_offset;

    set_source_status(filter->target_sources[BANNER_MAP], activate_map);

    /*
     * if inventory ESC button is found a further check must performed if inventory tab
     * is selected, otherwise in the other tabs player banner is not showed
     */
    bool esc_inventory_button = get_area_status(filter, ESC_INVENTORY_BUTTON);
    bool esc_inventory_button_offset = get_area_status_withoffset(filter, ESC_INVENTORY_BUTTON, match_offsets[filter->display][ESC_INVENTORY_BUTTON]);
    bool activate_inventory = esc_inventory_button || esc_inventory_button_offset;
    if (activate_inventory) {
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

    /*
     * spectate matching is done by checking a portion of the bottom center frame
     * with the name of the spectating person.
     * there are 4 different colors depending on who is spectating: red: enemy,
     * orange/blue/green: team mate.
     */
    bool spectate_red = get_area_status(filter, SPECTATE_IMAGE_RED);
    bool spectate_green = get_area_status(filter, SPECTATE_IMAGE_GREEN);
    bool spectate_orange = get_area_status(filter, SPECTATE_IMAGE_ORANGE);
    bool spectate_blue = get_area_status(filter, SPECTATE_IMAGE_BLUE);
    bool activate_specatate = spectate_red || spectate_green || spectate_orange || spectate_blue;

    set_source_status(filter->target_sources[BANNER_SPECTATE], activate_specatate);
}

static void match_ps4pad(apex_game_filter_context_t *filter)
{
    /*
     * checking map in control game mode moves the M button a little bit
     * with respect to all other game modes
     */
    bool pad_map = get_area_status(filter, PAD_MAP_BUTTON);
    bool pad_map_offset = get_area_status_withoffset(filter, PAD_MAP_BUTTON, match_offsets[filter->display][PAD_MAP_BUTTON]);
    bool activate_map = pad_map || pad_map_offset;

    set_source_status(filter->target_sources[BANNER_MAP], activate_map);

    /*
     * there's a funny behaviour if you use m&k and pad at the same time,
     * the absolute position of the button moves by 8 pixels wheter or not
     * you move mouse, handle this case by trying to recognize the reference
     * image twice considering that offset
     */
    bool pad_looting = get_area_status(filter, PAD_LOOTING_BUTTON);
    bool pad_looting_offset = get_area_status_withoffset(filter, PAD_LOOTING_BUTTON, match_offsets[filter->display][PAD_LOOTING_BUTTON]);
    bool activate_looting = pad_looting || pad_looting_offset;

    set_source_status(filter->target_sources[BANNER_LOOTING], activate_looting);

    /*
     * inventory for pad is difficult, the absolute position of the pg HUD moves when
     * analog joystick is moved making recognition of this HUD not perfect
     * as for now recognize only the inventory button, a much complex analysis is
     * necessary to move the source in the correct position
     */
    bool pad_inventory = get_area_status(filter, PAD_INVENTORY_BUTTON);
    bool pad_inventory_offset = get_area_status_withoffset(filter, PAD_INVENTORY_BUTTON, match_offsets[filter->display][PAD_INVENTORY_BUTTON]);
    bool activate_inventory = pad_inventory || pad_inventory_offset;

    if (activate_inventory) {
        struct gray_line lines[2];
        bool gray_line_found = find_banner_gray_lines(filter, lines);

        activate_inventory =  gray_line_found;
    }

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

    /*
     * spectate matching is done by checking a portion of the bottom center frame
     * with the name of the spectating person.
     * there are 4 different colors depending on who is spectating: red: enemy,
     * orange/blue/green: team mate.
     */
    bool spectate_red = get_area_status(filter, SPECTATE_IMAGE_RED);
    bool spectate_green = get_area_status(filter, SPECTATE_IMAGE_GREEN);
    bool spectate_orange = get_area_status(filter, SPECTATE_IMAGE_ORANGE);
    bool spectate_blue = get_area_status(filter, SPECTATE_IMAGE_BLUE);
    bool activate_specatate = spectate_red || spectate_green || spectate_orange || spectate_blue;

    set_source_status(filter->target_sources[BANNER_SPECTATE], activate_specatate);
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

    if (filter->display == DISPLAY_RESOLUTIONS)
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

    if (filter->display == DISPLAY_1080P) {
        if (filter->language == LANGUAGE_EN)
            filter->areas = areas_1080p_en;
        else if (filter->language == LANGUAGE_IT)
            filter->areas = areas_1080p_it;
        else if (filter->language == LANGUAGE_ZH)
            filter->areas = areas_1080p_zh;
    } else if (filter->display == DISPLAY_2K) {
        if (filter->language == LANGUAGE_EN)
            filter->areas = areas_2k_en;
        else if (filter->language == LANGUAGE_IT)
            filter->areas = areas_2k_it;
        else if (filter->language == LANGUAGE_ZH)
            filter->areas = areas_2k_zh;
    }

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
    update_source(settings, "spectate_source", &filter->target_sources[BANNER_SPECTATE]);

    filter->debug_mode = obs_data_get_bool(settings, "debug_mode");

    const char *game_lang = obs_data_get_string(settings, "game_lang");

    if (strcmp(game_lang, "it") == 0)
        filter->language = LANGUAGE_IT;
    else if (strcmp(game_lang, "en") == 0)
        filter->language = LANGUAGE_EN;
    else if (strcmp(game_lang, "zh") == 0)
        filter->language = LANGUAGE_ZH;
    else
        filter->language = LANGUAGE_EN;

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

static void load_1080p_references(apex_game_filter_context_t *filter)
{
    filter->banner_references[DISPLAY_1080P][MAP_GAME_BUTTON] = pixReadMemBmp(ref_game_map_bmp, ref_game_map_bmp_size);
    filter->banner_references[DISPLAY_1080P][GRENADE_GAME_BUTTON] = pixReadMemBmp(ref_game_grenade_bmp, ref_game_grenade_bmp_size);
    filter->banner_references[DISPLAY_1080P][ESC_LOOTING_BUTTON] = pixReadMemBmp(ref_looting_bmp, ref_looting_bmp_size);
    filter->banner_references[DISPLAY_1080P][ESC_INVENTORY_BUTTON] = pixReadMemBmp(ref_inventory_bmp, ref_inventory_bmp_size);
    filter->banner_references[DISPLAY_1080P][GRAYBAR_INVENTORY_BUTTON] = pixReadMemBmp(ref_graybar_inventory_bmp, ref_graybar_inventory_bmp_size);
    filter->banner_references[DISPLAY_1080P][M_MAP_BUTTON] = pixReadMemBmp(ref_map_bmp, ref_map_bmp_size);
    filter->banner_references[DISPLAY_1080P][PAD_MAP_BUTTON] = pixReadMemBmp(ref_pad_map_bmp, ref_pad_map_bmp_size);
    filter->banner_references[DISPLAY_1080P][PAD_LOOTING_BUTTON] = pixReadMemBmp(ref_pad_looting_bmp, ref_pad_looting_bmp_size);
    filter->banner_references[DISPLAY_1080P][PAD_INVENTORY_BUTTON] = pixReadMemBmp(ref_pad_inventory_bmp, ref_pad_inventory_bmp_size);
    filter->banner_references[DISPLAY_1080P][PAD_TACTICAL_BUTTON] = pixReadMemBmp(ref_pad_tactical_bmp, ref_pad_tactical_bmp_size);
    filter->banner_references[DISPLAY_1080P][SPECTATE_IMAGE_RED] = pixReadMemBmp(ref_spectate_red_bmp, ref_spectate_red_bmp_size);
    filter->banner_references[DISPLAY_1080P][SPECTATE_IMAGE_GREEN] = pixReadMemBmp(ref_spectate_green_bmp, ref_spectate_green_bmp_size);
    filter->banner_references[DISPLAY_1080P][SPECTATE_IMAGE_ORANGE] = pixReadMemBmp(ref_spectate_orange_bmp, ref_spectate_orange_bmp_size);
    filter->banner_references[DISPLAY_1080P][SPECTATE_IMAGE_BLUE] = pixReadMemBmp(ref_spectate_blue_bmp, ref_spectate_blue_bmp_size);

    filter->pg_references[DISPLAY_1080P][BLOODHOUND] = pixReadMemBmp(game_bloodhound_bmp, game_bloodhound_bmp_size);
    filter->pg_references[DISPLAY_1080P][GIBRALTAR] = pixReadMemBmp(game_gibraltar_bmp, game_gibraltar_bmp_size);
    filter->pg_references[DISPLAY_1080P][LIFELINE] = pixReadMemBmp(game_lifeline_bmp, game_lifeline_bmp_size);
    filter->pg_references[DISPLAY_1080P][PATHFINDER] = pixReadMemBmp(game_pathfinder_bmp, game_pathfinder_bmp_size);
    filter->pg_references[DISPLAY_1080P][WRAITH] = pixReadMemBmp(game_wraith_bmp, game_wraith_bmp_size);
    filter->pg_references[DISPLAY_1080P][BANGALORE] = pixReadMemBmp(game_bangalore_bmp, game_bangalore_bmp_size);
    filter->pg_references[DISPLAY_1080P][CAUSTIC] = pixReadMemBmp(game_caustic_bmp, game_caustic_bmp_size);
    filter->pg_references[DISPLAY_1080P][MIRAGE] = pixReadMemBmp(game_mirage_bmp, game_mirage_bmp_size);
    filter->pg_references[DISPLAY_1080P][OCTANE] = pixReadMemBmp(game_octane_bmp, game_octane_bmp_size);
    filter->pg_references[DISPLAY_1080P][WATTSON] = pixReadMemBmp(game_wattson_bmp, game_wattson_bmp_size);
    filter->pg_references[DISPLAY_1080P][CRYPTO] = pixReadMemBmp(game_crypto_bmp, game_crypto_bmp_size);
    filter->pg_references[DISPLAY_1080P][REVENANT] = pixReadMemBmp(game_revenant_bmp, game_revenant_bmp_size);
    filter->pg_references[DISPLAY_1080P][LOBA] = pixReadMemBmp(game_loba_bmp, game_loba_bmp_size);
    filter->pg_references[DISPLAY_1080P][RAMPART] = pixReadMemBmp(game_rampart_bmp, game_rampart_bmp_size);
    filter->pg_references[DISPLAY_1080P][HORIZON] = pixReadMemBmp(game_horizon_bmp, game_horizon_bmp_size);
    filter->pg_references[DISPLAY_1080P][FUSE] = pixReadMemBmp(game_fuse_bmp, game_fuse_bmp_size);
    filter->pg_references[DISPLAY_1080P][VALKYRIE] = pixReadMemBmp(game_valkyrie_bmp, game_valkyrie_bmp_size);
    filter->pg_references[DISPLAY_1080P][SEER] = pixReadMemBmp(game_seer_bmp, game_seer_bmp_size);
    filter->pg_references[DISPLAY_1080P][ASH] = pixReadMemBmp(game_ash_bmp, game_ash_bmp_size);
    filter->pg_references[DISPLAY_1080P][MADMAGGIE] = pixReadMemBmp(game_madmaggie_bmp, game_madmaggie_bmp_size);
    filter->pg_references[DISPLAY_1080P][NEWCASTLE] = pixReadMemBmp(game_newcastle_bmp, game_newcastle_bmp_size);
    filter->pg_references[DISPLAY_1080P][VANTAGE] = pixReadMemBmp(game_vantage_bmp, game_vantage_bmp_size);
    filter->pg_references[DISPLAY_1080P][CATALYST] = pixReadMemBmp(game_catalyst_bmp, game_catalyst_bmp_size);
    filter->pg_references[DISPLAY_1080P][BALLISTIC] = pixReadMemBmp(game_ballistic_bmp, game_ballistic_bmp_size);
}

static void load_2k_references(apex_game_filter_context_t *filter)
{
    filter->banner_references[DISPLAY_2K][MAP_GAME_BUTTON] = pixReadMemBmp(ref_game_map_2k_bmp, ref_game_map_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][GRENADE_GAME_BUTTON] = pixReadMemBmp(ref_game_grenade_2k_bmp, ref_game_grenade_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][ESC_LOOTING_BUTTON] = pixReadMemBmp(ref_looting_2k_bmp, ref_looting_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][ESC_INVENTORY_BUTTON] = pixReadMemBmp(ref_inventory_2k_bmp, ref_inventory_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][GRAYBAR_INVENTORY_BUTTON] = pixReadMemBmp(ref_graybar_inventory_2k_bmp, ref_graybar_inventory_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][M_MAP_BUTTON] = pixReadMemBmp(ref_map_2k_bmp, ref_map_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][PAD_MAP_BUTTON] = pixReadMemBmp(ref_pad_map_2k_bmp, ref_pad_map_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][PAD_LOOTING_BUTTON] = pixReadMemBmp(ref_pad_looting_2k_bmp, ref_pad_looting_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][PAD_INVENTORY_BUTTON] = pixReadMemBmp(ref_pad_inventory_2k_bmp, ref_pad_inventory_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][PAD_TACTICAL_BUTTON] = pixReadMemBmp(ref_pad_tactical_2k_bmp, ref_pad_tactical_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][SPECTATE_IMAGE_RED] = pixReadMemBmp(ref_spectate_red_2k_bmp, ref_spectate_red_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][SPECTATE_IMAGE_GREEN] = pixReadMemBmp(ref_spectate_green_2k_bmp, ref_spectate_green_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][SPECTATE_IMAGE_ORANGE] = pixReadMemBmp(ref_spectate_orange_2k_bmp, ref_spectate_orange_2k_bmp_size);
    filter->banner_references[DISPLAY_2K][SPECTATE_IMAGE_BLUE] = pixReadMemBmp(ref_spectate_blue_2k_bmp, ref_spectate_blue_2k_bmp_size);

    filter->pg_references[DISPLAY_2K][BLOODHOUND] = pixReadMemBmp(game_bloodhound_2k_bmp, game_bloodhound_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][GIBRALTAR] = pixReadMemBmp(game_gibraltar_2k_bmp, game_gibraltar_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][LIFELINE] = pixReadMemBmp(game_lifeline_2k_bmp, game_lifeline_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][PATHFINDER] = pixReadMemBmp(game_pathfinder_2k_bmp, game_pathfinder_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][WRAITH] = pixReadMemBmp(game_wraith_2k_bmp, game_wraith_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][BANGALORE] = pixReadMemBmp(game_bangalore_2k_bmp, game_bangalore_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][CAUSTIC] = pixReadMemBmp(game_caustic_2k_bmp, game_caustic_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][MIRAGE] = pixReadMemBmp(game_mirage_2k_bmp, game_mirage_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][OCTANE] = pixReadMemBmp(game_octane_2k_bmp, game_octane_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][WATTSON] = pixReadMemBmp(game_wattson_2k_bmp, game_wattson_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][CRYPTO] = pixReadMemBmp(game_crypto_2k_bmp, game_crypto_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][REVENANT] = pixReadMemBmp(game_revenant_2k_bmp, game_revenant_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][LOBA] = pixReadMemBmp(game_loba_2k_bmp, game_loba_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][RAMPART] = pixReadMemBmp(game_rampart_2k_bmp, game_rampart_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][HORIZON] = pixReadMemBmp(game_horizon_2k_bmp, game_horizon_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][FUSE] = pixReadMemBmp(game_fuse_2k_bmp, game_fuse_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][VALKYRIE] = pixReadMemBmp(game_valkyrie_2k_bmp, game_valkyrie_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][SEER] = pixReadMemBmp(game_seer_2k_bmp, game_seer_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][ASH] = pixReadMemBmp(game_ash_2k_bmp, game_ash_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][MADMAGGIE] = pixReadMemBmp(game_madmaggie_2k_bmp, game_madmaggie_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][NEWCASTLE] = pixReadMemBmp(game_newcastle_2k_bmp, game_newcastle_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][VANTAGE] = pixReadMemBmp(game_vantage_2k_bmp, game_vantage_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][CATALYST] = pixReadMemBmp(game_catalyst_2k_bmp, game_catalyst_2k_bmp_size);
    filter->pg_references[DISPLAY_2K][BALLISTIC] = pixReadMemBmp(game_ballistic_2k_bmp, game_ballistic_2k_bmp_size);
}

static void *apex_game_filter_create(obs_data_t *settings, obs_source_t *source)
{
    binfo("creating new filter");

    apex_game_filter_context_t *filter = bzalloc(sizeof(apex_game_filter_context_t));

    filter->source = source;
    filter->texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

    filter->image = pixCreate(2560, 1440, 32);

    load_1080p_references(filter);
    load_2k_references(filter);

    filter->debug_mode = false;
    filter->debug_counter = 0;

    filter->display = DISPLAY_RESOLUTIONS;

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

    for (enum display_resolution ds = 0; ds < DISPLAY_RESOLUTIONS; ds++)
        for (enum area_name an = 0; an < AREAS_NUM; an++)
            pixDestroy(&filter->banner_references[ds][an]);

    for (enum display_resolution ds = 0; ds < DISPLAY_RESOLUTIONS; ds++)
        for (character_name_t pg = 0; pg < CHARACTERS_NUM; pg++)
            pixDestroy(&filter->pg_references[ds][pg]);

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
        filter->width = width;
        filter->height = height;

        if (filter->width == 1920 && filter->height == 1080)
            filter->display = DISPLAY_1080P;
        else if (filter->width == 2560 && filter->height == 1440)
            filter->display = DISPLAY_2K;
        else
            filter->display = DISPLAY_RESOLUTIONS;

        binfo("new size, %dx%d, display %d", width, height, filter->display);
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
    obs_property_list_add_string(p, "简体中文", "zh");

    obs_properties_add_group(props, "sources", "Sources configuration", OBS_GROUP_NORMAL, group_2);

    p = obs_properties_add_list(group_2, "game_source", "Game Overlay Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    p = obs_properties_add_list(group_2, "inventory_source", "Game Inventory Overlay Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    p = obs_properties_add_list(group_2, "looting_source", "Game Looting Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    p = obs_properties_add_list(group_2, "map_source", "Game Map Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_enum_sources(list_add_sources, p);

    p = obs_properties_add_list(group_2, "spectate_source", "Spectate Source", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
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
