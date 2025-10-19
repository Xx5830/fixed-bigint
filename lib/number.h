#pragma once
#include <cstdint>
#include <iostream>
#include <ostream>

#pragma pack(push, 2)
struct int2025_t {
  static const uint32_t kSize = 64;

  friend std::ostream& operator<<(std::ostream& stream, const int2025_t& value);

  int2025_t();
  int2025_t(int64_t value);
  explicit int2025_t(uint64_t value);
  explicit int2025_t(const char* str);
  int2025_t(const int2025_t& other);

  int2025_t operator~() const;
  int2025_t operator-() const;
  int2025_t operator|(const int2025_t& other) const;
  int2025_t operator&(const int2025_t& other) const;
  int2025_t operator^(const int2025_t& other) const;
  int2025_t operator+(const int2025_t& other) const;
  int2025_t operator-(const int2025_t& other) const;
  int2025_t operator*(const int2025_t& other) const;
  int2025_t operator/(const int2025_t& other) const;
  int2025_t operator%(const int2025_t& other) const;
  int2025_t& operator|=(const int2025_t& other);
  int2025_t& operator&=(const int2025_t& other);
  int2025_t& operator^=(const int2025_t& other);
  int2025_t& operator+=(const int2025_t& other);
  int2025_t& operator-=(const int2025_t& other);
  int2025_t& operator*=(const int2025_t& other);
  int2025_t& operator/=(const int2025_t& other);
  int2025_t& operator%=(const int2025_t& other);
  bool operator==(const int2025_t& other) const;
  bool operator!=(const int2025_t& other) const;
  bool operator<(const int2025_t& other) const;
  bool operator>(const int2025_t& other) const;
  bool operator<=(const int2025_t& other) const;
  bool operator>=(const int2025_t& other) const;
  int2025_t& operator=(const int2025_t& other);

  char* ToBinString() const;
  char* ToString() const;
  char* ToHexString() const;
  char* ToOctString() const;
  int64_t ToInt64() const;
  uint8_t GetSgn() const;
  int2025_t& SelfLeftShift(uint32_t k);
  int2025_t& SelfRightShift(uint32_t k);
  int2025_t LeftShift(uint32_t k) const;
  int2025_t RightShift(uint32_t k) const;
  int32_t LowestOneBit(uint32_t left = 0) const;
  int32_t HighestOneBit(uint32_t right = kSize - 1) const;

 private:
  uint32_t arrBytes_[kSize - 1]{0};
  uint8_t byte_{0};
  bool sgn_{0};

  uint32_t GetChunk(uint32_t index) const;
  void SetChunk(uint32_t index, uint32_t value);
  void SetSgn(bool sgn);
  int2025_t& RevSgn();
};
#pragma pack(pop)

static_assert(sizeof(int2025_t) <= 254,
              "Size of int2025_t must be no higher than 254 bytes");

std::ostream& operator<<(std::ostream& stream, const int2025_t& value);

int2025_t from_string(const char* str);

int2025_t from_int(int64_t value);