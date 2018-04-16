#ifndef SOWN_AGENTS_HPP__GUARD_
#define SOWN_AGENTS_HPP__GUARD_

#include "clenche.hpp"
#include "property.hpp"

#include "snow.hpp"
#include "logic.hpp"

namespace sown
{
    struct planks : cl::property::entry<planks>
    {
        using int_type = globals::t_int;

        planks(int_type x, int_type y)
            : _x{x}, _y{y}
        { }

        template<typename t_machine>
        bool move(t_machine& machine, int_type x, int_type y)
        {
            auto& logic = machine.template get<sown::logic>();

            auto& here = logic._pixels->get(x, y);
            if(!here.g == 0)
                return false;
            here = globals::land::plank;

            logic._pixels->get(_x, _y).reset();
            _x = x;
            _y = y;
            return true;
        }

        template<typename t_logic>
        bool isplank(t_logic& logic, int_type xoff, int_type yoff)
        {
            auto& here = logic._pixels->get(_x + xoff, _y + yoff);
            return (here.g == globals::land::plank[1]);
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            auto& here = logic._pixels->get(_x, _y);
            if(true
                && here.r == globals::land::fire[0]
                && here.g == globals::land::fire[1]
                && here.a == globals::land::fire[3])
            {
                logic._pixels->get(_x, _y).reset();
                deleted = true;
                return;
            }

            if(logic._pixels->get(_x - 1, _y).b == globals::land::water[2]
                || logic._pixels->get(_x + 1, _y).b == globals::land::water[2]
                || logic._pixels->get(_x, _y - 1).b == globals::land::water[2])
            {
                logic._pixels->get(_x, _y) = globals::land::water;
                deleted = true;
                return;
            }

            if(logic.tick() != 0)
                return;

            int c = 0;
            for(int i = -1; i < 2; i += 1)
                for(int j = -1; j < 2; j += 1)
                    c += isplank(logic, i, j) ? 1 : 0;

            if(c > 2)
                return;

            if(!move(machine, _x, _y + 1))
            {
                if((_x + _y) % 2)
                {
                    if(logic._pixels->get(_x - 1, _y + 2).b == 0)
                        if(!move(machine, _x - 1, _y + 1))
                            move(machine, _x + 1, _y + 1);
                }
                else
                {
                    if(logic._pixels->get(_x + 1, _y + 2).b == 0)
                        if(!move(machine, _x + 1, _y + 1))
                            move(machine, _x - 1, _y + 1);
                }
            }
        }

        int_type _x, _y;
    };

    struct woods : cl::property::entry<woods>
    {
        using int_type = globals::t_int;

        woods(int_type x, int_type y, bool stable = false)
            : _x{x}, _y{y}, _stable{stable}
        { }

        template<typename t_machine>
        bool move(t_machine& machine, int_type x, int_type y)
        {
            auto& logic = machine.template get<sown::logic>();

            auto& here = logic._pixels->get(x, y);
            if(!here.g == 0)
                return false;
            here = globals::land::wood;

            logic._pixels->get(_x, _y).reset();
            _x = x;
            _y = y;
            return true;
        }

