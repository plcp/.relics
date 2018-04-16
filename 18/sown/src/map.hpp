#ifndef SOWN_MAP_HPP__GUARD_
#define SOWN_MAP_HPP__GUARD_

#include <memory>

#include <SFML/Window.hpp>

#include "globals.hpp"

namespace sown::map
{
    namespace details
    {
        struct rgba
        {
            sf::Uint8 r = 0, g = 0, b = 0, a = 0;

            void operator=(const int colors[])
            {
                r = colors[0];
                g = colors[1];
                b = colors[2];
                a = colors[3];
            }

            void reset()
            {
                r = 0;
                g = 0;
                b = 0;
                a = 0;
            }
        };

        struct xy
        {
            globals::t_int x = 0, y = 0;

            void reset()
            {
                x = 0;
                y = 0;
            }
        };

        struct field
        {
            xy dir, pos;
            globals::t_int cnt = 0;

            void reset()
            {
                dir.reset();
                pos.reset();
                cnt = 0;
            }
        };

        template<typename t_data, typename t_raw>
        struct map_type
        {
            map_type(const globals::t_int& width, const globals::t_int& height)
                : _grid{std::make_unique<t_data[]>(width * height)},
                  _width{width}, _height{height}
            { }

            void reset()
            {
                for(size_t i = 0; i < size_t(_width) * size_t(_height); ++i)
                    _grid[i].reset();
            }

            bool isoutside(const globals::t_int& x, const globals::t_int& y)
            {
                return (x < 0 || x >= _width || y < 0 || y >= _height);
            }

            const auto& cget(const globals::t_int& x, const globals::t_int& y)
            {
                return get(x, y);
            }

            auto& get(const globals::t_int& x, const globals::t_int& y)
            {
                if(isoutside(x, y))
                {
                    _outofbound.reset();
                    return _outofbound;
                }
                return _grid[x + y * _width];
            }

            t_raw* raw()
            {
                return reinterpret_cast<t_raw*>(_grid.get());
            }

            std::unique_ptr<t_data[]> _grid;
            globals::t_int _width, _height;

        private:
            t_data _outofbound;
        };
    }

    using field_map = details::map_type<details::field, globals::t_int>;
    using rgba_map = details::map_type<details::rgba, sf::Uint8>;
}

#endif
