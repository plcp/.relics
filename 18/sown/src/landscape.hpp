#ifndef SOWN_LANDSCAPE_HPP__GUARD_
#define SOWN_LANDSCAPE_HPP__GUARD_

#include <limits>
#include <random>

#include "clenche.hpp"
#include "property.hpp"

#include "logic.hpp"
#include "trees.hpp"
#include "player.hpp"
#include "globals.hpp"

namespace sown
{
    struct landscape : cl::enable<landscape>
    {
        using sroll_type = std::uniform_int_distribution<size_t>;
        using roll_type = std::uniform_int_distribution<int>;
        constexpr static int
            _flat = 0,
            _regular = 1,
            _climbing = 2,
            _lowering = 3;

        landscape()
        { }

        template<typename t_machine>
        void reset(t_machine& machine)
        {
            std::random_device rd("/dev/urandom");
            sroll_type rseed(0, std::numeric_limits<size_t>::max());
            _seed = rseed(rd);

            auto& rocks = machine.template get<sown::rocks>();
            auto& coals = machine.template get<sown::coals>();
            auto& logic = machine.template get<sown::logic>();

            _height = logic._height;
            _width = logic._width;
            _init = true;

            std::mt19937 roll(_seed);
            roll_type grass_height(1, 16);

            roll_type climbing_mode(0, globals::land::upper_width);
            roll_type lowering_mode(0, globals::land::lower_width);
            roll_type regular_mode(0, globals::land::regular_width);
            roll_type flat_mode(0, globals::land::flat_width);
            roll_type flatness(0, globals::land::flat_bias);
            roll_type choice(0, 2 + globals::land::regular_bias);
            roll_type trees(0, globals::land::trees_bias);
            roll_type delta(0, globals::land::max_delta);
            roll_type bush(0, globals::land::bush_bias);
            roll_type rock(0, globals::land::rock_bias);
            roll_type coal(0, globals::land::coal_bias);
            roll_type food(0, globals::land::food_bias);

            auto mode = _lowering;
            int width = _width, height = _height;
            for(int x = 0, y = height / 2; x < width; x += 1)
            {
                if(y < 1)
                    y = 1;
                if(y > height - globals::land::min_height)
                    y = height - globals::land::min_height;

                if(mode == _climbing || mode == _lowering)
                {
                    int i = 0;
                    if(coal(roll) == 0)
                    {
                        coals.add(x, y - 2, false);
                        i += 2;
                    }

                    for(; rock(roll) == 0 || rock(roll) == i - 1; i += 2)
                        rocks.add(x, y - 2 - i);

                    if(std::abs(width / 2 - x) > globals::trees::startzone)
                        if(i == 0 && bush(roll) == 0)
                            sown::trees::grow(machine, x, y - 1,
                                globals::trees::bush_size / 2);
                }
                else if(trees(roll) == 0)
                    if(std::abs(width / 2 - x) > globals::trees::startzone)
                        sown::trees::grow(machine, x, y - 1);

                if(food(roll) == 0)
                    trees::drop_food(machine, x, y - 1);

                auto gh = grass_height(roll);
                if(gh == 16)
                    gh = 4;
                else if(gh > 3)
                    gh = gh % 2 + 2;

                for(int i = 0; y + i < height; i += 1)
                {
                    auto& here = logic._pixels->get(x, y + i);
                    if(gh > i)
                    {
                        here = globals::land::grass;
                        continue;
                    }
                    else
                        here = globals::land::dirt;

                    auto offset = i / 2;
                    auto roffset = offset / 4;
                    here.r = here.r > roffset ? here.r - roffset : 1;
                    here.g = here.g > roffset ? here.g - roffset : 1;
                    here.b = here.b > roffset ? here.b - roffset : 1;

                    switch(3 - (offset % 4))
                    {
                        case 3:
                            [[fallthrough]];
                        case 2:
                            here.r += 1;
                            [[fallthrough]];
                        case 1:
                            here.g += 1;
                            [[fallthrough]];
                        case 0:
                            here.b += 1;
                    }
                }

                switch(mode)
                {
                    case _flat:
                        if(!flat_mode(roll))
                            mode = _regular;
                        break;
                    case _regular:
                        if(!regular_mode(roll))
                            switch(choice(roll))
                            {
                                case 0:
                                    mode = _flat; break;
                                case 1:
                                    mode = _climbing; break;
                                case 2:
                                    mode = _lowering; break;
                                default:
                                    mode = _regular; break;
                            }
                        if(!flatness(roll))
                            y += ((delta(roll) % 5) - 2) / 2;
                        break;
                    case _climbing:
                        if(!climbing_mode(roll))
                            switch(choice(roll))
                            {
                                case 0:
                                    mode = _regular; break;
                                case 1:
                                    mode = _climbing; break;
                                default:
                                    mode = _regular; break;
                            }
                        if(!flatness(roll))
                            y += (delta(roll)
                                    - globals::land::max_delta + 4) / 2;
                        break;
                    case _lowering:
                        if(!lowering_mode(roll))
                            switch(choice(roll))
                            {
                                case 0:
                                    mode = _regular; break;
                                case 1:
                                    mode = _lowering; break;
                                default:
                                    mode = _regular; break;
                            }
                        if(!flatness(roll))
                            y += (delta(roll) - 4) / 2;
                        break;
                }

                if(x < globals::land::lower_width * 4)
                    mode = _lowering;
                if(x > width - globals::land::upper_width * 4)
                    mode = _climbing;
                if(x > width / 2 - globals::land::start
                    && x < width / 2 + globals::land::start)
                    mode = _flat;

                if(x == width / 2)
                {
                    auto& player = machine.template get<sown::player>();
                    player.move(machine, x, y - 1);

                    player._help_sprite->setPosition(
                        sf::Vector2f(x - 21, y + 3));

                    auto w = x - globals::hut::width + 2;
                    trees::hut(machine, w, y);
                    trees::drop_food(machine, w + 1, y - 1);
                    trees::drop_food(machine, w + 1, y - 2);
                    trees::drop_food(machine, w + 2, y - 1);

                    rocks.add(x - 6, y - 2);
                    coals.add(x - 5, y - 2, true);
                    rocks.add(x - 4, y - 2);
                }
            }

            for(int i = 0; i < height; i += 1)
            {
                logic._pixels->get(0, i) = {1, 1, 1, 1};
                logic._pixels->get(width - 1, i) = {1, 1, 1, 1};
            }

            if(auto& player = machine.template get<sown::player>();
                player._y < height - globals::land::min_height)
            {
                auto water_height = player._y + globals::land::min_height;
                for(int i = 0; i < width; i += 1)
                {
                    for(int j = water_height; j < height - 1; j += 1)
                    {
                        if(!logic._pixels->get(i, j).b == 0)
                            break;
                        logic._pixels->get(i, j) = globals::land::water;
                    }
                }
            }

        }

