#pragma once
#include <cstdint>
#include <iostream>
#include <ostream>

struct int2025_t {
  static const uint32_t kSize = 254;

  friend std::ostream& operator<<(std::ostream& stream, const int2025_t& value);

  int2025_t();
  int2025_t(int64_t value);
  int2025_t(int32_t value);
  int2025_t(const char* str);
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
  int2025_t& operator=(const int64_t& value);
  int2025_t& operator=(const char* str);

  char* ToBinString() const;
  char* ToString() const;
  char* ToHexString() const;
  int64_t ToInt64() const;
  uint8_t GetSgn() const;
  int2025_t& SelfLeftShift(uint32_t k);
  int2025_t& SelfRightShift(uint32_t k);
  int2025_t LeftShift(uint32_t k) const;
  int2025_t RightShift(uint32_t k) const;
  int32_t GetLowerPow() const;
  int32_t GetHightPow() const;

 private:
  uint8_t arrBytes_[kSize]{0};

  const uint8_t& GetChunk(uint32_t index) const;
  void SetChunk(uint32_t index, uint8_t value);
  void SetChunk(uint32_t index, const char* value);
  int2025_t& RevSgn();
};

static_assert(sizeof(int2025_t) <= 254,
              "Size of int2025_t must be no higher than 254 bytes");

std::ostream& operator<<(std::ostream& stream, const int2025_t& value);

int2025_t from_string(const char* str);

int2025_t from_int(uint32_t value);