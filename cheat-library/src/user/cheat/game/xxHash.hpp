#pragma once
#include <cstdint> // for uint64_t
#include <cassert> // for assert
#include <string>

namespace xxh
{
    namespace detail
    {
        // Magic primes
        constexpr uint64_t PRIME1 = 0x9E3779B185EBCA87ull;
        constexpr uint64_t PRIME2 = 0xC2B2AE3D27D4EB4Full;
        constexpr uint64_t PRIME3 = 0x165667B19E3779F9ull;
        // Not used (yet)
        // constexpr uint64_t PRIME4 = 0x85EBCA77C2B2AE63ull;
        // constexpr uint64_t PRIME5 = 0x27D4EB2F165667C5ull;

        struct uint128_t
        {
            uint64_t low, high;
        };

        [[nodiscard]]
        inline constexpr uint128_t umul64(const uint64_t a, const uint64_t b)
        {
            const uint32_t aLo = static_cast<uint32_t>(a);
            const uint32_t aHi = static_cast<uint32_t>(a >> 32);
            const uint32_t bLo = static_cast<uint32_t>(b);
            const uint32_t bHi = static_cast<uint32_t>(b >> 32);

            const uint64_t b00 = static_cast<uint64_t>(aLo) * bLo;
            const uint64_t b01 = static_cast<uint64_t>(aLo) * bHi;
            const uint64_t b10 = static_cast<uint64_t>(aHi) * bLo;
            const uint64_t b11 = static_cast<uint64_t>(aHi) * bHi;

            const uint32_t b00Lo = static_cast<uint32_t>(b00);
            const uint32_t b00Hi = static_cast<uint32_t>(b00 >> 32);

            const uint64_t mid1 = b10 + b00Hi;
            const uint32_t mid1Lo = static_cast<uint32_t>(mid1);
            const uint32_t mid1Hi = static_cast<uint32_t>(mid1 >> 32);

            const uint64_t mid2 = b01 + mid1Lo;
            const uint32_t mid2Lo = static_cast<uint32_t>(mid2);
            const uint32_t mid2Hi = static_cast<uint32_t>(mid2 >> 32);

            const uint64_t pHi = b11 + mid1Hi + mid2Hi;
            const uint64_t pLo = (static_cast<uint64_t>(mid2Lo) << 32) + b00Lo;

            return { pLo, pHi };
        }

