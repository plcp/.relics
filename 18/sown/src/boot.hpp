#ifndef SOWN_BOOT_HPP__GUARD_
#define SOWN_BOOT_HPP__GUARD_

#include <memory>

#include "clenche.hpp"

#include <SFML/Window.hpp>

#include "logic.hpp"
#include "globals.hpp"

namespace sown
{
    struct boot : cl::enable<boot>
    {
        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            _window = std::make_shared<sf::RenderWindow>();
            _window->create(
                sf::VideoMode::getDesktopMode(),
                globals::window_name,
                sf::Style::Fullscreen);

            auto size = _window->getSize();
            _window->setFramerateLimit(globals::target_framerate + 1);

            auto& logic = machine.template get<sown::logic>();
            logic.reset(size.x, size.y, _window);
        }

        std::shared_ptr<sf::RenderWindow> _window;
    };
}

#endif
