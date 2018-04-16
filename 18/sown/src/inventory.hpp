#ifndef SOWN_INVENTORY_HPP__GUARD_
#define SOWN_INVENTORY_HPP__GUARD_

#include <memory>

#include "clenche.hpp"

#include <SFML/Graphics.hpp>

#include "snow.hpp"
#include "draw.hpp"
#include "logic.hpp"
#include "agents.hpp"
#include "globals.hpp"

namespace sown
{
    template<typename t_items>
    struct inventory_config;

    template<>
    struct inventory_config<sown::planks>
    {
        using uint_type = globals::t_uint;

        constexpr static const int* color = globals::land::plank;
        constexpr static int max = 10;
        constexpr static int ini = 0;

        template<typename t_machine>
        static void create(t_machine& machine, uint_type x, uint_type y)
        {
            auto& planks = machine.template get<sown::planks>();
            planks.add(x, y);
        }
    };

    template<>
    struct inventory_config<sown::woods>
    {
        using uint_type = globals::t_uint;

        constexpr static const int* color = globals::land::wood;
        constexpr static int max = 15;
        constexpr static int ini = 0;

        template<typename t_machine>
        static void create(t_machine& machine, uint_type x, uint_type y)
        {
            auto& woods = machine.template get<sown::woods>();
            woods.add(x, y, false);
        }
    };

    template<>
    struct inventory_config<sown::leafs>
    {
        using uint_type = globals::t_uint;

        constexpr static const int* color = globals::land::leaf;
        constexpr static int max = 10;
        constexpr static int ini = 0;

        template<typename t_machine>
        static void create(t_machine& machine, uint_type x, uint_type y)
        {
            auto& leafs = machine.template get<sown::leafs>();
            leafs.add(x, y, false);
        }
    };

    template<>
    struct inventory_config<sown::rocks>
    {
        using uint_type = globals::t_uint;

        constexpr static const int* color = globals::land::rock;
        constexpr static int max = 5;
        constexpr static int ini = 1;

        template<typename t_machine>
        static void create(t_machine& machine, uint_type x, uint_type y)
        {
            auto& rocks = machine.template get<sown::rocks>();
            rocks.add(x, y);
        }
    };

    template<>
    struct inventory_config<sown::coals>
    {
        using uint_type = globals::t_uint;

        constexpr static const int* color = globals::land::coal;
        constexpr static int max = 2;
        constexpr static int ini = 0;

        template<typename t_machine>
        static void create(t_machine& machine, uint_type x, uint_type y)
        {
            auto& coals = machine.template get<sown::coals>();
            coals.add(x, y, false);
        }
    };

    template<>
    struct inventory_config<sown::snow::flakes>
    {
        using uint_type = globals::t_uint;

        constexpr static const int color[] = {0xFF, 0xFF, 0xFF, 0xFF};
        constexpr static int max = 20;
        constexpr static int ini = 0;

        template<typename t_machine>
        static void create(t_machine& machine, uint_type x, uint_type y)
        {
            auto& flakes = machine.template get<sown::snow::flakes>();
            flakes.add(x, y, 0, 0, 0);
        }
    };

    template<typename t_items>
    struct inventory_item
    {
        using uint_type = globals::t_uint;

        template<typename t_machine, typename t_inventory>
        void reset(t_machine& machine, t_inventory& inventory)
        {
            _count = inventory_config<t_items>::ini;
            _index = inventory._index;
            inventory._index += 1;

            _shape = std::make_shared<sf::RectangleShape>();
            _shape->setFillColor(sf::Color(
                inventory_config<t_items>::color[0],
                inventory_config<t_items>::color[1],
                inventory_config<t_items>::color[2],
                inventory_config<t_items>::color[3]));

            auto& drawables = machine.template get<sown::drawables>();
            drawables.add(_shape);
        }

        template<typename t_machine, typename t_inventory>
        void update(t_machine& machine, t_inventory& inventory)
        {
            if(!_shape)
                reset(machine, inventory);

            auto& logic = machine.template get<sown::logic>();
            if(auto window = logic._window.lock(); window)
            {
                auto pos = window->mapPixelToCoords(
                    sf::Vector2i(12, 7 + (3 + _index) * 8));
                _shape->setSize(sf::Vector2f(_count, 1));
                _shape->setPosition(pos);
            }
        }

        template<typename t_machine, typename t_inventory>
        void next(t_machine& machine, t_inventory& inventory)
        {
            if(_index == 0)
                return;

            if(inventory._active == _index && _count < 1)
                inventory.next(machine);
        }