        template<typename t_logic>
        bool iswood(t_logic& logic, int_type xoff, int_type yoff,
            bool strict = false)
        {
            auto& here = logic._pixels->get(_x + xoff, _y + yoff);
            return (here.g == globals::land::wood[1]
                || (!strict && here.g == globals::land::leaf[1]));
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            auto& here = logic._pixels->get(_x, _y);
            if(true
                && here.r == globals::land::fire[0]
                && here.g == globals::land::fire[1]
                && here.a == globals::land::fire[3])
            {
                logic._pixels->get(_x, _y).reset();
                deleted = true;
                return;
            }

            if(logic._pixels->get(_x - 1, _y).b == globals::land::water[2]
                || logic._pixels->get(_x + 1, _y).b == globals::land::water[2]
                || logic._pixels->get(_x, _y - 1).b == globals::land::water[2])
            {
                logic._pixels->get(_x, _y) = globals::land::water;
                deleted = true;
                return;
            }

            if(_stable
                && here.r == 0
                && here.g == 0
                && here.b == 0)
                here = globals::land::wood;

            if(logic.tick() != 0)
                return;

            if(_stable)
            {
                int c = 0;
                c += iswood(logic, -1, -1, true) ? 1 : 0;
                c += iswood(logic, -1, 0, true) ? 1 : 0;
                c += iswood(logic, 1, -1, true) ? 1 : 0;
                c += iswood(logic, 0, -1, true) ? 1 : 0;
                c += iswood(logic, 1, 0, true) ? 1 : 0;

                if(c > 2)
                    logic._pixels->get(_x, _y).r = 0;

                if(false
                    || iswood(logic, -1, 0)
                    || iswood(logic, 1, 0)
                    || iswood(logic, 0, 1)
                    || iswood(logic, 1, 1)
                    || iswood(logic, -1, 1))
                    return;

                auto k = globals::trees::unstable;
                for(int i = -k; i < k; i += 1)
                    for(int j = -k; j < k; j += 1)
                        if(iswood(logic, i, j))
                            logic._pixels->get(_x + i, _y + j).g -= 1;

                _stable = false;
            }

            auto& top = logic._pixels->get(_x, _y - 1);
            if(top.r == 0xFF && top.g == 0xFF && top.b == 0xFF)
            {
                if(logic.tick(globals::trees::decay) == 0)
                {
                    deleted = true;
                    here.reset();
                    return;
                }
            }

            if(!move(machine, _x, _y + 1))
            {
                if((_x + _y) % 2)
                {
                    if(logic._pixels->get(_x - 1, _y + 2).b == 0)
                        if(!move(machine, _x - 1, _y + 1))
                            move(machine, _x + 1, _y + 1);
                }
                else
                {
                    if(logic._pixels->get(_x + 1, _y + 2).b == 0)
                        if(!move(machine, _x + 1, _y + 1))
                            move(machine, _x - 1, _y + 1);
                }
            }
        }

        int_type _x, _y;
        bool _stable;
    };

    struct leafs : cl::property::entry<leafs>
    {
        using int_type = globals::t_int;

        leafs(int_type x, int_type y, int_type stable)
            : _x{x}, _y{y}, _stable{stable}
        { }

        template<typename t_machine>
        bool move(t_machine& machine, int_type x, int_type y)
        {
            auto& logic = machine.template get<sown::logic>();

            auto& here = logic._pixels->get(x, y);
            if(!here.g == 0)
                return false;
            here = globals::land::leaf;

            logic._pixels->get(_x, _y).reset();
            _x = x;
            _y = y;
            return true;
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            auto& here = logic._pixels->get(_x, _y);
            if(_stable > 0
                && here.r == 0
                && here.g == 0
                && here.b == 0)
                here = globals::land::leaf;

            if(logic._pixels->get(_x - 1, _y).b == globals::land::water[2]
                || logic._pixels->get(_x + 1, _y).b == globals::land::water[2]
                || logic._pixels->get(_x, _y - 1).b == globals::land::water[2])
            {
                logic._pixels->get(_x, _y) = globals::land::water;
                deleted = true;
                return;
            }

            if(logic.tick() != 0)
                return;

            if(move(machine, _x, _y + 1))
            {
                _stable -= 1;
            }
            else if(_stable < 1
                && logic._pixels->get(_x, _y - 1).g == globals::land::leaf[1]
                && logic._pixels->get(_x, _y - 1).a == globals::land::leaf[3]
                && ((logic._pixels->get(_x, _y + 1).g == globals::land::leaf[1]
                && logic._pixels->get(_x, _y + 1).a == globals::land::leaf[3])
                || (logic._pixels->get(_x, _y + 1).g == globals::land::wood[1]
                && logic._pixels->get(_x, _y + 1).a == globals::land::wood[3]))
                )
                {
                    logic._pixels->get(_x, _y).reset();
                    deleted = true;
                    return;
                }
        }

        int_type _x, _y, _stable;
    };

    struct rocks : cl::property::entry<rocks>
    {
        using int_type = globals::t_int;

        rocks(int_type x, int_type y)
            : _x{x}, _y{y}
        { }

