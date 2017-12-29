#include <random>
#include <memory>
#include <cstdint>
#include <iostream>

#include "clenche.hpp"
#include "sequence.hpp"
#include "property.hpp"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>


#ifndef FEW
    constexpr int coh = 2;
    constexpr int dis = 2;
    constexpr int gsize = 4;
    constexpr int velocity = 8;
    constexpr int quantity = 1000000;
    constexpr int mouse_weight = 16;
    constexpr int target_framerate = 100;
    constexpr uint16_t max_age = 120 * target_framerate;
#else
    constexpr float coh = 0.5;
    constexpr float dis = 0.1;
    constexpr int gsize = 64;
    constexpr int velocity = 6;
    constexpr int quantity = 1000;
    constexpr int mouse_weight = 32;
    constexpr int target_framerate = 80;
    constexpr uint16_t max_age = 1000;
#endif


struct poll;
struct agents;

struct rgba
{
    sf::Uint8 r = 0, g = 0, b = 0, a = 0xFF;

    void reset()
    {
        r = 0;
        g = 0;
        b = 0;
        a = 0xFF;
    }
};

struct xy
{
    int16_t x = 0, y = 0;

    void reset()
    {
        x = 0;
        y = 0;
    }
};

constexpr int16_t gdiv(int16_t g)
{
    return int16_t(g - gsize / 2) / gsize;
}

struct boid
{
    xy dir, pos;
    int16_t cnt = 0;

    void reset()
    {
        dir.reset();
        pos.reset();
        cnt = 0;
    }
};

template<typename t_dtype, typename t_rawtype>
struct map_type
{
    map_type(int16_t width, int16_t height)
        : pixels{std::make_unique<t_dtype[]>(width * height)},
          width{width}, height{height}
    { }

    void reset()
    {
        for(int32_t i = 0; i < width * height; ++i)
            pixels[i].reset();
    }

    bool inbound(const int32_t x, const int32_t y) const
    {
        if(x < 0 || x >= width || y < 0 || y >= height)
            return false;
        return true;
    }

    const auto& cget(const int32_t x, const int32_t y) const
    {
        if(!inbound(x, y))
            return _outofbound;
        return pixels[x + y * width];
    }

    auto& get(const int32_t x, const int32_t y)
    {
        if(!inbound(x, y))
            return _outofbound;
        return pixels[x + y * width];
    }

    auto raw()
    {
        return reinterpret_cast<t_rawtype*>(pixels.get());
    }

    std::unique_ptr<t_dtype[]> pixels;
    t_dtype _outofbound;
    int16_t width, height;
};

struct logic : cl::enable<logic>
{
    template<typename t_machine>
    void operator()(t_machine& machine)
    {
        window.create(sf::VideoMode::getDesktopMode(), "cars",
            sf::Style::Fullscreen);
        window.setFramerateLimit(target_framerate + 10);

        auto& _poll = machine.template get<poll>();
        auto size = window.getSize();
        _poll.height = size.y;
        _poll.width = size.x;
        _poll.focus = true;

        grid = std::make_unique<map_type<boid, int16_t>>(
            gdiv(size.x), gdiv(size.y));

        map = std::make_unique<map_type<rgba, sf::Uint8>>(size.x, size.y);
        texture.create(size.x, size.y);
    }
    std::unique_ptr<map_type<rgba, sf::Uint8>> map;
    std::unique_ptr<map_type<boid, int16_t>> grid;
    sf::RenderWindow window;
    sf::Texture texture;
};

