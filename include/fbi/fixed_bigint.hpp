#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace fbi {

template <size_t Bits>
class fixed_bigint {
    static_assert(Bits > 0, "Number of bits must be greater than 0");

  public:
    static constexpr uint32_t kSize = (Bits + 31) / 32;
    static constexpr uint32_t kTopChunkMask = (Bits % 32 == 0) ? 0xFFFFFFFF : (1U << (Bits % 32)) - 1;

    constexpr fixed_bigint() noexcept = default;

    constexpr fixed_bigint(int64_t value) noexcept {
        bool swap_sgn = value < 0;
        uint64_t uval = swap_sgn ? static_cast<uint64_t>(-(value + 1)) + 1 : static_cast<uint64_t>(value);

        SetChunk(0, static_cast<uint32_t>(uval));
        if constexpr (kSize > 1) {
            SetChunk(1, static_cast<uint32_t>(uval >> 32));
        }

        if (swap_sgn) {
            RevSgn();
        }
    }

    constexpr fixed_bigint(int value) noexcept : fixed_bigint(static_cast<int64_t>(value)) {}

    explicit constexpr fixed_bigint(uint64_t value) noexcept {
        SetChunk(0, static_cast<uint32_t>(value));
        if constexpr (kSize > 1) {
            SetChunk(1, static_cast<uint32_t>(value >> 32));
        }
    }

    constexpr explicit fixed_bigint(std::string_view str) {
        if (str.empty())
            return;

        uint32_t index = 0;
        bool sgn = false;
        if (str[index] == '-') {
            sgn = true;
            ++index;
        } else if (str[index] == '+') {
            ++index;
        }

        while (index < str.length()) {
            uint64_t buff = 0;
            uint64_t step = 1;

            while (index < str.length() && step < 1000000000000000000ULL) {
                buff *= 10;
                buff += static_cast<uint64_t>(str[index] - '0');
                ++index;
                step *= 10;
            }

            *this *= fixed_bigint<Bits>(step);
            *this += fixed_bigint<Bits>(buff);
        }

        if (sgn && *this != fixed_bigint<Bits>(0)) {
            RevSgn();
        }
    }

    [[nodiscard]] constexpr fixed_bigint operator~() const noexcept {
        fixed_bigint result;
        for (uint32_t index = 0; index < kSize; ++index) {
            result.SetChunk(index, ~this->GetChunk(index));
        }
        result.SetSgn(!this->GetSgn());
        return result;
    }

    [[nodiscard]] constexpr fixed_bigint operator-() const noexcept {
        fixed_bigint result = *this;
        return result.RevSgn();
    }

    constexpr fixed_bigint& operator+=(const fixed_bigint& other) noexcept {
        bool adds = false;
        for (uint32_t index_chunk = 0; index_chunk < kSize; ++index_chunk) {
            uint32_t chunk_a = this->GetChunk(index_chunk);
            uint32_t chunk_b = other.GetChunk(index_chunk);
            uint32_t result_chunk = chunk_a + chunk_b;

            bool next_adds = (result_chunk < chunk_a || result_chunk < chunk_b);
            if (adds && result_chunk == 0xFFFFFFFF) {
                next_adds = true;
            }
            result_chunk += adds ? 1 : 0;
            adds = next_adds;

            this->SetChunk(index_chunk, result_chunk);
        }
        SetSgn((GetSgn() + other.GetSgn() + adds) % 2);
        return *this;
    }

    constexpr fixed_bigint& operator-=(const fixed_bigint& other) noexcept {
        *this += (-other);
        return *this;
    }

