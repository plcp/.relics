#ifndef SOWN_PLAYER_HPP__GUARD_
#define SOWN_PLAYER_HPP__GUARD_

#include <memory>

#include "clenche.hpp"

#include <SFML/Graphics.hpp>

namespace sown
{
    template<typename t_machine>
    void warm_player(t_machine&, globals::t_int, globals::t_int);

    template<typename t_machine>
    void tombstone(t_machine&, globals::t_int, globals::t_int);
}

#include "snow.hpp"
#include "draw.hpp"
#include "cook.hpp"
#include "sound.hpp"
#include "logic.hpp"
#include "agents.hpp"
#include "inventory.hpp"

namespace sown
{
    struct view;

    struct player : cl::enable<player>
    {
        player()
        {
            _help_texture.loadFromFile(
                std::string("")
                + globals::sprites_directory
                + globals::help_sprite);
        }

        template<typename t_machine>
        void reset(t_machine& machine)
        {
            _shape = std::make_shared<sf::RectangleShape>();
            _shape->setSize(sf::Vector2f(1, 2));
            _shape->setOrigin(0, 1);
            _shape->setFillColor(sf::Color(
                globals::player::color[0],
                globals::player::color[1],
                globals::player::color[2],
                globals::player::color[3]));

            _temp_shape = std::make_shared<sf::RectangleShape>();
            _temp_shape->setFillColor(sf::Color(
                globals::player::temp_color[0],
                globals::player::temp_color[1],
                globals::player::temp_color[2],
                globals::player::temp_color[3]));
            _temp = globals::player::max_temp;

            _food_shape = std::make_shared<sf::RectangleShape>();
            _food_shape->setFillColor(sf::Color(
                globals::player::food_color[0],
                globals::player::food_color[1],
                globals::player::food_color[2],
                globals::player::food_color[3]));
            _food = globals::player::max_food;

            _help_sprite = std::make_shared<sf::Sprite>();
            _help_texture.setSmooth(false);
            _help_sprite->setTexture(_help_texture);
            _help_sprite->setColor(sf::Color(0xFF, 0xFF, 0xFF, 0xA0));

            auto& drawables = machine.template get<sown::drawables>();
            drawables.add(_shape);
            drawables.add(_temp_shape);
            drawables.add(_food_shape);
            drawables.add(_help_sprite);
        }

        template<typename t_machine>
        void update(t_machine& machine)
        {
            if(!_temp_shape || !_food_shape)
                reset(machine);

            auto& logic = machine.template get<sown::logic>();
            if(auto window = logic._window.lock(); window)
            {
                auto tpos = window->mapPixelToCoords(
                    sf::Vector2i(12, 7));
                if(_temp > 1)
                    _temp_shape->setSize(sf::Vector2f(_temp, 1));
                else
                    _temp_shape->setSize(sf::Vector2f(1, 1));
                _temp_shape->setPosition(tpos);

                auto fpos = window->mapPixelToCoords(
                    sf::Vector2i(12, 7 + 8));
                if(_food > 1)
                    _food_shape->setSize(sf::Vector2f(_food, 1));
                else
                    _food_shape->setSize(sf::Vector2f(1, 1));
                _food_shape->setPosition(fpos);
            }
        }

        template<typename t_machine>
        void decay(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            auto ptemp = _temp;
            auto pfood = _food;

            if(!logic.tick(globals::player::hungry::rate))
                _food -= 1;

            if(!logic.tick(globals::player::freeze::rate))
                _temp -= 1;

            if(!logic.tick(globals::player::freeze::snow))
                if(false
                    || logic._pixels->get(_x - 1, _y).b >= 0xF0
                    || logic._pixels->get(_x + 1, _y).b >= 0xF0)
                        _temp -= 1;

            if(!logic.tick(globals::player::freeze::iced))
                if(false
                    || logic._pixels->get(_x, _y + 1).b >= 0xF0
                    || logic._pixels->get(_x, _y + 1).b ==
                        globals::land::ice[3])
                        _temp -= 1;

            if(!logic.tick(globals::player::freeze::feet))
                if(false
                    || logic._pixels->get(_x, _y).b >= 0xF0)
                        _temp -= 1;

            if(!logic.tick(globals::player::freeze::head))
                if(false
                    || logic._pixels->get(_x, _y - 1).b >= 0xF0)
                        _temp -= 1;

            if(_temp < globals::player::freeze::min_temp)
            {
                _temp = globals::player::freeze::min_temp;
                if(!logic.tick(globals::player::hungry::freezing))
                   _food -= 1;

                sound::play<t_machine, sown::sound::hurt>(machine);
            }

            if(ptemp != _temp || pfood != _food)
                update(machine);
        }

