#include "clenche/clenche.hpp"
#include "clenche/property.hpp"

#include <array>
#include <random>
#include <iostream>

constexpr size_t nb_possible = 5; // rock-paper-scissors-lizard-spock
constexpr size_t nb_player = 1000000;

template<typename t_factory>
struct player;
struct turn;
struct init;

// hideous code
#include "hideous.cpp"

// less hideous code (sic)
template<typename t_factory>
struct player : cl::property::entry<player<t_factory>>
{
    using factory_type = t_factory;
    using state_type = typename factory_type::state_type;

    // « from-scratch » constructor, creates a new player
    player(factory_type& factory)
        : factory(factory), alice_state(factory()), bob_state(factory())
    { }

    // state getter & setters
    void set_alice_state(item first, item second, item value)
    {
        alice_state[unpick(first)][unpick(second)] = value;
    }

    void set_bob_state(item first, item second, item value)
    {
        bob_state[unpick(first)][unpick(second)] = value;
    }

    item get_alice_state(item first, item second)
    {
        return alice_state[unpick(first)][unpick(second)];
    }

    item get_bob_state(item first, item second)
    {
        return bob_state[unpick(first)][unpick(second)];
    }

    // « mutate-from-other » constructor, mutate a new player from an other one
    player(const player<factory_type>& other)
        : factory(other.factory),
          alice_state(other.alice_state),
          bob_state(other.bob_state)
    {
        auto& rfactory = factory.get();
        set_alice_state(rfactory.get(), rfactory.get(), rfactory.get());
        set_bob_state(rfactory.get(), rfactory.get(), rfactory.get());
    }

    // before static method, called before the operator() core iteration
    template<typename t_machine, typename t_property>
    static void before(t_machine& machine, t_property& property)
    {
        // remove old ones
        property.clean();

        // stop the machine if no one left
        if(property.size() == 0)
        {
            machine.finish();
            return;
        }

        // add new entries some when needed
        if(property.size() < nb_player / 16)
        {
            auto target = property.size();
            auto& first = property.functors[0];
            for(size_t i = 0, j = 0; i < target * 8; ++i, j = i % target)
            {
                if(i % 3 == 1)
                    property.add(first.factory);
                else
                    property.add(property.functors[j]);
            }
        }
    }

    // core iteration
    template<typename t_property>
    void operator()(t_property&, item last_play, item next_play)
    {
        auto& rfactory = factory.get();

        item alice_next_play = get_alice_state(last_play, play);
        if(alice_next_play != next_play)
            set_alice_state(last_play, play, rfactory.get());

        play = get_bob_state(alice_next_play, last_play);
        if(play == lplay)
            play = rfactory.get();
        lplay = play;

        win = rules(next_play, play);
        if(win == match_result::alice)
            set_bob_state(alice_next_play, last_play, rfactory.get());
    }

    // after static method, called after the operator() core iteration
    template<typename t_machine, typename t_property>
    static void after(t_machine& machine, t_property& property)
    {
        // prepare the next call
        machine.template prepare<turn>();

        // skip this part half the time
        auto target = property.size();
        if(target % 2 == 0)
        {
                property.remove(property.size());
                return;
        }

        // remove losers and tie-ers
        for(size_t i = 0; i < property.size(); ++i)
        {
            const auto& result = property.functors[i].win;
            if(result != match_result::alice)
                property.remove(i);
        }
    }

    // MUST use reference_wrapper to keep move assignement possible
    std::reference_wrapper<factory_type> factory;
    player<factory_type>& operator=(player<factory_type>&&) = default;

    // internal state of each entry
    state_type alice_state;
    state_type bob_state;
    match_result win = match_result::not_started;
    item play, lplay;
};

using player_type = player<factory_type>;

// tournament turn
struct turn : cl::enable<turn>
{
    template<typename t_machine>
    void operator()(t_machine& machine)
    {
        stats(lplay, machine.template get<player_type>());

        item nplay = ask();
        machine.template prepare<player_type>(lplay, nplay);
        lplay = nplay;
    }

    item lplay;
};

// tournament init
struct init : cl::enable<init>
{
    template<typename t_machine>
    void operator()(t_machine& machine, factory_type& factory)
    {
        auto& property = machine.template get<player_type>();
        for(size_t i = 0; i < nb_player; ++i)
            property.add(factory);

        item lplay = factory.get();
        item nplay = ask();

        auto& next_turn = machine.template get<turn>();
        next_turn.lplay = nplay;

        machine.template prepare<player_type>(lplay, nplay);
    }
};

int main()
{
    // factory building new internal states, non-copyable
    std::random_device entropy("/dev/urandom");
    factory_type factory(entropy);

    // our usual machine loop
    cl::property::machine<init, turn, player_type> machine(factory);
    while(machine.pending)
        machine.execute();
}