struct poll : cl::enable<poll>
{
    template<typename t_machine>
    void operator()(t_machine& machine)
    {
        auto& window = machine.template get<logic>().window;

        if(!window.isOpen())
        {
            machine.finish();
            return;
        }

        sf::Time frametime = clock.restart();
        framerate = (framerate + 1000000 / frametime.asMicroseconds()) / 2;

        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            else if(event.type == sf::Event::GainedFocus)
                focus = true;
            else if(event.type == sf::Event::LostFocus)
                focus = false;
            else if(event.type == sf::Event::Resized)
            {
                width = event.size.width;
                height = event.size.height;

                auto& texture = machine.template get<logic>().texture;
                auto& grid = machine.template get<logic>().grid;
                auto& map = machine.template get<logic>().map;

                grid = std::make_unique<map_type<boid, int16_t>>(
                    gdiv(width), gdiv(height));

                map = std::make_unique<
                    map_type<rgba, sf::Uint8>>(width, height);
                texture.create(width, height);
            }
            else if(event.type == sf::Event::MouseMoved)
            {
                xmouse = event.mouseMove.x;
                ymouse = event.mouseMove.y;
            }
            else if(event.type == sf::Event::MouseButtonPressed)
            {
                getmouse = true;
            }
            else if(event.type == sf::Event::MouseButtonReleased)
            {
                getmouse = false;
            }
            else if(event.type == sf::Event::KeyPressed)
            {
                auto& _agents = machine.template get<agents>();
                std::cout << _agents.size() << std::endl;

                rgba _path;
                if(path.b)
                    _path.r = 0x40;
                else if(path.r)
                    ;
                else
                    _path.b = 0x40;
                path = _path;
            }
        }

        if(!focus)
            machine.template prepare<poll>();
    }
    rgba path;
    bool focus;
    int framerate = 0;
    sf::Clock clock;
    int16_t width, height, xmouse, ymouse;
    bool getmouse = false;
};

struct clear : cl::enable<clear>
{
    template<typename t_machine>
    void operator()(t_machine& machine)
    {
        auto& window = machine.template get<logic>().window;
        window.clear(sf::Color::Black);
    }
};

struct update : cl::enable<update>
{
    template<typename t_machine>
    void operator()(t_machine& machine)
    {
        auto& grid = machine.template get<logic>().grid;
        grid->reset();

        auto& _agents = machine.template get<agents>();
        for(auto& f : _agents.functors)
        {
            auto& boid = grid->get(gdiv(f.x), gdiv(f.y));
            boid.dir.x += f.vx;
            boid.dir.y += f.vy;
            boid.pos.x += f.x;
            boid.pos.y += f.y;
            boid.cnt += 1;
        }
    }
};

struct display : cl::enable<display>
{
    template<typename t_machine>
    void operator()(t_machine& machine)
    {
        auto& texture = machine.template get<logic>().texture;
        auto& window = machine.template get<logic>().window;
        auto& map = machine.template get<logic>().map;

        texture.update(map->raw());
        sf::Sprite sprite(texture);

        window.draw(sprite);
        window.display();
    }
};

struct spawn : cl::enable<spawn>
{
    template<typename t_machine, typename t_functor>
    static void process(t_machine& machine, t_functor& agents)
    {
        auto size = agents.size();
        if(size >= quantity)
            return;

        auto& _poll = machine.template get<poll>();
        if(_poll.getmouse)
            return;
        else if(_poll.framerate < target_framerate)
            return;

        auto seed = _poll.framerate + size;
        if(size > 2)
        {
            seed += agents.functors[0].age;
            seed += agents.functors[1].age;
            seed += agents.functors[2].age;
        }

        std::mt19937 roll(seed);
        std::uniform_int_distribution<int16_t> rvx(-2, 2);
        std::uniform_int_distribution<int16_t> rvy(-2, 2);
        std::uniform_int_distribution<uint16_t> rage(0, max_age - 1);
        std::uniform_int_distribution<int16_t> rwidth(0, _poll.width - 1);
        std::uniform_int_distribution<int16_t> rheight(0, _poll.height - 1);

        constexpr int v = 8 * quantity / max_age;
        for(size_t i = 0; size + i < quantity && i < v; ++i)
            agents.add(
                rwidth(roll), rheight(roll),
                rvx(roll), rvy(roll),
                rage(roll));
    }
};

struct destroy : cl::enable<destroy>
{
    template<typename t_machine, typename t_functor>
    static void process(t_machine& machine, t_functor& agents)
    {
        auto& map = machine.template get<logic>().map;
        for(auto& f : agents.functors)
        {
            if(f.age > max_age)
            {
                f.deleted = true;
                agents.dirty = true;
                map->get(f.x, f.y).reset();
            }
        }
        agents.clean();
    }
};

struct agents : cl::property::entry<agents>
{
    using before = spawn;
    using after = destroy;

    agents(int16_t x, int16_t y, int16_t vx, int16_t vy, uint16_t age)
        : x{x}, y{y}, vx{vx}, vy{vy}, age{age}
    { }

