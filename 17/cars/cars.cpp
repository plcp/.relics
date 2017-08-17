#include <random>
#include <memory>
#include <cstdint>
#include <iostream>

#include "clenche.hpp"
#include "sequence.hpp"
#include "property.hpp"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

constexpr int velocity = 256;
constexpr int target_framerate = 60;

#ifdef COLLIDE
    constexpr bool physics = true;
    constexpr int quantity = 200000; // we need to limit in order to see smth
    constexpr int wobble = 30;
#else
    constexpr bool physics = false;
    constexpr int quantity = 10000000; // as much as possible
    #ifdef WOBBLE
        constexpr int wobble = 10;
    #else
        constexpr int wobble = 0;
    #endif
#endif

struct poll;
struct logic;

struct rgba
{
    sf::Uint8 r = 0, g = 0, b = 0, a = 0xFF;
};

struct map_type
{
    map_type(int16_t width, int16_t height)
        : pixels{std::make_unique<rgba[]>(width * height)},
          width{width}, height{height}
    { }

    bool inbound(const int16_t x, const int16_t y) const
    {
        if(x < 0 || x >= width || y < 0 || y >= height)
            return false;
        return true;
    }

    const auto& cget(const int16_t x, const int16_t y) const
    {
        if(!inbound(x, y))
            return _outofbound;
        return pixels[x + y * width];
    }

    auto& get(const int16_t x, const int16_t y)
    {
        if(!inbound(x, y))
            return _outofbound;
        return pixels[x + y * width];
    }

    auto raw()
    {
        return reinterpret_cast<sf::Uint8*>(pixels.get());
    }

    std::unique_ptr<rgba[]> pixels;
    rgba _outofbound;
    int16_t width, height;
};

struct bootstrap : cl::enable<bootstrap>
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

        map = std::make_unique<map_type>(size.x, size.y);
        texture.create(size.x, size.y);
    }
    std::unique_ptr<map_type> map;
    sf::RenderWindow window;
    sf::Texture texture;
};

struct poll : cl::enable<poll>
{
    template<typename t_machine>
    void operator()(t_machine& machine)
    {
        auto& texture = machine.template get<bootstrap>().texture;
        auto& window = machine.template get<bootstrap>().window;
        auto& map = machine.template get<bootstrap>().map;

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

                map = std::make_unique<map_type>(width, height);
                texture.create(width, height);
            }
            else if(event.type == sf::Event::MouseMoved)
            {
                xmouse = event.mouseMove.x;
                ymouse = event.mouseMove.y;
            }
            else if(event.type == sf::Event::MouseButtonPressed)
            {
                polarity = -1;
            }
            else if(event.type == sf::Event::MouseButtonReleased)
            {
                polarity = 1;
            }
            else if(event.type == sf::Event::KeyPressed)
            {
                auto& _logic = machine.template get<logic>();
                std::cout << _logic.size() << std::endl;

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
    int16_t width, height, xmouse, ymouse, polarity = 1;
};

struct clear : cl::enable<clear>
{
    template<typename t_machine>
    void operator()(t_machine& machine)
    {
        auto& window = machine.template get<bootstrap>().window;
        window.clear(sf::Color::Black);
    }
};

struct display : cl::enable<display>
{
    template<typename t_machine>
    void operator()(t_machine& machine)
    {
        auto& texture = machine.template get<bootstrap>().texture;
        auto& window = machine.template get<bootstrap>().window;
        auto& map = machine.template get<bootstrap>().map;

        texture.update(map->raw());
        sf::Sprite sprite(texture);

        window.draw(sprite);
        window.display();
    }
};

struct spawn : cl::enable<spawn>
{
    template<typename t_machine, typename t_functor>
    static void process(t_machine& machine, t_functor& logic)
    {
        auto size = logic.size();
        if(size >= quantity)
            return;

        auto& _poll = machine.template get<poll>();
        if(_poll.framerate < target_framerate)
            return;

        std::mt19937 roll(size);
        std::uniform_int_distribution<int16_t> rwidth(0, _poll.width - 1);
        std::uniform_int_distribution<int16_t> rheight(0, _poll.height - 1);

        constexpr int v = quantity / target_framerate;
        for(size_t i = 0; size + i < quantity && i < v; ++i)
            logic.add(rwidth(roll), rheight(roll));
    }
};

struct destroy : cl::enable<destroy>
{
    template<typename t_machine, typename t_functor>
    static void process(t_machine& machine, t_functor& logic)
    {
        auto& _poll = machine.template get<poll>();
        for(auto& f : logic.functors)
        {
            if(!physics && f.x == _poll.xmouse && f.y == _poll.ymouse)
            {
                f.deleted = true;
                logic.dirty = true;
            }
        }
        logic.clean();
    }
};

struct logic : cl::property::entry<logic>
{
    using before = spawn;
    using after = destroy;

    logic(int16_t xstart, int16_t ystart)
        : x{xstart}, y{ystart}
    { }

    int16_t target(const int16_t& a, const int16_t& b, const int16_t& v) const
    {
        int16_t d = 0;
        if(b > a + v * v && v < velocity)
            d += 1;
        else if(v > wobble)
            d -= 1;

        if(b < a - v * v && v > -velocity)
            d -= 1;
        else if(v < -wobble)
            d += 1;

        return d;
    }

    template<typename t_machine>
    void operator()(t_machine& machine)
    {
        auto& _poll = machine.template get<poll>();
        auto& map = machine.template get<bootstrap>().map;

        if(!map->inbound(x, y))
        {
            auto& _logic = machine.template get<logic>();
            _logic.dirty = true;
            deleted = true;
            return;
        }

        vx += target(x, _poll.xmouse, vx) * _poll.polarity;
        vy += target(y, _poll.ymouse, vy) * _poll.polarity;

        if(!physics)
        {
            map->get(x, y) = _poll.path;
            x += vx;
            y += vy;

        }
        else if(map->inbound(x + vx, y + vy) && !map->cget(x + vx, y + vy).g)
        {
            map->get(x, y) = _poll.path;
            x += vx;
            y += vy;
        }
        else
        {
            vx = -(vx * 0.9);
            vy = -(vy * 0.9);
        }

        if(map->inbound(x, y))
        {
            map->get(x, y) = {0xFF, 0xFF, 0xFF, 0xFF};
        }
    }
    int16_t x, y, vx = 0, vy = 0;
};

int main()
{
    using flow = cl::compose<
        bootstrap,
        poll,
        clear,
        logic,
        cl::edge<display, poll>>;

    cl::property::machine<flow> machine;
    while(machine.pending)
        machine.execute();
}