        template<typename t_machine>
        void scan(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();

            bool iced = false;
            for(int i = 0; i < globals::land::scan_speed; i += 1)
            {
                yscan += 1;
                if(yscan > _height)
                {
                    xscan += 1;
                    yscan = 0;
                }
                if(xscan > _width)
                {
                    xscan = 0;
                }

                auto& here = logic._pixels->get(xscan, yscan);
                if(!iced && here.r == 0xFF)
                    iced = true;
                else if(!iced
                    && logic.tick(4) == 0
                    && here.r == globals::land::ice[0]
                    && here.g == globals::land::ice[1]
                    && here.b == globals::land::ice[2])
                    iced = true;
                else if(!iced
                    && logic.tick(4) == 0
                    && here.r == globals::land::iced[0]
                    && here.g == globals::land::iced[1]
                    && here.b == globals::land::iced[2])
                    iced = true;
                else if(iced)
                {
                    if(true
                        && logic.tick(2) == 0
                        && here.r == globals::land::grass[0]
                        && here.g == globals::land::grass[1]
                        && here.b == globals::land::grass[2])
                    {
                        here = globals::land::ice;
                        yscan -= globals::land::min_height;
                        xscan += 1;
                    }
                    else if(true
                        && logic.tick(2) == 0
                        && here.r == globals::land::water[0]
                        && here.g == globals::land::water[1]
                        && here.b == globals::land::water[2])
                    {
                        here = globals::land::iced;
                        yscan -= globals::land::min_height;
                        xscan += 1;
                    }

                    iced = false;
                }
            }
            if(yscan > 2)
                yscan -= 2;
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(!_init || logic._width != _width || logic._height != _height)
                reset(machine);
            scan(machine);
        }

        bool _init = false;
        size_t xscan = 0, yscan = 0;
        size_t _width, _height, _seed = 0;
    };
}

#endif