    constexpr fixed_bigint& operator*=(const fixed_bigint& other) noexcept {
        fixed_bigint my_copy = *this;
        fixed_bigint other_copy = other;

        if (my_copy == fixed_bigint(0) || other_copy == fixed_bigint(0)) {
            *this = fixed_bigint(0);
            return *this;
        }

        bool final_sgn = my_copy.GetSgn() ^ other_copy.GetSgn();

        if (my_copy.GetSgn())
            my_copy.RevSgn();
        if (other_copy.GetSgn())
            other_copy.RevSgn();

        fixed_bigint result(0);

        std::array<int8_t, 256> where_dp{};
        for (auto& v : where_dp)
            v = -1;

        std::array<fixed_bigint, 64> dp{};
        uint8_t end_dp = 0;

        for (uint32_t index_chunk = 0;
             index_chunk < kSize && index_chunk < static_cast<uint32_t>(other_copy.HighestOneBit() / 32 + 2);
             ++index_chunk) {
            uint32_t chunk = other_copy.GetChunk(index_chunk);

            for (uint32_t index_byte = 0; index_byte < 4 && chunk > 0; ++index_byte) {
                uint8_t byte = chunk & 0b11111111;
                chunk >>= 8;

                if (where_dp[byte] == -1) {
                    dp[end_dp] = fixed_bigint(0);
                    where_dp[byte] = end_dp;

                    uint8_t copy_byte = byte;
                    for (uint32_t index = 0; index < 8; ++index) {
                        if (copy_byte & 1) {
                            dp[end_dp] += my_copy.LeftShift(index);
                        }
                        copy_byte >>= 1;
                    }

                    ++end_dp;
                    if (end_dp == 64) {
                        end_dp = 0;
                        for (auto& v : where_dp)
                            v = -1;
                    }
                }
                result += dp[where_dp[byte]].LeftShift(index_chunk * 32 + index_byte * 8);
            }
        }

        if (final_sgn)
            result.RevSgn();
        *this = result;
        return *this;
    }

    constexpr fixed_bigint& operator/=(const fixed_bigint& other) {
        if (other == fixed_bigint(0))
            throw std::domain_error("Division by zero");

        fixed_bigint my_copy = *this;
        fixed_bigint other_copy = other;
        bool final_sgn = my_copy.GetSgn() ^ other_copy.GetSgn();

        if (my_copy.GetSgn())
            my_copy.RevSgn();
        if (other_copy.GetSgn())
            other_copy.RevSgn();

        fixed_bigint result(0);

        if (my_copy < other_copy) {
            *this = result;
            return *this;
        }

        int32_t dif = my_copy.HighestOneBit() - other_copy.HighestOneBit();
        int32_t dif_pos = dif;

        other_copy.SelfLeftShift(dif);

        while (dif_pos >= 0) {
            while (dif_pos >= 0 && other_copy > my_copy) {
                other_copy.SelfRightShift(1);
                --dif_pos;
            }
            if (dif_pos < 0)
                break;

            result += fixed_bigint(uint64_t{1}).SelfLeftShift(dif_pos);
            my_copy -= other_copy;
        }

        if (final_sgn)
            result.RevSgn();
        *this = result;
        return *this;
    }

    constexpr fixed_bigint& operator%=(const fixed_bigint& other) {
        *this = *this - (*this / other) * other;
        return *this;
    }

    constexpr fixed_bigint& operator|=(const fixed_bigint& other) noexcept {
        for (uint32_t i = 0; i < kSize; ++i)
            SetChunk(i, GetChunk(i) | other.GetChunk(i));
        SetSgn(GetSgn() | other.GetSgn());
        return *this;
    }

    constexpr fixed_bigint& operator&=(const fixed_bigint& other) noexcept {
        for (uint32_t i = 0; i < kSize; ++i)
            SetChunk(i, GetChunk(i) & other.GetChunk(i));
        SetSgn(GetSgn() | other.GetSgn());
        return *this;
    }

    constexpr fixed_bigint& operator^=(const fixed_bigint& other) noexcept {
        for (uint32_t i = 0; i < kSize; ++i)
            SetChunk(i, GetChunk(i) ^ other.GetChunk(i));
        SetSgn(GetSgn() | other.GetSgn());
        return *this;
    }

