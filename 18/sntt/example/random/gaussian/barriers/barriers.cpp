#include <iostream>
#include "FastGaussianNoise.hpp"

int main()
{
    nfl::FastGaussianNoise<uint16_t, uint16_t, 2> prng(20, 128, 1 << 14);
    prng.precomputeBarrierValues();

    std::cout << prng._number_of_barriers << std::endl;
    for(size_t i = 0; i < prng._number_of_barriers; ++i)
    {
        for(size_t j = 0; j < prng._word_precision; ++j)
            std::cout << prng.barriers[i][j] << ", ";
        std::cout << "\n";
    }
    return 0;
}
