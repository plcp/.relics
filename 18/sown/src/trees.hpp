#ifndef SOWN_TREES_HPP__GUARD_
#define SOWN_TREES_HPP__GUARD_

#include <random>

#include "logic.hpp"
#include "agents.hpp"
#include "globals.hpp"

namespace sown::trees
{
    using int_type = globals::t_int;
    using roll_type = std::uniform_int_distribution<int>;

    template<typename t_machine>
    void wall(t_machine& machine, int_type x, int_type y)
    {
        auto& woods = machine.template get<sown::woods>();
        auto& rocks = machine.template get<sown::rocks>();

        int i, j;
        for(i = 0; i < globals::hut::height; i += 1)
            woods.add(x, y - i, true);
        for(j = 0; j < globals::hut::fork; j += 1)
        {
            woods.add(x - j, y - i - j, true);
            woods.add(x + j, y - i - j, true);
        }

        rocks.add(x, y - i - j - 2);
        rocks.add(x - 1, y - i - j - 2);
        rocks.add(x + 1, y - i - j - 2);
        rocks.add(x, y - i - j - 3);
    }

    template<typename t_machine>
    void hut(t_machine& machine, int_type x, int_type y)
    {
        auto& woods = machine.template get<sown::woods>();
        auto& leafs = machine.template get<sown::leafs>();

        wall(machine, x, y);
        wall(machine, x + globals::hut::width, y);

        auto k = globals::hut::offset;
        auto l = globals::hut::height + globals::hut::fork;
        for(int i = -k + 1; i < globals::hut::width + k; i += 1)
            woods.add(x + i, y - l, true);

        for(int i = 2; i < globals::hut::width - 1; i += 1)
            leafs.add(x + i, y - l - 1, 2);
    }

    std::mt19937 roll(0);
    roll_type bigger(0, globals::trees::bigger_bias);
    roll_type branch(0, globals::trees::branch_bias);
    roll_type trunk(0, globals::trees::trunk_bias);
    roll_type leaf(0, globals::trees::leaf_bias);
    roll_type binary(0, 1);
    roll_type lr(-1, 1);

    template<typename t_machine>
    void drop_food(t_machine& machine, int_type x, int_type y)
    {
        auto& logic = machine.template get<sown::logic>();
        auto& here = logic._pixels->get(x, y);
        if(false
            || here.r != 0
            || here.g != 0
            || here.b != 0)
            return;

        here = globals::player::food_color;
    }

    template<typename t_machine>
    void add_leafs(t_machine& machine, int_type x, int_type y)
    {
        auto& logic = machine.template get<sown::logic>();
        auto& leafs = machine.template get<sown::leafs>();
        for(int j = 0; j < leaf(roll) + 1; j += 1)
        {
            auto& here = logic._pixels->get(x, y - j - 1);
            if(here.r == 0 && here.g == 0 && here.b == 0)
               leafs.add(x, y - j - 1, 2);
        }
    }

    template<typename t_machine>
    bool grow(t_machine& machine, int_type x, int_type y,
        int_type size = globals::trees::tree_size, int_type limit = 0)
    {
        auto& logic = machine.template get<sown::logic>();
        auto& here = logic._pixels->get(x, y);
        if(false
            || here.r != 0
            || here.g != 0
            || here.b != 0)
            return false;

        auto& woods = machine.template get<sown::woods>();
        here = globals::land::wood;
        woods.add(x, y, true);

        size -= 1;
        if(bigger(roll) == 0)
            size += globals::trees::bigger_rate;

        if(size < 0 || limit > globals::trees::limit)
            return true;

        if(size > globals::trees::bush_size)
        {
            if(false
                || logic._pixels->get(x - 1, y).g == globals::land::wood[1]
                || logic._pixels->get(x + 1, y).g == globals::land::wood[1])
                return false;

            grow(machine, x, y - 1, size - 1, limit + 1);
            return true;
        }

        if(branch(roll) == 0)
        {
            if(binary(roll))
            {
                grow(machine, x - 1, y, size - 1, limit + 1);
                grow(machine, x + 1, y, size - 1, limit + 1);
            }
            else
            {
                grow(machine, x + 1, y, size - 1, limit + 1);
                grow(machine, x - 1, y, size - 1, limit + 1);
            }
        }
        else if(branch(roll) == 0)
        {
            if(binary(roll))
            {
                if(trunk(roll) == 0)
                    grow(machine, x, y - 1, size - 1, limit + 1);
                grow(machine, x - 1, y, size - 1, limit + 1);
            }
            else
            {
                if(trunk(roll) == 0)
                    grow(machine, x, y - 1, size - 1, limit + 1);
                grow(machine, x + 1, y, size - 1, limit + 1);
            }
        }
        else if(trunk(roll) == 0)
        {
            grow(machine, x - lr(roll), y - 1, size - 1, limit + 1);
        }
        add_leafs(machine, x, y);
        return true;
    }
}

#endif
