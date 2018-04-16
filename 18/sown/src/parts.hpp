#ifndef SOWN_PARTS_HPP__GUARD_
#define SOWN_PARTS_HPP__GUARD_

#include "clenche.hpp"

#include <SFML/Graphics.hpp>

#include "globals.hpp"

namespace sown
{
    template<size_t t_text_size = 20, bool t_bold = true, size_t t_margin = 2>
    struct framerate : cl::enable<framerate<t_text_size, t_bold, t_margin>>
    {
        template<typename t_machine>
        void reset(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();

            _text = std::make_shared<sf::Text>("--",
                logic._fonts.get(globals::default_font));

            if constexpr(t_margin > 0)
                _text->setPosition(t_margin + 4, t_margin);

            if constexpr(t_text_size > 0)
                _text->setCharacterSize(t_text_size);

            if constexpr(t_bold)
                _text->setStyle(sf::Text::Bold);

            auto& drawables = machine.template get<sown::drawables>();
            drawables.add(_text);
        }

        void update(size_t frametime)
        {
            _framerate *= 3;
            _framerate += 1000000 / frametime;
            _framerate /= 4;
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            if(!_text)
                reset(machine);

            auto& logic = machine.template get<sown::logic>();
            switch(logic.tick())
            {
                case 0:
                    update(logic._frametime);
                    break;
                case 1:
                    _text->setString(std::to_string(_framerate));
                    break;
                default:
                    ;
            }
        }

        std::shared_ptr<sf::Text> _text;
        size_t _framerate = globals::target_framerate;
    };
}

#endif
