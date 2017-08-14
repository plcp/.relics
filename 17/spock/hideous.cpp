// hideous code, do not look

enum item
{
    rock = 'r',
    paper = 'p',
    scissors = 'c',
    lizard = 'l',
    spock = 's'
};


enum match_result
{
    alice,
    tie,
    bob,
    not_started
};


match_result rules(const item& alice, const item& bob)
{
    if(alice == bob)
        return match_result::tie;

    switch(alice - bob)
    {
        case item::rock     - item::scissors:
        case item::rock     - item::lizard:
        case item::paper    - item::rock:
        case item::paper    - item::spock:
        case item::scissors - item::paper:
        case item::scissors - item::lizard:
        case item::lizard   - item::paper:
        case item::lizard   - item::spock:
        case item::spock    - item::rock:
        case item::spock    - item::scissors:
            return match_result::alice;

        default:
            return match_result::bob;
    }
}


item pick(const unsigned char which)
{
    switch(which)
    {
        case 4:
            return item::spock;
        case 1:
            return item::paper;
        case 2:
            return item::scissors;
        case 3:
            return item::lizard;
        default:
            return item::rock;
    }
}


unsigned char unpick(const item which)
{
    switch(which)
    {
        case item::spock:
            return 4;
        case item::paper:
            return 1;
        case item::scissors:
            return 2;
        case item::lizard:
            return 3;
        default:
            return 0;
    }
}


std::string string_pick(const item which)
{
    switch(which)
    {
        case item::spock:
            return "spock";
        case item::paper:
            return "paper";
        case item::scissors:
            return "scissors";
        case item::lizard:
            return "lizard";
        default:
            return "rock";
    }
}


template<size_t t_state_space>
using state = std::array<std::array<item, t_state_space>, t_state_space>;


template<size_t t_state_space>
struct state_factory
{
    using state_type = state<t_state_space>;
    constexpr static size_t state_space = t_state_space;

    state_factory(std::random_device& entropy)
        : roll(entropy()), dice(0, t_state_space - 1)
    { }

    item get()
    {
        return pick(dice(roll));
    }

    state_type operator()()
    {
        state_type state;
        for(auto& array : state)
            for(auto& it : array)
            {
                it = get();
            }

        return state;
    }

    std::mt19937 roll;
    std::uniform_int_distribution<int> dice;
};


using factory_type = state_factory<nb_possible>;


item ask()
{
    while(true)
    {
        std::cout << "Your pick ? ";
        std::string answer;
        std::cin >> answer;
        if(answer == "rock")
            return item::rock;
        else if(answer == "paper")
            return item::paper;
        else if(answer == "scissors")
            return item::scissors;
        else if(answer == "lizard")
            return item::lizard;
        else if(answer == "spock")
            return item::spock;
        std::cout << "Invalid ! Please retry...\n\n";
    }
}


size_t total_won = 0;
size_t total_tie = 0;
size_t total_lost = 0;

template<typename t_property>
void stats(const item& lplay, t_property& property)
{
    size_t won = 0;
    size_t tie = 0;
    size_t lost = 0;
    std::array<size_t, factory_type::state_space> plays;
    for(size_t& n : plays)
        n = 0;

    for(auto& f : property.functors)
    {
        switch(f.win)
        {
            case match_result::alice:
                ++lost;
                break;
            case match_result::bob:
                ++won;
                break;
            default:
                ++tie;
        }
        ++plays[unpick(f.play)];
    }

    int imax = -1;
    size_t max = 0;
    for(size_t i = 0; i < plays.size(); ++i)
    {
        if(plays[i] > max)
        {
            max = plays[i];
            imax = i;
        }
    }

    auto tplay = pick(imax);

    std::cout << "They play: " << string_pick(tplay);
    std::cout << "    \t" << string_pick(lplay);
    std::cout << " vs " << string_pick(tplay) << " --> ";

    size_t total = total_won + total_lost + total_tie;
    float base = 100.0 / float(total_won + total_lost + total_tie);
    switch(rules(lplay, tplay))
    {
        case match_result::alice:
            std::cout << "You win ! ";
            if(total > 3)
                std::cout
                    << "\r\t\t\t\t\t\t\t(" << int(total_won * base)
                    << "% " << "of the time)";
            ++total_won;
            break;
        case match_result::bob:
            std::cout << "You lost !";
            if(total > 3)
                std::cout
                    << "\r\t\t\t\t\t\t\t(" << int(total_lost * base)
                    << "% " << "of the time)";
            ++total_lost;
            break;
        default:
            std::cout << "Tie !      ";
            if(total > 3)
                std::cout
                    << "\r\t\t\t\t\t\t\t(" << int(total_tie * base)
                    << "% " << "of the time)";
            ++total_tie;
    }
    if(total_won < total_lost)
        std::cout << " and they are leading !";
    std::cout << "\n\n";

    base = 100.0 / float(won + lost + tie);

    std::cout << won + lost + tie << " have played, ";
    std::cout << int(won * base) << "% won, ";
    std::cout << int(tie * base) << "% " << "tied, ";
    std::cout << int(lost * base) << "% " << "lost";
    std::cout << "\t\n(";
    for(size_t i = 0; i < plays.size(); ++i)
    {
        std::cout << string_pick(pick(i)) << ": ";
        std::cout << int(plays[i] * base) << "%";
        if(i + 1 < plays.size())
            std::cout << ", ";
    }
    std::cout << ")\n" << std::endl;

}
