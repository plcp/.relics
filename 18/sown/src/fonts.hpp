#ifndef SOWN_FONTS_HPP__GUARD_
#define SOWN_FONTS_HPP__GUARD_

#include <map>
#include <exception>

#include <SFML/Graphics.hpp>

namespace sown
{
    struct fonts
    {
        fonts()
        {
            get("Inconsolata.otf");
        }

        sf::Font& get(std::string name)
        {
            auto search = _fonts.find(name);
            if(search != _fonts.end())
                return search->second;

            auto entry = std::make_pair(name, sf::Font());
            if(entry.second.loadFromFile(globals::fonts_directory + name))
            {
                _fonts.emplace(entry);
                return get(name);
            }

            search = _fonts.find(globals::default_font);
            if(search != _fonts.end())
                return search->second;

            throw std::runtime_error("Default font not found.");
        }

        std::map<std::string, sf::Font> _fonts;
    };
}


#endif