    int16_t target(const int16_t& a, const int16_t& b, const int16_t& v) const
    {
        int16_t d = 0;
        if(b > a + v * v && v < velocity)
            d += 1;
        else if(v > 0)
            d -= 1;

        if(b < a - v * v && v > -velocity)
            d -= 1;
        else if(v < 0)
            d += 1;

        return d;
    }

    template<typename t_grid>
    void get_boid(t_grid&& grid, const int16_t xoff, const int16_t yoff)
    {
        int16_t gx = gdiv(x) + xoff;
        int16_t gy = gdiv(y) + yoff;
        if(!grid->inbound(gx, gy))
            return;

        auto& boid = grid->get(gx, gy);
        if(boid.cnt > 0)
        {
            int16_t rx = x - gx * gsize - gsize / 2;
            int16_t ry = y - gy * gsize - gsize / 2;

            int16_t cx = (x * boid.cnt) - boid.pos.x;
            int16_t cy = (y * boid.cnt) - boid.pos.y;

            int16_t ax = boid.dir.x;
            int16_t ay = boid.dir.y;

            int16_t dx = (ax - cx * coh) / boid.cnt + (rx * dis) / gsize;
            int16_t dy = (ay - cy * coh) / boid.cnt + (ry * dis) / gsize;

            int16_t max = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
            if(max < 1)
                return;

            dx /= max;
            dy /= max;

            if(dx > 0 && vx + dx < velocity)
                vx += dx;
            else if (dx < 0 && vx + dx > -velocity)
                vx += dx;

            #ifdef GRAVITY
                if(dy > 0 && vy + dy < velocity)
                    vy += dy;
                else if (dy < 0 && vy + dy > -velocity / 2)
                    vy += dy;
            #else
                if(dy > 0 && vy + dy < velocity)
                    vy += dy;
                else if (dy < 0 && vy + dy > -velocity)
                    vy += dy;
            #endif
        }
    }

    template<typename t_machine>
    void operator()(t_machine& machine)
    {
        auto& _poll = machine.template get<poll>();
        auto& map = machine.template get<logic>().map;

        if(!map->inbound(x, y))
        {
            vx = target(x, _poll.xmouse, vx);
            vy = target(y, _poll.ymouse, vy);
            x += vx;
            y += vy;
            return;
        }

        if(!_poll.getmouse && !(age % mouse_weight))
        {
            vx += target(x, _poll.xmouse, vx);
            vy += target(y, _poll.ymouse, vy);
            #ifdef GRAVITY
                vy += 1;
            #endif
        }
        else if(_poll.getmouse && !(age % 8))
        {
            vx -= target(x, _poll.xmouse, vx);
            vy -= target(y, _poll.ymouse, vy);

            if(age > max_age / 2 && (age / 8) % 2)
            {
                vx -= 1;
                vy -= 1;
            }

        }
        else
        {
            auto& grid = machine.template get<logic>().grid;

            #ifndef FEW
                switch(age % 9)
                {
                    case 0: get_boid(grid, 1, 1); break;
                    case 1: get_boid(grid, 0, 1); break;
                    case 2: get_boid(grid, -1, 1); break;
                    case 3: get_boid(grid, 1, -1); break;
                    case 4: get_boid(grid, 0, -1); break;
                    case 5: get_boid(grid, -1, -1); break;
                    case 6: get_boid(grid, 1, 0); break;
                    case 7: get_boid(grid, 0, 0); break;
                    case 8: get_boid(grid, -1, 0); break;
                }
            #else
                get_boid(grid, 0, 0);
            #endif
        }

        map->get(x, y) = _poll.path;

        if(age % 3)
        {
            x += vx;
            y += vy;
        }

        if(map->inbound(x, y))
        {
            map->get(x, y) = {0xFF, 0xFF, 0xFF, 0xFF};
        }

        age += 1;
    }
    int16_t x, y, vx, vy;
    uint16_t age;
};

int main()
{
    using flow = cl::compose<
        logic,
        poll,
        clear,
        update,
        agents,
        cl::edge<display, poll>>;

    cl::property::machine<flow> machine;
    while(machine.pending)
        machine.execute();
}
