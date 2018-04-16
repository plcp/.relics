#ifndef SOWN_TIMER_HPP__GUARD_
#define SOWN_TIMER_HPP__GUARD_

#include "clenche.hpp"

#include <SFML/Graphics.hpp>

#include "globals.hpp"

namespace sown
{
    struct timer : cl::enable<timer>
    {
        template<typename t_machine>
        void reset(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            _text = std::make_shared<sf::Text>("Help is on the way...",
                logic._fonts.get(globals::default_font));

            _text->setCharacterSize(12);
            _quantity = 0;
            _started = false;
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            if(!_text)
                reset(machine);

            auto& logic = machine.template get<sown::logic>();

            _view.setCenter(logic._width / 2, logic._height / 2);
            _view.setSize(logic._width, logic._height);

            if(auto window = logic._window.lock(); window)
            {
                auto& view = machine.template get<sown::view>();
                if(!view._view)
                    return;

                if(!_started && logic._grain > globals::starttime
                    * globals::target_framerate)
                {
                    _started = true;
                    _quantity += get_difficulty(machine) / 4;
                    _quantity *= globals::player::hungry::single_drop;
                    _quantity += globals::player::max_food;
                    _quantity *= globals::player::hungry::rate;
                }

                if(_started)
                {
                    auto days = _quantity / (24 * 60 * 25);
                    auto hours = (_quantity - days * 24 * 60 * 25) / (60 * 25);
                    auto minutes = (_quantity - days * 24 * 60 * 25
                        - hours * 60 * 25) / 25;

                    _text->setString("Rescue in "
                        + std::to_string(days)
                        + " day" + (days > 1 ? "s" : "") + ", "
                        + std::to_string(hours)
                        + " hour" + (hours > 1 ? "s" : "") + " and "
                        + std::to_string(minutes)
                        + " minute" + (minutes > 1 ? "s" : "") + "...");
                    _quantity -= 1;
                }

                auto xpos = logic._width - _text->getLocalBounds().width;
                _text->setPosition(sf::Vector2f(xpos - 16, 8));

                window->setView(_view);
                window->draw(*_text);

                window->setView(*view._view);
            }
        }

        std::shared_ptr<sf::Text> _text;
        sf::View _view;
        bool _started = false;
        int _quantity = 0;
    };
}

#endif