    [[nodiscard]] friend constexpr fixed_bigint operator+(fixed_bigint lhs, const fixed_bigint& rhs) noexcept {
        lhs += rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr fixed_bigint operator-(fixed_bigint lhs, const fixed_bigint& rhs) noexcept {
        lhs -= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr fixed_bigint operator*(fixed_bigint lhs, const fixed_bigint& rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr fixed_bigint operator/(fixed_bigint lhs, const fixed_bigint& rhs) {
        lhs /= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr fixed_bigint operator%(fixed_bigint lhs, const fixed_bigint& rhs) {
        lhs %= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr fixed_bigint operator|(fixed_bigint lhs, const fixed_bigint& rhs) noexcept {
        lhs |= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr fixed_bigint operator&(fixed_bigint lhs, const fixed_bigint& rhs) noexcept {
        lhs &= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr fixed_bigint operator^(fixed_bigint lhs, const fixed_bigint& rhs) noexcept {
        lhs ^= rhs;
        return lhs;
    }

    [[nodiscard]] friend constexpr bool operator==(const fixed_bigint& lhs, const fixed_bigint& rhs) noexcept {
        if (lhs.GetSgn() != rhs.GetSgn())
            return false;
        for (uint32_t i = 0; i < kSize; ++i) {
            if (lhs.GetChunk(i) != rhs.GetChunk(i))
                return false;
        }
        return true;
    }
    [[nodiscard]] friend constexpr bool operator!=(const fixed_bigint& lhs, const fixed_bigint& rhs) noexcept {
        return !(lhs == rhs);
    }
    [[nodiscard]] friend constexpr bool operator<(const fixed_bigint& lhs, const fixed_bigint& rhs) noexcept {
        if (lhs.GetSgn() != rhs.GetSgn())
            return lhs.GetSgn();

        if (lhs.GetSgn()) {
            for (int32_t i = kSize - 1; i >= 0; --i) {
                if (lhs.GetChunk(i) > rhs.GetChunk(i))
                    return true;
                if (lhs.GetChunk(i) < rhs.GetChunk(i))
                    return false;
            }
        } else {
            for (int32_t i = kSize - 1; i >= 0; --i) {
                if (lhs.GetChunk(i) < rhs.GetChunk(i))
                    return true;
                if (lhs.GetChunk(i) > rhs.GetChunk(i))
                    return false;
            }
        }
        return false;
    }
    [[nodiscard]] friend constexpr bool operator>(const fixed_bigint& lhs, const fixed_bigint& rhs) noexcept {
        return rhs < lhs;
    }
    [[nodiscard]] friend constexpr bool operator<=(const fixed_bigint& lhs, const fixed_bigint& rhs) noexcept {
        return !(rhs < lhs);
    }
    [[nodiscard]] friend constexpr bool operator>=(const fixed_bigint& lhs, const fixed_bigint& rhs) noexcept {
        return !(lhs < rhs);
    }

    constexpr fixed_bigint& SelfLeftShift(uint32_t k) noexcept {
        if (k == 0)
            return *this;
        uint32_t cnt_chunk = k / 32;
        k %= 32;

        if (cnt_chunk > 0) {
            for (int32_t i = kSize - 1; i >= 0; --i) {
                SetChunk(i, (i >= cnt_chunk) ? GetChunk(i - cnt_chunk) : 0);
            }
        }
        if (k > 0) {
            for (int32_t i = kSize - 1; i >= 0; --i) {
                SetChunk(i, GetChunk(i) << k);
                if (i > 0) {
                    SetChunk(i, GetChunk(i) | (GetChunk(i - 1) >> (32 - k)));
                }
            }
        }
        return *this;
    }

    constexpr fixed_bigint& SelfRightShift(uint32_t k) noexcept {
        if (k == 0)
            return *this;
        uint32_t cnt_chunk = k / 32;
        k %= 32;

        if (cnt_chunk > 0) {
            for (uint32_t i = 0; i < kSize; ++i) {
                SetChunk(i, (i + cnt_chunk < kSize) ? GetChunk(i + cnt_chunk) : 0);
            }
        }
        if (k > 0) {
            for (uint32_t i = 0; i < kSize; ++i) {
                SetChunk(i, GetChunk(i) >> k);
                if (i < kSize - 1) {
                    SetChunk(i, GetChunk(i) | (GetChunk(i + 1) << (32 - k)));
                }
            }
        }
        return *this;
    }

    [[nodiscard]] constexpr fixed_bigint LeftShift(uint32_t k) const noexcept {
        fixed_bigint result = *this;
        return result.SelfLeftShift(k);
    }

    [[nodiscard]] constexpr fixed_bigint RightShift(uint32_t k) const noexcept {
        fixed_bigint result = *this;
        return result.SelfRightShift(k);
    }

    [[nodiscard]] std::string ToBinString() const {
        std::string result;
        result.push_back('0' + GetSgn());

        for (int32_t i = kSize - 1; i >= 0; --i) {
            uint32_t chunk = GetChunk(i);
            int32_t bits_to_process = (i == kSize - 1 && Bits % 32 != 0) ? (Bits % 32) - 1 : 31;
            for (int32_t bit = bits_to_process; bit >= 0; --bit) {
                result.push_back('0' + ((chunk >> bit) & 1));
            }
        }
        return result;
    }

    [[nodiscard]] std::string ToString() const {
        struct BigInt {
            std::string buff;
            BigInt() : buff(1, '0') {}
            BigInt& operator+=(const BigInt& other) {
                bool next = false;
                size_t max_len = std::max(buff.size(), other.buff.size());
                buff.resize(max_len, '0');

                for (size_t i = 0; i < other.buff.size() || next; ++i) {
                    if (i == buff.size())
                        buff.push_back('0');
                    int val = buff[i] - '0' + (i < other.buff.size() ? other.buff[i] - '0' : 0) + next;
                    next = val >= 10;
                    if (next)
                        val -= 10;
                    buff[i] = '0' + val;
                }
                return *this;
            }
        };

        BigInt result;
        BigInt p;
        p.buff[0] = '1';

        fixed_bigint my_copy = *this;
        bool sgn = my_copy.GetSgn();
        if (sgn)
            my_copy.RevSgn();

        std::string bits = my_copy.ToBinString();

        for (int32_t i = static_cast<int32_t>(bits.length()) - 1; i > 0; --i) {
            if (bits[i] == '1')
                result += p;
            p += p;
        }

        std::string final_str;
        if (sgn)
            final_str.push_back('-');
        for (auto it = result.buff.rbegin(); it != result.buff.rend(); ++it) {
            final_str.push_back(*it);
        }
        return final_str;
    }

    [[nodiscard]] std::string ToHexString() const {
        if (*this == 0)
            return "0";

        fixed_bigint num = *this;
        bool neg = false;
        if (num.GetSgn()) {
            neg = true;
            num.RevSgn();
        }

        std::string hex;
        while (num > 0) {
            uint32_t rem = (num % 16).GetChunk(0);
            hex += "0123456789ABCDEF"[rem];
            num = num / 16;
        }

        if (neg)
            hex += '-';
        std::reverse(hex.begin(), hex.end());
        return hex;
    }

    [[nodiscard]] std::string ToOctString() const {
        if (*this == 0)
            return "0";

        fixed_bigint num = *this;
        bool neg = false;
        if (num.GetSgn()) {
            neg = true;
            num.RevSgn();
        }

        std::string oct;
        while (num > 0) {
            uint32_t rem = (num % 8).GetChunk(0);
            oct += '0' + rem;
            num = num / 8;
        }

        if (neg)
            oct += '-';
        std::reverse(oct.begin(), oct.end());
        return oct;
    }

    [[nodiscard]] constexpr int32_t HighestOneBit() const noexcept {
        for (int32_t index = kSize - 1; index >= 0; --index) {
            uint32_t byte = GetChunk(index);
            if (byte > 0) {
                int32_t ost = 31;
                while ((byte & (1U << ost)) == 0)
                    --ost;
                return index * 32 + ost;
            }
        }
        return -1;
    }

    [[nodiscard]] constexpr bool GetSgn() const noexcept { return sgn_; }

    friend std::ostream& operator<<(std::ostream& stream, const fixed_bigint& value) {
        if (stream.flags() & std::ios::hex) {
            stream << value.ToHexString();
        } else if (stream.flags() & std::ios::oct) {
            stream << value.ToOctString();
        } else {
            stream << value.ToString();
        }
        return stream;
    }

  private:
    std::array<uint32_t, kSize> chunks_{0};
    bool sgn_{false};

    [[nodiscard]] constexpr uint32_t GetChunk(uint32_t index) const noexcept { return chunks_[index]; }

    constexpr void SetChunk(uint32_t index, uint32_t value) noexcept {
        if (index == kSize - 1) {
            chunks_[index] = value & kTopChunkMask;
        } else {
            chunks_[index] = value;
        }
    }

    constexpr void SetSgn(bool sgn) noexcept { sgn_ = sgn; }

    constexpr fixed_bigint& RevSgn() noexcept {
        if (*this == fixed_bigint(0)) {
            sgn_ = false;
            return *this;
        }
        *this = ~(*this);
        *this = *this + fixed_bigint(1);
        return *this;
    }
};

} // namespace fbi