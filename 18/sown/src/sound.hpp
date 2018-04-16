#ifndef SOWN_SOUND_HPP__GUARD_
#define SOWN_SOUND_HPP__GUARD_

#include <string>

#include <SFML/Audio.hpp>

namespace sown::sound
{
    template<typename t_machine, typename t_emitter>
    void play(t_machine& machine);

    template<typename t_machine>
    void intensity(t_machine& machine, int relative, bool isabsolute = false);

    struct jump { };
    struct pick { };
    struct hurt { };
}

#include "snow.hpp"
#include "agents.hpp"

namespace sown::sound
{
    struct music : cl::enable<music>
    {
        music()
        {
            _snow.openFromFile(
                std::string("") + globals::sounds_directory + "snowstorm.ogg");
            _fire.openFromFile(
                std::string("") + globals::sounds_directory + "fireplace.ogg");

            _snow.setLoop(true);
            _fire.setLoop(true);

            _snow.play();
            _fire.play();
        }

        template<typename t_machine>
        void operator()(t_machine& machine)
        {
            auto& logic = machine.template get<sown::logic>();
            if(logic.tick(2) == 0)
            {
                if(_firequantity < 0)
                    _firevolume -= 1;
                else
                {
                    _firequantity -= 1;
                    _firevolume += 1;
                }
            }

            if(_firequantity > 64)
                _firequantity = 64;

            if(_firevolume > globals::musicvolume - 1)
                _firevolume = globals::musicvolume;
            if(_firevolume < 1)
                _firevolume = 0;

            if(_intensity > 256)
            {
                _snow.setVolume(globals::musicvolume - _firevolume);
                _fire.setVolume(_firevolume);
            }
            else if(_intensity < 1)
            {
                _snow.setVolume((globals::musicvolume - _firevolume) / 2);
                _fire.setVolume(_firevolume / 2);
            }
            else
            {
                auto rvol = 256 - _intensity;
                auto fvol = ((rvol * _firevolume) / 256 + _firevolume) / 2;
                auto svol = ((rvol * (globals::musicvolume - _firevolume)
                    ) / 256 + globals::musicvolume - _firevolume) / 2;

                _snow.setVolume(svol);
                _fire.setVolume(fvol);
            }
        }

        int _firevolume = globals::musicvolume, _firequantity = 0;
        int _intensity = 1;
        sf::Music _snow, _fire;
    };

    template<typename t_items>
    struct sound_config;

    template<>
    struct sound_config<sown::sound::hurt>
    {
        constexpr static char name[] = "hurt.wav";
    };

    template<>
    struct sound_config<sown::sound::pick>
    {
        constexpr static char name[] = "pick.wav";
    };

    template<>
    struct sound_config<sown::sound::jump>
    {
        constexpr static char name[] = "jump.wav";
    };

    template<>
    struct sound_config<sown::rocks>
    {
        constexpr static char name[] = "rocks.wav";
    };

    template<>
    struct sound_config<sown::coals>
    {
        constexpr static char name[] = "coals.wav";
    };

    template<>
    struct sound_config<sown::woods>
    {
        constexpr static char name[] = "woods.wav";
    };

    template<>
    struct sound_config<sown::snow::flakes>
    {
        constexpr static char name[] = "flakes.wav";
    };

    template<>
    struct sound_config<sown::planks>
    {
        constexpr static char name[] = "planks.wav";
    };

    template<>
    struct sound_config<sown::leafs>
    {
        constexpr static char name[] = "leafs.wav";
    };

    template<typename t_emitter>
    struct sound_entry
    {
        sound_entry()
        {
            _buffer.loadFromFile(
                std::string("")
                + globals::sounds_directory
                + sound_config<t_emitter>::name);
        }

        void play()
        {
            _sound.setBuffer(_buffer);
            _sound.setVolume(globals::fxvolume);
            _sound.play();
        }

        sf::SoundBuffer _buffer;
        sf::Sound _sound;
    };

    template<typename... t_sounds>
    struct sound_details : cl::enable<sound_details<t_sounds...>>,
        sound_entry<t_sounds>...
    {
        template<typename t_machine>
        void operator()(t_machine&)
        { }
    };

    using sound = sound_details<
        sown::rocks,
        sown::coals,
        sown::woods,
        sown::leafs,
        sown::snow::flakes,
        sown::planks,
        sown::sound::hurt,
        sown::sound::pick,
        sown::sound::jump
        >;

    template<typename t_machine, typename t_emitter>
    void play(t_machine& machine)
    {
        auto& sound = machine.template get<sown::sound::sound>();
        static_cast<sound_entry<t_emitter>&>(sound).play();
    }

    template<typename t_machine>
    void intensity(t_machine& machine, int relative, bool isabsolute)
    {
        auto& music = machine.template get<sown::sound::music>();
        if(isabsolute)
        {
            music._intensity = relative;
            return;
        }
        music._intensity += relative;
    }
}

#endif
