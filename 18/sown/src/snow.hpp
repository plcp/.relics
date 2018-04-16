#ifndef SOWN_SNOW_HPP__GUARD_
#define SOWN_SNOW_HPP__GUARD_

#include <memory>
#include <random>

#include "clenche.hpp"
#include "property.hpp"

#include "sound.hpp"
#include "globals.hpp"

namespace sown::snow
{
    struct spawn_bars : cl::enable<spawn_bars>
    {
        template<typename t_machine, typename t_bars>
        static void process(t_machine&, t_bars& bars)
        {
            if(bars.size() < globals::view::width)
                for(int i = 0; i < globals::view::width; i += 1)
                    bars.add(i);
        }
    };

    struct bars : cl::property::entry<bars>
    {
        using int_type = globals::t_int;

        using before = spawn_bars;

        bars(int_type rx)
            : _rx{rx}
        { }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            if(!_shape)
            {
                _shape = std::make_shared<sf::RectangleShape>();

                auto& drawables = machine.template get<sown::drawables>();
                drawables.add(_shape);
            }
            _shape->setSize(sf::Vector2f(0, 0));

            auto& logic = machine.template get<sown::logic>();
            if(auto window = logic._window.lock(); window)
            {
                auto center = window->getView().getCenter();

                auto x = center.x - globals::view::width / 2 + _rx;
                auto y = center.y - globals::view::height / 2;

                int i, cnt;
                for(cnt = 0, i = 0; i < globals::view::height; i += 1)
                {
                    if(logic._pixels->get(x, y + i).r == 0xFF)
                        cnt += 1;
                    else if (cnt > 0)
                        cnt -= 1;

                    if(cnt >= 5)
                        break;
                }

                if(cnt < 5)
                    return;
                else
                    i -= 5;

                int j;
                for(cnt = 0, j = 0; j < globals::view::height; j += 1)
                {
                    if(logic._pixels->get(x, y + i + j).r == 0xFF)
                        cnt = 0;
                    else
                        cnt += 1;

                    if(true
                        && logic._pixels->get(x, y + i + j).g != 0xFF
                        && logic._pixels->get(x, y + i + j).g != 0x00)
                    {
                        j += 8 - cnt;
                        break;
                    }

                    if(cnt >= 8)
                        break;
                }

                j -= 8;
                _shape->setSize(sf::Vector2f(1, j));
                _shape->setPosition(sf::Vector2f(x, y + i + 1));
            }
        }

