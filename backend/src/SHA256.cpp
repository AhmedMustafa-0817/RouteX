#include "SHA256.h"
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

namespace
{
    const uint32_t K[64] =
    {
        0x428a2f98, 0x71374491, 0xb5c0fbcf,
        0xe9b5dba5, 0x3956c25b, 0x59f111f1,
        0x923f82a4, 0xab1c5ed5, 0xd807aa98,
        0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
        0xc19bf174, 0xe49b69c1, 0xefbe4786,
        0x0fc19dc6, 0x240ca1cc, 0x2de92c6f,
        0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8,
        0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
        0x06ca6351, 0x14292967, 0x27b70a85,
        0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e,
        0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3, 0xd192e819,
        0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c,
        0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
        0x5b9cca4f, 0x682e6ff3, 0x748f82ee,
        0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7,
        0xc67178f2
    };

    uint32_t rotateRight(uint32_t value, int bits)
    {
        return (value >> bits) | (value << (32 - bits));
    }

    void processBlock(
        const unsigned char block[64],
        uint32_t hashValues[8])
    {
        uint32_t words[64];

        for (int i = 0; i < 16; i++)
        {
            int j = i * 4;

            words[i] =
                ((uint32_t)block[j] << 24) |
                ((uint32_t)block[j + 1] << 16) |
                ((uint32_t)block[j + 2] << 8) |
                ((uint32_t)block[j + 3]);
        }

        for (int i = 16; i < 64; i++)
        {
            uint32_t s0 =
                rotateRight(words[i - 15], 7) ^
                rotateRight(words[i - 15], 18) ^
                (words[i - 15] >> 3);

            uint32_t s1 =
                rotateRight(words[i - 2], 17) ^
                rotateRight(words[i - 2], 19) ^
                (words[i - 2] >> 10);

            words[i] =
                words[i - 16] + s0 +
                words[i - 7] + s1;
        }

        uint32_t a = hashValues[0];
        uint32_t b = hashValues[1];
        uint32_t c = hashValues[2];
        uint32_t d = hashValues[3];
        uint32_t e = hashValues[4];
        uint32_t f = hashValues[5];
        uint32_t g = hashValues[6];
        uint32_t h = hashValues[7];

        for (int i = 0; i < 64; i++)
        {
            uint32_t sum1 =
                rotateRight(e, 6) ^
                rotateRight(e, 11) ^
                rotateRight(e, 25);

            uint32_t choice =
                (e & f) ^ ((~e) & g);

            uint32_t temp1 =
                h + sum1 + choice + K[i] + words[i];

            uint32_t sum0 =
                rotateRight(a, 2) ^
                rotateRight(a, 13) ^
                rotateRight(a, 22);

            uint32_t majority =
                (a & b) ^ (a & c) ^ (b & c);

            uint32_t temp2 = sum0 + majority;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        hashValues[0] += a;
        hashValues[1] += b;
        hashValues[2] += c;
        hashValues[3] += d;
        hashValues[4] += e;
        hashValues[5] += f;
        hashValues[6] += g;
        hashValues[7] += h;
    }
}

string SHA256::hash(const string& input)
{
    uint32_t hashValues[8] =
    {
        0x6a09e667, 0xbb67ae85,
        0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c,
        0x1f83d9ab, 0x5be0cd19
    };

    uint64_t originalSize = input.size();
    uint64_t bitLength = originalSize * 8ULL;

    uint64_t paddedSize = originalSize + 1;

    while (paddedSize % 64 != 56)
    {
        paddedSize++;
    }

    paddedSize += 8;

    unsigned char* data = new unsigned char[paddedSize];

    for (uint64_t i = 0; i < originalSize; i++)
    {
        data[i] = (unsigned char)input[i];
    }

    data[originalSize] = 0x80;

    for (uint64_t i = originalSize + 1; i < paddedSize - 8; i++)
    {
        data[i] = 0;
    }

    for (int i = 0; i < 8; i++)
    {
        data[paddedSize - 1 - i] =
            (unsigned char)((bitLength >> (i * 8)) & 0xff);
    }

    for (uint64_t i = 0; i < paddedSize; i += 64)
    {
        processBlock(data + i, hashValues);
    }

    delete[] data;

    stringstream result;
    result << hex << setfill('0');

    for (int i = 0; i < 8; i++)
    {
        result << setw(8) << hashValues[i];
    }

    return result.str();
}