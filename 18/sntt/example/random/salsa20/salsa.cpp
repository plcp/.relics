#include <iostream>
#include <iterator>
#include <fstream>
#include <cstring>
#include <vector>

#include <nfl/prng/crypto_stream_salsa20.h>

void salsa20_keystream(
    std::vector<uint8_t>& buffer,
    const uint64_t& nonce,
    const std::vector<uint8_t>& key)
{
    // See https://blog.regehr.org/archives/959
    uint8_t vnonce[8] = {0};
    memcpy(vnonce, &nonce, 8); // Little-endian only

    // Use a « const unsigned uint8_t* » as nonce
    nfl_crypto_stream_salsa20_amd64_xmm6(
        buffer.data(),
        buffer.size(),
        vnonce,
        key.data());
}

int main()
{
    std::vector<uint8_t> buffer(1024), key(32);
    std::ifstream ifs("/dev/stdin", std::ios::in | std::ios::binary);
    ifs.read((char*) key.data(), key.size());

    for(uint64_t nonce = 0; nonce < 1024; ++nonce)
    {
        salsa20_keystream(buffer, nonce, key);
        std::copy(
            buffer.begin(), buffer.end(),
            std::ostream_iterator<uint8_t>(std::cout));
    }
}