        [[nodiscard]]
        inline constexpr uint64_t byteswap_uint64(const uint64_t value)
        {
            return
                ((value & 0xff000000'00000000) >> 56) |
                ((value & 0x00ff0000'00000000) >> 40) |
                ((value & 0x0000ff00'00000000) >> 24) |
                ((value & 0x000000ff'00000000) >> 8 ) |

                ((value & 0x00000000'ff000000) << 8 ) |
                ((value & 0x00000000'00ff0000) << 24) |
                ((value & 0x00000000'0000ff00) << 40) |
                ((value & 0x00000000'000000ff) << 56);
        }

        [[nodiscard]]
        inline constexpr uint64_t umul128_fold64(const uint64_t lhs, const uint64_t rhs)
        {
            const auto [low, high] = umul64(lhs, rhs);
            return low ^ high;
        }

        [[nodiscard]]
        inline constexpr uint64_t rotate_left64(const uint64_t x, const uint8_t bits)
        {
            return (x << bits) | (x >> (64 - bits));
        }

        [[nodiscard]]
        inline constexpr uint64_t xorshift64(const uint64_t x, const uint8_t shift)
        {
            assert(0 <= shift && shift < 64);
            return x ^ (x >> shift);
        }

        [[nodiscard]]
        inline constexpr uint64_t xxh_avalanche(uint64_t hash)
        {
            hash ^= hash >> 33;
            hash *= PRIME2;
            hash ^= hash >> 29;
            hash *= PRIME3;
            hash ^= hash >> 32;
            return hash;
        }

        [[nodiscard]]
        inline constexpr uint64_t xxh3_avalanche(uint64_t hash)
        {
            hash = xorshift64(hash, 37);
            hash *= 0x165667919E3779F9ULL;
            hash = xorshift64(hash, 32);
            return hash;
        }

        [[nodiscard]]
        inline constexpr uint64_t rrmxmx(uint64_t hash, size_t len)
        {
            hash ^= rotate_left64(hash, 49) ^ rotate_left64(hash, 24);
            hash *= 0x9FB21C651E98DF25ULL;
            hash ^= (hash >> 35) + len;
            hash *= 0x9FB21C651E98DF25ULL;
            return xorshift64(hash, 28);
        }

        // Helper function to read a uint32 from a char array in constexpr
        // constexpr does not allow us to reinterpret_cast the
        // the string straight to a uint32
        [[nodiscard]]
        constexpr uint32_t read32(const char *data)
        {
            const uint32_t a = static_cast<const uint8_t>(data[0]);
            const uint32_t b = static_cast<const uint8_t>(data[1]);
            const uint32_t c = static_cast<const uint8_t>(data[2]);
            const uint32_t d = static_cast<const uint8_t>(data[3]);
            return (d << 24) | (c << 16) | (b << 8) | a;
        }

        // Helper function to read a uint64 from a char array.
        // constexpr does not allow us to reinterpret_cast the
        // the string straight to a uint64
        [[nodiscard]]
        constexpr uint64_t read64(const char *data)
        {
            const uint64_t a = static_cast<const uint8_t>(data[0]);
            const uint64_t b = static_cast<const uint8_t>(data[1]);
            const uint64_t c = static_cast<const uint8_t>(data[2]);
            const uint64_t d = static_cast<const uint8_t>(data[3]);
            const uint64_t e = static_cast<const uint8_t>(data[4]);
            const uint64_t f = static_cast<const uint8_t>(data[5]);
            const uint64_t g = static_cast<const uint8_t>(data[6]);
            const uint64_t h = static_cast<const uint8_t>(data[7]);
            return (h << 56) | (g << 48) | (f << 40) | (e << 32) | (d << 24) | (c << 16) | (b << 8) | a;
        }

        [[nodiscard]]
        constexpr uint64_t xxh_len_1to3_64b(const char *data, size_t size)
        {
            const uint32_t c1 = static_cast<uint8_t>(data[0]); // yes, promote to 32
            const uint32_t c2 = static_cast<uint8_t>(data[size / 2]);
            const uint32_t c3 = static_cast<uint8_t>(data[size - 1]);
            const uint32_t len = static_cast<uint32_t>(size);
            const uint32_t combined = (
                (c1 << 16) |
                (c2 << 24) |
                (c3 << 0)  |
                (len << 8)
            );
            const uint32_t secret1 = 0x396CFEB8;
            const uint32_t secret2 = 0xBE4BA423;
            const uint64_t bitflip = secret1 ^ secret2;
            const uint64_t keyed = static_cast<uint64_t>(combined) ^ bitflip;
            return xxh_avalanche(keyed);
        }

        [[nodiscard]]
        constexpr uint64_t xxh_len_4to8_64b(const char *data, size_t size)
        {
            const uint64_t input1 = read32(data); // yes, promote to 64
            const uint64_t input2 = read32(data + size - 4);
            const uint64_t input64 = (input1 << 32) + input2;

            const uint64_t secret1 = 0x1cad21f72c81017c;
            const uint64_t secret2 = 0xdb979083e96dd4de;
            const uint64_t bitflip = secret1 ^ secret2;

            const uint64_t keyed = input64 ^ bitflip;
            return rrmxmx(keyed, size);
        }

        [[nodiscard]]
        constexpr uint64_t xxh_len_9to16_64b(const char *data, size_t size)
        {
            const uint64_t input1 = read64(data);
            const uint64_t input2 = read64(data + size - 8);

            const uint64_t secret1 = 0X1F67B3B7A4A44072;
            const uint64_t secret2 = 0X78E5C0CC4EE679CB;
            const uint64_t secret3 = 0X2172FFCC7DD05A82;
            const uint64_t secret4 = 0X8E2443F7744608B8;
            const uint64_t bitflip1 = secret1 ^ secret2;
            const uint64_t bitflip2 = secret3 ^ secret4;

            const uint64_t input_lo = input1 ^ bitflip1;
            const uint64_t input_hi = input2 ^ bitflip2;

            const uint64_t acc = size +
                byteswap_uint64(input_lo) + input_hi +
                umul128_fold64(input_lo, input_hi);
            return xxh3_avalanche(acc);
        }

        [[nodiscard]]
        constexpr uint64_t xxh_len_17to128_64b(const char *data, size_t size)
        {
            const uint64_t secrets[] = {
                0xBE4BA423396CFEB8,
                0x1CAD21F72C81017C,
                0xDB979083E96DD4DE,
                0x1F67B3B7A4A44072,
                0x78E5C0CC4EE679CB,
                0x2172FFCC7DD05A82,
                0x8E2443F7744608B8,
                0x4C263A81E69035E0,
                0xCB00C391BB52283C,
                0xA32E531B8B65D088,
                0x4EF90DA297486471,
                0xD8ACDEA946EF1938,
                0x3F349CE33F76FAA8,
                0x1D4F0BC7C7BBDCF9,
                0x3159B4CD4BE0518A
            };

            uint64_t acc = size * PRIME1;
            size_t i = (size - 1) / 32;
            do
            {
                const uint64_t input1 = read64(data + 16 * i);
                const uint64_t input2 = read64(data + 16 * i + 8);
                const uint64_t input3 = read64(data + size - 16 * (i + 1));
                const uint64_t input4 = read64(data + size - 16 * (i + 1) + 8);
                const uint64_t secret1 = secrets[2 * i];
                const uint64_t secret2 = secrets[2 * i + 1];
                const uint64_t secret3 = secrets[2 * (i + 1)];
                const uint64_t secret4 = secrets[2 * (i + 1) + 1];

                acc += umul128_fold64(input1 ^ secret1, input2 ^ secret2);
                acc += umul128_fold64(input3 ^ secret3, input4 ^ secret4);
            } while (i-- != 0);

            return xxh3_avalanche(acc);
        }
    }

    /// <summary>
    /// Hashes a string of up to 128 characters.
    /// </summary>
    /// <param name="data">The character array to be hashed</param>
    /// <param name="size">
    /// The size of the character array.
    /// If working with null-terminated strings, the size should not include
    /// the null character.
    /// </param>
    /// <returns>
    /// A 64bit unsigned integer representing a unique hash for
    /// the given string.
    /// </returns>
    [[nodiscard]]
    constexpr uint64_t xxHash64(const char* data, size_t size)
    {
        assert(0 < size && size <= 128);
        if (size < 4) // yes, this is strictly less
            return detail::xxh_len_1to3_64b(data, size);
        if (size <= 8)
            return detail::xxh_len_4to8_64b(data, size);
        if (size <= 16)
            return detail::xxh_len_9to16_64b(data, size);
        if (size <= 128)
            return detail::xxh_len_17to128_64b(data, size);
        return 0;
    }

    [[nodiscard]]
    constexpr uint64_t xxHash64(const std::string &data)
    {
        return xxHash64(data.c_str(), data.size());
    }

    /// <summary>Decorator meant for ease of use.</summary>
    /// <param name="arr">Passed automatically by compiler</param>
    /// <param name="size">Passed automatically by compiler</param>
    /// <returns>
    /// A 64bit unsigned integer representing a unique hash for
    /// the given string.
    /// </returns>
    [[nodiscard]]
    constexpr uint64_t operator "" _h(const char *arr, size_t size)
    {
        return xxHash64(arr, size);
    }
}