        std::shared_ptr<sf::RectangleShape> _shape;
        int_type _rx;
    };

    struct flakes : cl::property::entry<flakes>
    {
        using int_type = globals::t_int;
        using uint_type = globals::t_uint;

        flakes(int_type x, int_type y, int_type vx, int_type vy,
            uint_type _age)
            : _x{x}, _y{y}, _vx{vx}, _vy{vy}, _age{_age}
        {
            if(_age % globals::snow::melting)
                _melting = true;
        }

        template<typename t_grid>
        void get_field(t_grid&& grid, const int16_t xoff, const int16_t yoff)
        {
            int16_t gx = scale_to_grid(_x) + xoff;
            int16_t gy = scale_to_grid(_y) + yoff;
            if(grid.isoutside(gx, gy))
                return;

            auto coh = globals::snow::cohesion;
            auto dis = globals::snow::dispertion;
            auto gsize = globals::grid_size;
            auto velocity = globals::snow::velocity;

            auto& field = grid.get(gx, gy);
            if(field.cnt > 1)
            {
                int16_t rx = _x - gx * gsize - gsize / 2;
                int16_t ry = _y - gy * gsize - gsize / 2;

                int16_t cx = (_x * field.cnt) - field.pos.x;
                int16_t cy = (_y * field.cnt) - field.pos.y;

                int16_t ax = field.dir.x;
                int16_t ay = field.dir.y;

                int16_t dx = (ax - cx / coh) / field.cnt + (rx / dis) / gsize;
                int16_t dy = (ay - cy / coh) / field.cnt + (ry / dis) / gsize;

                int16_t max = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
                if(max < 1)
                    return;

                dx /= max;
                dy /= max;

                if(dx > 0 && _vx + dx < velocity)
                    _vx += dx;
                else if (dx < 0 && _vx + dx > -velocity)
                    _vx += dx;

                if(_age % 2)
                {
                    if(dy > 0 && _vy + dy < velocity)
                        _vy += dy;
                    else if (dy < 0 && _vy + dy > -velocity / 2)
                        _vy += dy;
                }
            }
            else if(_vy <= 0 && _age % 4 == 0)
                _vy -= 1;
        }

        template<typename t_machine>
        void behavior(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(false
                || logic._pixels->get(_x, _y + 1).r == globals::land::grass[0]
                || (true
                    && logic._pixels->get(_x, _y + 1).r == 0xFF
                    && logic._pixels->get(_x, _y - 1).r == 0xFF))
            {
                _windy = true;
                if(_vy > 1)
                    _vy = 1;
            }
            if(_vy == 0)
                _vy = 1;

            if(_vy < 0
                && logic._pixels->get(_x - 1, _y).g == 0xFF
                && logic._pixels->get(_x + 1, _y).g == 0xFF
                && logic._pixels->get(_x, _y - 1).g == 0xFF
                && logic._pixels->get(_x, _y - 16).g == 0xFF
                && logic._pixels->get(_x, _y + 1).g == 0xFF)
                _vy /= 2;

            if(_age % 2)
            {
                if(logic._pixels->get(_x + 1, _y + 1).g == 0)
                    _vx += 1;
                else if(logic._pixels->get(_x - 1, _y + 1).g == 0)
                    _vx -= 1;
            }
            else
            {
                if(logic._pixels->get(_x - 1, _y + 1).g == 0)
                    _vx -= 1;
                else if(logic._pixels->get(_x + 1, _y + 1).g == 0)
                    _vx += 1;
            }

            auto& grid = *logic._fields;
            switch(_age % 9)
            {
                case 0: get_field(grid, 1, 1);    break;
                case 1: get_field(grid, 0, 1);    break;
                case 2: get_field(grid, -1, 1);   break;
                case 3: get_field(grid, 1, -1);   break;
                case 4: get_field(grid, 0, -1);   break;
                case 5: get_field(grid, -1, -1);  break;
                case 6: get_field(grid, 1, 0);    break;
                case 7: get_field(grid, 0, 0);    break;
                case 8: get_field(grid, -1, 0);   break;
            }

            if(!_windy)
            {
                switch(_age % 32)
                {
                    case 0:
                        _vx += -1;
                        break;
                    case 16:
                        _vx += 2;
                        break;
                    case 3:
                        [[fallthrough]];
                    case 9:
                        [[fallthrough]];
                    case 15:
                        [[fallthrough]];
                    case 19:
                        [[fallthrough]];
                    case 25:
                        [[fallthrough]];
                    case 31:
                        _vx /= 2;
                        break;
                    default:
                        break;
                }
                if(std::abs(_vy) > 3)
                    _vy /= 2;
            }

        }

        template<typename t_machine, typename t_map>
        void update(t_machine& machine, t_map& pixels)
        {
            behavior(machine);

            auto& h = pixels->get(_x, _y);
            if(h.r == 0xFF && h.g == 0xFF && h.b == 0xFF)
                pixels->get(_x, _y).reset();
            else if(_y > 0 && !h.r && !h.g && !h.b)
                _y -= 1;

            auto& w = pixels->get(_x + _vx, _y + _vy);
            if(!w.r && !w.g && !w.b)
            {
                if(int sign = _vx < 0 ? -1 : 1; std::abs(_vx) > 0)
                {
                    for(int i = 0; i < std::abs(_vx); i += 1)
                    {
                        auto& z = pixels->get(_x + sign, _y);
                        if(!z.r && !z.g && !z.b)
                            _x += sign;
                    }
                }

                if(_vy > 0 || _age % 2 == 0)
                {
                    if(int sign = _vy < 0 ? -1 : 1; std::abs(_vy) > 0)
                    {
                        for(int i = 0; i < std::abs(_vy); i += 1)
                        {
                            auto& z = pixels->get(_x, _y + sign);
                            if(!z.r && !z.g && !z.b)
                                _y += sign;
                        }
                    }
                }

            } else {
                auto& z = pixels->get(_x, _y + 1);
                if(!z.r && !z.g && !z.b)
                {
                    _y += 1;
                }
            }

            auto& k = pixels->get(_x, _y);
            if(!k.r && !k.g && !k.b)
                k = {0xFF, 0xFF, 0xFF, 0xFF};
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(logic.tick(globals::snow::slow) != 0)
                return;

            update(machine, logic._pixels);
            if(logic._pixels->get(_x, _y + 1).b == globals::land::water[2])
            {
                logic._pixels->get(_x, _y + 1) = globals::land::iced;
                logic._pixels->get(_x, _y).reset();
                deleted = true;
            }

            if((_melting
                && _age > globals::snow::melttime * logic._height / 16)
                || _x < 4 || _x > int_type(logic._width) - 4)
            {
                logic._pixels->get(_x, _y).reset();
                deleted = true;
                return;
            }

            _age += 1;
            if(_age > globals::snow::lifetime)
            {
                if(logic._pixels->isoutside(_x, _y))
                    deleted = true;
            }
        }

        int_type _x, _y, _vx, _vy;
        uint_type _age;
        bool _windy = false, _melting = false;
    };

    struct wind : cl::enable<wind>
    {
        using roll_type = std::uniform_int_distribution<globals::t_int>;

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            auto& flakes = machine.template get<sown::snow::flakes>();
            if(flakes.size() < logic._width * globals::snow::blown_average)
                return;

            logic._fields->reset();

            std::mt19937 roll(logic._grain);
            roll_type lr(-2, 1);

            if(logic.tick(globals::snow::blown_period) == 0)
            {
                auto vx = lr(roll) * globals::snow::blown_x;
                while(vx == 0)
                    vx = lr(roll) * globals::snow::blown_x;

                auto vy = -std::abs(lr(roll)) * globals::snow::blown_y;
                for(auto& f : flakes.functors)
                {
                    f._vx = vx;
                    if(f._windy)
                    {
                        f._vy = vy;
                    }
                }
            }

            for(auto& f : flakes.functors)
            {
                auto& field = logic._fields->get(
                    scale_to_grid(f._x), scale_to_grid(f._y));

                field.dir.x += f._vx;
                field.dir.y += f._vy;
                field.pos.x += f._x;
                field.pos.y += f._y;
                field.cnt += 1;
            }
        }
    };

    struct spawn : cl::enable<spawn>
    {
        using roll_type = std::uniform_int_distribution<globals::t_int>;
        using uroll_type = std::uniform_int_distribution<globals::t_uint>;

        template<typename t_machine>
        void reset(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            _width = logic._width;
            _height = logic._height;

            globals::snow::rate = 1;
            globals::snow::irate = 1;

            sown::sound::intensity(machine, 1, true);

            _roll = std::make_unique<std::mt19937>(logic._grain);
            _x = std::make_unique<roll_type>(0, _width - 1);
            _y = std::make_unique<roll_type>(-256, -2);
            _vx = std::make_unique<roll_type>(-2, 2);
            _vy = std::make_unique<roll_type>(1, 2);
            _age = std::make_unique<uroll_type>(0, 64);
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(logic._width != _width || logic._height != _height || !_roll)
                reset(machine);

            if(logic.tick(globals::snow::growth) == 0)
            {
                if(_period)
                {
                    globals::snow::irate += 1;
                    sown::sound::intensity(machine, -1);
                    if(globals::snow::irate == globals::snow::period)
                        _period = false;
                }
                else
                {
                    globals::snow::irate -= 1;
                    sown::sound::intensity(machine, 1);
                    if(globals::snow::irate == 1)
                        _period = true;
                }
                globals::snow::rate += 1;
                sown::sound::intensity(machine, 2);
            }

            auto& flakes = machine.template get<sown::snow::flakes>();
            if(flakes.size() < globals::snow::max_quantity)
                if(logic.tick(globals::snow::irate * 2) == 0)
                    for(int i = 0; i < globals::snow::rate; i += 1)
                        flakes.add(
                            (*_x)(*_roll),
                            (*_y)(*_roll),
                            (*_vx)(*_roll),
                            (*_vy)(*_roll),
                            (*_age)(*_roll));

            if(logic.tick(1024) == 0)
                flakes.dirty = true;
            if(logic.tick() == 0)
                flakes.clean();
        }

        std::unique_ptr<std::mt19937> _roll;
        std::unique_ptr<roll_type> _vx, _vy, _x, _y;
        std::unique_ptr<uroll_type> _age;
        size_t _width, _height;
        bool _period = true;
    };
}

#endif
