#ifndef SOWN_COOK_HPP__GUARD_
#define SOWN_COOK_HPP__GUARD_

#include "globals.hpp"

namespace sown::cook
{
    using int_type = globals::t_int;

    template<typename t_machine>
    void recipes(t_machine& machine, int_type x, int_type y);
}

#include "logic.hpp"
#include "agents.hpp"

namespace sown::cook
{
    struct food
    {
        template<typename t_machine>
        static void apply(t_machine& machine, int_type x, int_type y)
        {
            auto& logic = machine.template get<sown::logic>();
            for(int i = 0; i < 2; i += 1)
                if(!(logic._pixels->get(x, y + i).g ==
                    globals::land::leaf[1]))
                        return;
            for(int i = 0; i < 2; i += 1)
                logic._pixels->get(x, y + i).reset();

            auto& leafs = machine.template get<sown::leafs>();
            for(auto& f : leafs.functors)
            {
                if(!(f._x == x))
                    continue;

                if(!(f._y == y || f._y == y + 1))
                    continue;

                leafs.dirty = true;
                f.deleted = true;
            }

            logic._pixels->get(x, y + 1) =
                globals::player::food_color;
        }
    };

    struct plank
    {
        template<typename t_machine>
        static void apply(t_machine& machine, int_type x, int_type y)
        {
            auto& logic = machine.template get<sown::logic>();
            for(int i = 0; i < 2; i += 1)
                if(!(logic._pixels->get(x, y + i).g ==
                    globals::land::wood[1]))
                        return;
            for(int i = 0; i < 2; i += 1)
                logic._pixels->get(x, y + i).reset();

            auto& woods = machine.template get<sown::woods>();
            for(auto& f : woods.functors)
            {
                if(!(f._x == x))
                    continue;

                if(!(f._y == y || f._y == y + 1))
                    continue;

                woods.dirty = true;
                f.deleted = true;
            }

            logic._pixels->get(x, y + 1) = globals::land::plank;
            auto& planks = machine.template get<sown::planks>();
            planks.add(x, y + 1);
        }
    };

    template<typename t_machine, typename t_recipe>
    void apply_item(t_machine& machine, int_type x, int_type y)
    {
        t_recipe::apply(machine, x, y);
    }

    template<typename t_machine, typename... t_recipes>
    void recipes_details(t_machine& machine, int_type x, int_type y)
    {
        auto& logic = machine.template get<sown::logic>();
        if(!(logic._pixels->get(x, y - 1).r == globals::land::rock[0]))
            return;

        for(int i = 0; i < 2; i += 1)
        {
            if(!(logic._pixels->get(x - 1, y + i).r
                == globals::land::rock[0]))
                return;

            if(!(logic._pixels->get(x + 1, y + i).r
                == globals::land::rock[0]))
                return;
        }

        [](...){}((apply_item<t_machine, t_recipes>(machine, x, y), 0)...);
    }

    template<typename t_machine>
    void recipes(t_machine& machine, int_type x, int_type y)
    {
        recipes_details<t_machine, food, plank>(machine, x, y);
    }
}

#endif