        template<typename t_machine>
        void move(t_machine& machine, int x, int y)
        {
            if(!_shape)
                reset(machine);

            _x = x;
            _y = y;
            _shape->setPosition(x, y);

            auto& view = machine.template get<sown::view>();
            view.update(machine);
            update(machine);
        }

        template<typename t_machine>
        void jump(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(!logic._pixels->get(_x, _y + 1).b == 0
                && logic._pixels->get(_x, _y - 1).g == 0)
            {
                int i;
                for(i = 0; i < globals::player::jump; i += 1)
                    if(!logic._pixels->get(_x, _y - i - 1).b == 0)
                        break;
                _grace = globals::player::grace;
                move(machine, _x, _y - i);

                sound::play<t_machine, sown::sound::jump>(machine);
            }
            else if(!logic._pixels->get(_x, _y + 1).g == 0)
            {
                if(logic._pixels->get(_x, _y - 1).b == 0)
                    move(machine, _x, _y - 1);
            }
        }

        template<typename t_machine>
        void pick(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            auto& inventory = machine.template get<sown::inventory>();
            switch(logic._pixels->get(_x, _y + 1).g)
            {
                case globals::player::food_color[1]:
                    _food += globals::player::hungry::single_drop;
                    if(_food >= globals::player::max_food)
                        _food = globals::player::max_food;
                    logic._pixels->get(_x, _y + 1).reset();
                    update(machine);
                    sound::play<t_machine, sown::sound::pick>(machine);
                    break;
                case globals::land::rock[1]:
                    inventory.template pick_item<
                        sown::rocks, t_machine>(machine, _x, _y + 1);
                    break;
                case globals::land::coal[1]:
                    inventory.template pick_item<
                        sown::coals, t_machine>(machine, _x, _y + 1);
                    break;
                case globals::land::plank[1]:
                    inventory.template pick_item<
                        sown::planks, t_machine>(machine, _x, _y + 1);
                    break;
                case globals::land::wood[1]:
                    [[fallthrough]];
                case globals::land::wood[1] - 1:
                    inventory.template pick_item<
                        sown::woods, t_machine>(machine, _x, _y + 1);
                    break;
                case globals::land::leaf[1]:
                    [[fallthrough]];
                case globals::land::leaf[1] - 1:
                    inventory.template pick_item<
                        sown::leafs, t_machine>(machine, _x, _y + 1);
                    break;
                case 0xFF:
                    inventory.template pick_item<
                        sown::snow::flakes, t_machine>(machine, _x, _y + 1);
                    break;
            }
        }

        template<typename t_machine>
        void next(t_machine& machine)
        {
            auto& inventory = machine.template get<sown::inventory>();
            inventory.next(machine);
        }

