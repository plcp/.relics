#ifndef SOWN_DRAW_HPP__GUARD_
#define SOWN_DRAW_HPP__GUARD_

#include "clenche.hpp"
#include "property.hpp"

#include <SFML/Graphics.hpp>

namespace sown
{
    struct clear : cl::enable<clear>
    {
        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(auto window = logic._window.lock(); window)
            {
                window->clear(sf::Color::Black);
            }
        }
    };

    struct background : cl::enable<background>
    {
        template<typename t_machine, typename t_functor>
        void static process(t_machine& machine, t_functor&)
        {
            auto& logic = machine.template get<sown::logic>();

            if(auto window = logic._window.lock(); window)
            {
                logic._texture->update(logic._pixels->raw());
                sf::Sprite sprite(*logic._texture);
                window->draw(sprite);
            }
        }
    };

    struct display : cl::enable<display>
    {
        template<typename t_machine, typename t_functor>
        void static process(t_machine& machine, t_functor& self)
        {
            auto& logic = machine.template get<sown::logic>();
            if(auto window = logic._window.lock(); window)
                window->display();

            if(_dirty > globals::cleanup_threshold)
            {
                self.clean();
                _dirty = 0;
            }
        }
        static size_t _dirty;
    };
    size_t display::_dirty = 0;

    struct drawables : cl::property::entry<drawables>
    {
        using before = background;
        using after = display;

        drawables(std::weak_ptr<sf::Drawable> drawable)
            : _drawable{drawable}
        { }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(auto drawable = _drawable.lock(); drawable)
            {
                if(auto window = logic._window.lock(); window)
                    window->draw(*drawable);
            }
            else
            {
                display::_dirty += 1;
                deleted = true;
            }
        }

        std::weak_ptr<sf::Drawable> _drawable;
    };
}

#endif
