#ifndef SOWN_POLL_HPP__GUARD_
#define SOWN_POLL_HPP__GUARD_

#include <memory>

#include "clenche.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "logic.hpp"
#include "player.hpp"

namespace sown::details
{
    struct mouse_type
    {
        void update(sf::Event::MouseMoveEvent& entry)
        {
            _x = entry.x;
            _y = entry.y;
        }

        void update(sf::Event::MouseWheelScrollEvent& entry)
        {
            _wheel = entry.wheel;
            _delta = entry.delta;
            _x = entry.x;
            _y = entry.y;

            _sid += 1;
        }

        size_t _sid = 0; // scroll event id
        float _delta = 0.0;
        bool _pressed = false;
        sf::Mouse::Wheel _wheel = sf::Mouse::Wheel::HorizontalWheel;
        globals::t_int _x = 0, _y = 0;
    };
}

namespace sown
{
    struct poll : cl::enable<poll>
    {
        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(auto window = logic._window.lock(); window && window->isOpen())
            {
                sf::Event event;
                while(window->pollEvent(event))
                {
                    if(false)
                        ;
                    else if(event.type == sf::Event::Closed)
                        window->close();
                    else if(event.type == sf::Event::GainedFocus)
                        _focus = true;
                    else if(event.type == sf::Event::LostFocus)
                        _focus = false;
                    else if(event.type == sf::Event::Resized)
                        logic.resize(event.size.width, event.size.height);
                    else if(event.type == sf::Event::MouseMoved)
                        _mouse.update(event.mouseMove);
                    else if(event.type == sf::Event::MouseButtonPressed)
                        _mouse._pressed = true;
                    else if(event.type == sf::Event::MouseButtonReleased)
                        _mouse._pressed = false;
                    else if(event.type == sf::Event::MouseWheelScrolled)
                        _mouse.update(event.mouseWheelScroll);
                    else if(event.type == sf::Event::KeyPressed)
                    {
                        auto& player = machine.template get<sown::player>();
                        switch(event.key.code)
                        {
                            case sf::Keyboard::Tab:
                                player.next(machine); break;
                            case sf::Keyboard::Up:
                                player.jump(machine); break;
                            case sf::Keyboard::Down:
                                player.pick(machine); break;
                            case sf::Keyboard::Space:
                                player.unpick(machine); break;
                            case sf::Keyboard::R:
                                reset(machine); return; break;
                            default:
                                ;
                        }
                    }
                }
            }
            else
                machine.finish();

            if(!_focus)
                machine.template prepare<poll>();
        }

        details::mouse_type _mouse;
        bool _focus = true;
    };
}


#endif
