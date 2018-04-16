#ifndef SOWN_GLOBALS_HPP__GUARD_
#define SOWN_GLOBALS_HPP__GUARD_

#include <cstdint>

namespace sown::globals
{
    /* Integer type used */
    using t_int = int16_t;
    using t_uint = uint16_t;

    /* Name of the window */
    constexpr char window_name[] = "sown 2006 :: deluxe edition";

    /* Timers */
    constexpr int starttime = 30;

    /* Volumes */
    constexpr int musicvolume = 60;
    constexpr int fxvolume = 50;

    /* Resources directory */
    constexpr char resources_directory[] = "./resources/";
    constexpr char sprites_directory[] = "./resources/sprites/";
    constexpr char sounds_directory[] = "./resources/sounds/";
    constexpr char fonts_directory[] = "./resources/fonts/";

    constexpr char help_sprite[] = "help.png";

    /* Font names */
    constexpr char default_font[] = "Inconsolata.otf";

    /* Framerate limit (sfml builtin) */
    constexpr int target_framerate = 60;

    /* Target frametime in microseconds */
    constexpr size_t target_frametime = 20000;

    /* Cleanup threshold (trigger deleted entries cleanup) */
    constexpr size_t cleanup_threshold = 2;

    /* Size of the grid used to simulate the snow */
    constexpr int grid_size = 4;

    /* Snow constants */
    namespace snow
    {
        constexpr int max_quantity = 300000;
        constexpr int lifetime = 4000;
        constexpr int melttime = 16;
        constexpr int melting = 128;

        constexpr int period = 64;
        constexpr int growth = 128;
        constexpr int slow = 2;
        int rate = 1;
        int irate = 1;

        constexpr int velocity = 5;
        constexpr int cohesion = 2;
        constexpr int dispertion = 2;

        constexpr int blown_period = 4 * 256;
        constexpr int blown_x = 1;
        constexpr int blown_y = 1;

        constexpr int blown_average = 2;
    }

    /* Hut constants */
    namespace hut
    {
        constexpr int height = 7;
        constexpr int width = 14;
        constexpr int offset = 3;
        constexpr int fork = 2;
    }

    /* Trees constants */
    namespace trees
    {
        constexpr int decay = 16 * 32;

        constexpr int limit = 64;
        constexpr int unstable = 4;
        constexpr int startzone = 32;

        constexpr int bigger_bias = 8;
        constexpr int bigger_rate = 2;
        constexpr int branch_bias = 2;
        constexpr int trunk_bias = 1;
        constexpr int leaf_bias = 1;

        constexpr int bush_size = 24;
        constexpr int tree_size = 32;
    }

    /* Landscape constants */
    namespace land
    {
        constexpr int regular_bias = 4;
        constexpr int trees_bias = 16;
        constexpr int bush_bias = 16;
        constexpr int rock_bias = 4;
        constexpr int food_bias = 32;
        constexpr int coal_bias = 128;
        constexpr int flat_bias = 2;

        constexpr int regular_width = 8;
        constexpr int upper_width = 16;
        constexpr int lower_width = 16;
        constexpr int flat_width = 16;

        constexpr int start = 32;

        constexpr int min_height = 32;
        constexpr int max_delta = 32;

        constexpr int scan_speed = 512;

        constexpr int grass[] = {0x07, 0x21, 0x18, 0xFF};
        constexpr int dirt[] = {0x21, 0x18, 0x07, 0xFF};
        constexpr int ice[] = {0x07, 0x41, 0x38, 0xFF};

        constexpr int water[] = {0x00, 0x00, 0xFD, 0x30};
        constexpr int iced[] = {0xA0, 0xA0, 0xFC, 0x50};

        constexpr int fire[] = {0xF0, 0x00, 0x00, 0x70};
        constexpr int rock[] = {0x42, 0x42, 0x42, 0xFE};
        constexpr int coal[] = {0x37, 0x13, 0x13, 0xFD};
        constexpr int wood[] = {0x51, 0x21, 0x00, 0xFC};
        constexpr int leaf[] = {0x10, 0x23, 0x00, 0xFB};
        constexpr int plank[] = {0x87, 0x57, 0x00, 0xFA};
    }

    namespace fire
    {
        constexpr int start = 16 * 64;
        constexpr int range = 8;
        constexpr int rate = 64;
        constexpr int life = 4;
        constexpr int warm = 2;
    }

    /* Player constants */
    namespace player
    {
        constexpr int rate = 4;
        constexpr int fall = 2;
        constexpr int jump = 6;
        constexpr int grace = 2;
        constexpr int grace_unpick = 6;

        constexpr int max_temp = 40;
        constexpr int max_food = 30;

        constexpr int tomb[] = {0xFF, 0x00, 0x00, 0xFF};
        constexpr int color[] = {0x20, 0x40, 0x60, 0xE0};
        constexpr int cursor[] = {0xFF, 0xFF, 0x0, 0xFF};

        namespace freeze
        {
            constexpr int min_temp = 0;

            constexpr int rate = 8 * 32;
            constexpr int iced = 6 * 32;
            constexpr int snow = 4 * 32;
            constexpr int feet = 2 * 32;
            constexpr int head = 16;
        }

        namespace hungry
        {
            constexpr int min_food = 0;

            constexpr int rate = 2 * 256;
            constexpr int freezing = 4;

            constexpr int single_drop = 3;
        }

        constexpr int temp_color[] = {0x20, 0x40, 0x60, 0xE0};
        constexpr int food_color[] = {0x20, 0x60, 0x40, 0xE0};
    }

    /* View constants */
    namespace view
    {
        constexpr int max_dist = 32;
        constexpr int hoffset = 16;
        constexpr int height = 144;
        constexpr int width = 256;
    }
}

namespace sown
{
    constexpr globals::t_int scale_to_grid(const globals::t_int& grid_offset)
    {
        globals::t_int aligned = grid_offset - globals::grid_size / 2;
        return aligned / globals::grid_size;
    }
}

#endif