        template<typename t_machine>
        bool move(t_machine& machine, int_type x, int_type y)
        {
            auto& logic = machine.template get<sown::logic>();

            auto& here = logic._pixels->get(x, y);
            if(!here.g == 0)
                return false;
            here = globals::land::rock;

            logic._pixels->get(_x, _y).reset();
            if(logic._pixels->get(_x - 1, _y).b == globals::land::water[2]
                || logic._pixels->get(_x + 1, _y).b == globals::land::water[2])
                logic._pixels->get(_x, _y) = globals::land::water;

            _x = x;
            _y = y;
            return true;
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(logic.tick() != 0)
                return;

            if(!move(machine, _x, _y + 1)
                && logic._pixels->get(_x, _y + 1).b == globals::land::rock[2])
            {
                if((_x + _y) % 2)
                {
                    if(logic._pixels->get(_x - 1, _y + 2).g == 0)
                        if(!move(machine, _x - 1, _y + 1))
                            move(machine, _x + 1, _y + 1);
                }
                else
                {
                    if(logic._pixels->get(_x + 1, _y + 2).g == 0)
                        if(!move(machine, _x + 1, _y + 1))
                            move(machine, _x - 1, _y + 1);
                }
            }
        }

        int_type _x, _y;
    };

    struct fires : cl::property::entry<fires>
    {
        using int_type = globals::t_int;

        fires(int_type x, int_type y)
            : _x{x}, _y{y}
        {
            _life = globals::fire::life;
        }

        template<typename t_machine>
        bool move(t_machine& machine, int_type x, int_type y)
        {
            auto& logic = machine.template get<sown::logic>();

            auto& here = logic._pixels->get(x, y);
            if(!here.g == 0)
                return false;
            here = globals::land::fire;

            logic._pixels->get(_x, _y).reset();
            _x = x;
            _y = y;
            return true;
        }

        template<typename t_machine>
        void warm(t_machine& machine, int_type x, int_type y)
        {
            auto& logic = machine.template get<sown::logic>();
            auto& here = logic._pixels->get(x, y);

            warm_player(machine, x, y);

            if(true
                && here.r == globals::land::ice[0]
                && here.g == globals::land::ice[1]
                && here.b == globals::land::ice[2])
            {
                here = globals::land::grass;
                return;
            }

            if(true
                && here.r == globals::land::iced[0]
                && here.g == globals::land::iced[1]
                && here.b == globals::land::iced[2])
            {
                here = globals::land::water;
                return;
            }

            if(true
                && here.r == here.g
                && here.g == here.b
                && here.b == 0xFF)
            {
                auto& flakes = machine.template get<sown::snow::flakes>();
                for(auto& f : flakes.functors)
                {
                    if(f.deleted)
                        continue;

                    if(f._x != x || f._y != y)
                        continue;

                    f.deleted = true;
                    flakes.dirty = true;
                    here.reset();
                    return;
                }
            }
        }

        template<typename t_machine>
        void warm_zone(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            int i = 0, j = 0;
            while(true)
            {
                auto& here = logic._pixels->get(_x + i, _y + j);
                if(here.r == globals::land::coal[0])
                    break;
                j += 1;

                if(j > globals::fire::range)
                    break;
            }
            j += 1;

            while(i < globals::fire::range)
            {
                warm(machine, _x - i, _y + j);
                warm(machine, _x + i, _y + j);
                for(int k = 1; logic._pixels->get(_x + i, _y + j + k).b ==
                    globals::land::ice[2]; k += 1)
                    warm(machine, _x + i, _y + j + k);
                for(int k = 1; logic._pixels->get(_x - i, _y + j + k).b ==
                    globals::land::ice[2]; k += 1)
                    warm(machine, _x - i, _y + j + k);

                for(int k = 1; k < globals::fire::range; k += 1)
                {
                    if(k == 1 && i < 2)
                        k += 1;

                    auto& here = logic._pixels->get(_x + i, _y + j - k);
                    if(here.a >= 0xF0 && here.b < 0xF0 && here.r >= 0x20)
                        break;

                    warm(machine, _x + i, _y + j - k);
                }
                for(int k = 1; k < globals::fire::range; k += 1)
                {
                    if(k == 1 && i < 2)
                        k += 1;

                    auto& here = logic._pixels->get(_x - i, _y + j - k);
                    if(here.a >= 0xF0 && here.b < 0xF0 && here.r >= 0x20)
                        break;

                    warm(machine, _x - i, _y + j - k);
                }
                i += 1;
            }
        }

