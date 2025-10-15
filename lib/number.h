#pragma once
#include <cinttypes>
#include <iostream>

struct int2025_t {
  static const uint32_t kSize = 254;

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

  char* toBinString() const;
  char* toString() const;
  int64_t toInt64() const;
  uint8_t getSgn() const;
  int2025_t& selfLeftShift(uint32_t k);
  int2025_t& selfRightShift(uint32_t k);
  int2025_t leftShift(uint32_t k) const;
  int2025_t rightShift(uint32_t k) const;
  int32_t getLowerPow() const;
  int32_t getHightPow() const;

 private:
  uint8_t arrBytes[kSize]{0};

  const uint8_t& getChunk(uint32_t index) const;
  void setChunk(uint32_t index, uint8_t value);
  void setChunk(uint32_t index, const char* value);
  template <class T>
  int2025_t binaryOperation(const int2025_t& other, T f) const;
  int2025_t& revSgn();
};

static_assert(sizeof(int2025_t) <= 254,
              "Size of int2025_t must be no higher than 254 bytes");

std::ostream& operator<<(std::ostream& stream, const int2025_t& value);

int2025_t from_string(const char* str);

int2025_t from_int(uint32_t value);