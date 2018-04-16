#ifndef SOWN_LOGIC_HPP__GUARD_
#define SOWN_LOGIC_HPP__GUARD_

#include <memory>
#include <cstdint>

#include "clenche.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "map.hpp"
#include "fonts.hpp"

namespace sown
{
    struct logic : cl::enable<logic>
    {
        void resize(const size_t& width, const size_t& height)
        {
            _width = width;
            _height = height;

            auto swidth = scale_to_grid(width);
            auto sheight = scale_to_grid(height);
            _fields = std::make_unique<sown::map::field_map>(swidth, sheight);
            _pixels = std::make_unique<sown::map::rgba_map>(width, height);

            _texture = std::make_shared<sf::Texture>();
            _texture->create(width, height);
        }

        void reset(const size_t& width, const size_t& height,
            std::shared_ptr<sf::RenderWindow>& window)
        {
            _grain = 0;
            _window = window;
            resize(width, height);
        }

        template<typename t_machine>
        void operator()(t_machine&)
        {
            _grain += 1;

            _frametime *= 15;
            _frametime += _frameclock.restart().asMicroseconds();
            _frametime /= 16;

            if(_frametime < globals::target_frametime)
                sf::sleep(
                    sf::microseconds(globals::target_frametime - _frametime));
        }

        uint16_t tick(uint16_t time = 32)
        {
            return _grain % time;
        }

        std::unique_ptr<sown::map::field_map> _fields;
        std::unique_ptr<sown::map::rgba_map> _pixels;
        std::weak_ptr<sf::RenderWindow> _window;
        std::shared_ptr<sf::Texture> _texture;
        sf::Clock _frameclock;
        size_t _frametime = 0;
        size_t _width, _height;
        sown::fonts _fonts;
        size_t _grain;
    };
}

#endif
