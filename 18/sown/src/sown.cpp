#include "clenche.hpp"
#include "sequence.hpp"
#include "property.hpp"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

namespace sown
{
    template<typename t_machine>
    void reset(t_machine& machine);

    template<typename t_machine>
    int get_difficulty(t_machine& machine);
}

#include "boot.hpp"
#include "draw.hpp"
#include "sound.hpp"
#include "poll.hpp"
#include "snow.hpp"
#include "cook.hpp"
#include "parts.hpp"
#include "logic.hpp"
#include "timer.hpp"
#include "agents.hpp"
#include "player.hpp"
#include "landscape.hpp"
#include "inventory.hpp"

namespace sown
{
    template<typename t_machine>
    int get_difficulty(t_machine& machine)
    {
        return machine.template get<sown::leafs>().size();
    }

    template<typename t_entries>
    void empty(t_entries& entries)
    {
        for(auto& f : entries.functors)
            f.deleted = true;
        entries.dirty = true;
        entries.clean();
    }

    template<typename t_machine>
    void reset(t_machine& machine)
    {
        empty(machine.template get<sown::rocks>());
        empty(machine.template get<sown::coals>());
        empty(machine.template get<sown::woods>());
        empty(machine.template get<sown::leafs>());
        empty(machine.template get<sown::planks>());
        empty(machine.template get<sown::fires>());
        empty(machine.template get<sown::snow::flakes>());
        empty(machine.template get<sown::snow::bars>());
        empty(machine.template get<sown::drawables>());

        auto& boot = machine.template get<sown::boot>();
        auto& logic = machine.template get<sown::logic>();
        logic.reset(logic._width, logic._height, boot._window);

        auto& timer = machine.template get<sown::timer>();
        timer.reset(machine);

        auto& spawn = machine.template get<sown::snow::spawn>();
        spawn.reset(machine);

        auto& player = machine.template get<sown::player>();
        player.reset(machine);

        auto& inventory = machine.template get<sown::inventory>();
        inventory.reset(machine);

        auto& landscape = machine.template get<sown::landscape>();
        landscape.reset(machine);

        machine.template prepare<sown::logic>();
    }
}

int main()
{
    using loop = cl::compose<
        sown::poll,
        sown::clear,
        sown::timer,
        sown::view,
        sown::sound::sound,
        sown::player,
        sown::inventory,
        sown::landscape,
        sown::rocks,
        sown::coals,
        sown::woods,
        sown::leafs,
        sown::planks,
        sown::fires,
        sown::snow::spawn,
        sown::snow::flakes,
        sown::snow::wind,
        sown::snow::bars,
        sown::drawables,
        sown::sound::music,
        cl::edge<sown::logic, sown::poll>
        >;

    using startup = cl::edge<sown::boot, sown::logic>;

    cl::property::machine<startup, loop> machine;
    while(machine.pending)
        machine.execute();
}