        template<typename t_machine, typename t_logic>
        void burn(t_machine& machine, t_logic& logic,
            int_type xoff, int_type yoff)
        {
            auto& here = logic._pixels->get(_x + xoff, _y + yoff);
            if(here.g == globals::land::wood[1]
                && here.a == globals::land::wood[3])
            {
                auto& fires = machine.template get<sown::fires>();
                fires.add(_x + xoff, _y + yoff);
                here = globals::land::fire;
            }
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(logic._pixels->get(_x, _y + 1).r == globals::land::coal[0])
            {
                if(logic._pixels->get(_x, _y).r == 0)
                {
                    logic._pixels->get(_x, _y) = globals::land::fire;
                    logic._pixels->get(_x, _y).a = 0x50;
                }
                return;
            }

            if(logic.tick(4))
                return;

            burn(machine, logic, 0, 1);
            burn(machine, logic, 0, 0);
            burn(machine, logic, 0, -1);
            burn(machine, logic, 1, 1);
            burn(machine, logic, 1, 0);
            burn(machine, logic, 1, -1);
            burn(machine, logic, -1, 1);
            burn(machine, logic, -1, 0);
            burn(machine, logic, -1, -1);

            if(true
                && logic._pixels->get(_x, _y - 1).b == 0
                && logic._pixels->get(_x, _y + 1).b == 0)
            {
                _life -= 1;

                if(_life < 0)
                {
                    warm_zone(machine);

                    logic._pixels->get(_x, _y).reset();
                    deleted = true;
                    return;
                }
            }
            move(machine, _x, _y - 1);
        }

        int_type _x, _y;
        int_type _life;
    };

    struct coals : cl::property::entry<coals>
    {
        using int_type = globals::t_int;

        coals(int_type x, int_type y, bool burning = false)
            : _x{x}, _y{y}, _burning{burning}
        { }

        template<typename t_machine>
        bool move(t_machine& machine, int_type x, int_type y)
        {
            auto& logic = machine.template get<sown::logic>();

            auto& here = logic._pixels->get(x, y);
            if(!here.g == 0)
                return false;
            here = globals::land::coal;

            logic._pixels->get(_x, _y).reset();
            if(logic._pixels->get(_x - 1, _y).b == globals::land::water[2]
                || logic._pixels->get(_x + 1, _y).b == globals::land::water[2])
                logic._pixels->get(_x, _y) = globals::land::water;

            _x = x;
            _y = y;
            return true;
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();

            if(logic.tick() != 0)
                return;

            if(true
                && logic._pixels->get(_x - 1, _y).g == globals::land::rock[1]
                && logic._pixels->get(_x + 1, _y).g == globals::land::rock[1])
            {
                if(logic._pixels->get(_x, _y - 1).r == globals::land::fire[0])
                {
                    if(logic.tick(globals::fire::rate) == 0)
                    {
                        auto& fires = machine.template get<sown::fires>();
                        fires.add(_x, _y - 2);
                        _burning = false;
                    }
                }
                else if(logic.tick(globals::fire::start) == 0 || _burning)
                {
                    auto& fires = machine.template get<sown::fires>();
                    fires.add(_x, _y - 1);
                }
            }
            else if(logic._pixels->get(_x, _y - 1).r == globals::land::fire[0])
            {
                logic._pixels->get(_x, _y).reset();
            }
            else if(logic._pixels->get(_x, _y).r != globals::land::coal[0])
            {
                logic._pixels->get(_x, _y) = globals::land::coal;
            }

            if(!move(machine, _x, _y + 1))
            {
                if((_x + _y) % 2)
                {
                    if(logic._pixels->get(_x - 1, _y + 2).g == 0)
                        if(!move(machine, _x - 1, _y + 1))
                            move(machine, _x + 1, _y + 1);
                }
                else
                {
                    if(logic._pixels->get(_x + 1, _y + 2).g == 0)
                        if(!move(machine, _x + 1, _y + 1))
                            move(machine, _x - 1, _y + 1);
                }
            }
        }

        int_type _x, _y;
        bool _burning;
    };
}

#endif