        template<typename t_machine>
        void unpick(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(false
                || logic._pixels->get(_x, _y).g != 0
                || logic._pixels->get(_x, _y).b != 0)
                return;
            for(int i = 1; i < 3; i += 1)
                if(!logic._pixels->get(_x, _y - i).b == 0)
                    return;
            if(true
                && logic._pixels->get(_x, _y + 1).g == 0
                && logic._pixels->get(_x, _y + 1).b == 0)
                return;

            auto& inventory = machine.template get<sown::inventory>();
            inventory.unpick(machine, _x, _y);

            cook::recipes(machine, _x, _y + 1);

            move(machine, _x, _y - 1);
            _grace = globals::player::grace_unpick;
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(logic.tick(globals::player::rate) != 0)
                return;

            decay(machine);
            if(_food < globals::player::hungry::min_food)
            {
                tombstone(machine, _x, _y);
                return;
            }

            if(logic.tick(globals::player::fall) == 0)
            {
                if(_grace > 0)
                    _grace -= 1;
                else if(logic._pixels->get(_x, _y + 1).g == 0)
                    move(machine, _x, _y + 1);
            }

            if(logic._pixels->get(_x, _y + 1).b == globals::land::water[2])
                return;

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            {
                if(logic._pixels->get(_x - 1, _y).b == 0)
                    move(machine, _x - 1, _y);
                else if(logic._pixels->get(_x - 1, _y - 1).b == 0)
                    move(machine, _x - 1, _y - 1);
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            {
                if(logic._pixels->get(_x + 1, _y).b == 0)
                    move(machine, _x + 1, _y);
                else if(logic._pixels->get(_x + 1, _y - 1).b == 0)
                    move(machine, _x + 1, _y - 1);
            }
        }

        int _x = 0, _y = 0, _grace = 0, _food = 100, _temp = 100;
        std::shared_ptr<sf::RectangleShape> _shape, _temp_shape, _food_shape;
        std::shared_ptr<sf::Sprite> _help_sprite;
        sf::Texture _help_texture;
    };

    struct view : cl::enable<view>
    {
        void reset()
        {
            _view = std::make_unique<sf::View>();
        }

        template<typename t_machine>
        void update(t_machine& machine)
        {
            if(!_view)
                reset();

            if(!sf::Keyboard::isKeyPressed(sf::Keyboard::V))
            {
                auto& player = machine.template get<sown::player>();
                if(std::abs(player._x - _x) > globals::view::max_dist)
                {
                    if(player._x > _x)
                        _x = player._x - globals::view::max_dist;
                    else
                        _x = player._x + globals::view::max_dist;
                }

                if(std::abs(player._y - _y) > globals::view::max_dist)
                {
                    if(player._y > _y)
                        _y = player._y - globals::view::max_dist;
                    else
                        _y = player._y + globals::view::max_dist;
                }
                _view->setCenter(_x, _y - globals::view::hoffset);
                _view->setSize(globals::view::width, globals::view::height);
            }
            else
            {
                auto& logic = machine.template get<sown::logic>();
                _view->setCenter(logic._width / 2, logic._height / 2);
                _view->setSize(logic._width, logic._height);
            }

            auto& logic = machine.template get<sown::logic>();
            if(auto window = logic._window.lock(); window)
                window->setView(*_view);

            auto& player = machine.template get<sown::player>();
            player.update(machine);
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(logic.tick() == 0)
                update(machine);
        }

        std::unique_ptr<sf::View> _view;
        int _x = 0, _y = 0;
    };

    template<typename t_machine>
    void warm_player(t_machine& machine, globals::t_int x, globals::t_int y)
    {
        auto& music = machine.template get<sown::sound::music>();
        auto& player = machine.template get<sown::player>();
        if(std::abs(x - player._x) + std::abs(y - player._y) < 6)
            music._firequantity += 32;

        if(x == player._x && y == player._y)
        {
            player._temp += globals::fire::warm;
            if(player._temp > globals::player::max_temp)
                player._temp = globals::player::max_temp;
            player.update(machine);
        }
    }

    template<typename t_machine>
    void tombstone(t_machine& machine, globals::t_int x, globals::t_int y)
    {
        auto& logic = machine.template get<sown::logic>();
        for(int i = 4; i < 8; i += 1)
            logic._pixels->get(x, y - i) = globals::player::tomb;
        for(int i = -1; i < 2; i += 1)
            logic._pixels->get(x + i, y - 6) = globals::player::tomb;
        if(logic._grain > 600)
            logic._grain = 0;
        if(logic._grain == 600)
            reset(machine);

        machine.template prepare<sown::logic>();
    }
}

#endif