        template<typename t_machine, typename t_inventory>
        void unpick(
                t_machine& machine,
                t_inventory& inventory,
                uint_type x,
                uint_type y)
        {
            if(_index != inventory._active)
                return;

            if(_count < 1)
                return;

            sound::play<t_machine, t_items>(machine);

            _count -= 1;
            inventory_config<t_items>::create(machine, x, y);
            update(machine, inventory);

            auto& logic = machine.template get<sown::logic>();
            logic._pixels->get(x, y) = inventory_config<t_items>::color;
        }

        template<typename t_machine, typename t_inventory>
        bool pick(
                t_machine& machine,
                t_inventory& inventory,
                uint_type x,
                uint_type y)
        {
            auto lcount = _count;
            auto& items = machine.template get<t_items>();
            for(auto& f : items.functors)
            {
                if(_count >= inventory_config<t_items>::max)
                    break;

                if(f.deleted)
                    continue;

                if(f._x != x || f._y != y)
                    continue;

                f.deleted = true;
                items.dirty = true;

                auto& logic = machine.template get<sown::logic>();
                logic._pixels->get(f._x, f._y).reset();

                _count += 1;
                break;
            }
            items.clean();
            update(machine, inventory);

            if(lcount != _count)
                sound::play<t_machine, sown::sound::pick>(machine);
            return lcount != _count;
        }

    private:
        std::shared_ptr<sf::RectangleShape> _shape;
        uint_type _count = 0;
        uint_type _index = 0;
    };

    template<typename... t_items>
    struct inventory_details :
        cl::enable<inventory_details<t_items...>>,
        inventory_item<t_items>...
    {
        using uint_type = globals::t_uint;

        template<typename t_item, typename t_machine>
        void reset_item(t_machine& machine)
        {
            static_cast<inventory_item<t_item>*>(this)->reset(machine, *this);
        }

        template<typename t_item, typename t_machine>
        void next_item(t_machine& machine)
        {
            static_cast<inventory_item<t_item>*>(this)->next(machine, *this);
        }

        template<typename t_item, typename t_machine>
        void update_item(t_machine& machine)
        {
            static_cast<inventory_item<t_item>*>(this)->update(machine, *this);
        }

        template<typename t_item, typename t_machine>
        bool pick_item(t_machine& machine, uint_type x, uint_type y)
        {
            return static_cast<inventory_item<t_item>*>(this
                )->pick(machine, *this, x, y);
        }

        template<typename t_item, typename t_machine>
        void unpick_item(t_machine& machine, uint_type x, uint_type y)
        {
            static_cast<inventory_item<t_item>*>(this
                )->unpick(machine, *this, x, y);
        }

        template<typename t_machine>
        void reset(t_machine& machine)
        {
            _index = 0;
            _active = 0;

            _shape = std::make_shared<sf::RectangleShape>();
            _shape->setFillColor(sf::Color(
                globals::player::cursor[0],
                globals::player::cursor[1],
                globals::player::cursor[2],
                globals::player::cursor[3]));
            _shape->setSize(sf::Vector2f(1, 1));

            auto& drawables = machine.template get<sown::drawables>();
            drawables.add(_shape);

            [](...){}((reset_item<t_items, t_machine>(machine), 0)...);
        }

        template<typename t_machine>
        void update(t_machine& machine)
        {
            if(!_shape)
                reset(machine);

            auto& logic = machine.template get<sown::logic>();
            if(auto window = logic._window.lock(); window)
            {
                auto pos = window->mapPixelToCoords(
                    sf::Vector2i(4, 7 + (3 + _active) * 8));
                _shape->setPosition(pos);
            }
        }

        template<typename t_machine>
        void unpick(t_machine& machine, uint_type x, uint_type y)
        {
            [](...){}((unpick_item<t_items, t_machine>(machine, x, y), 0)...);
        }

        template<typename t_machine>
        void next(t_machine& machine)
        {
            _active = (_active + 1) % _index;
            [](...){}((next_item<t_items, t_machine>(machine), 0)...);
            update(machine);
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            update(machine);
            [](...){}((update_item<t_items, t_machine>(machine), 0)...);
        }

        uint_type _index = 0, _active = 0;
        std::shared_ptr<sf::RectangleShape> _shape;
    };

    using inventory = inventory_details<
        sown::coals,
        sown::planks,
        sown::leafs,
        sown::woods,
        sown::snow::flakes,
        sown::rocks>;
}

#endif
