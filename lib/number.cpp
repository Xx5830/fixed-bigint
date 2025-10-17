#include "number.h"
#include <cinttypes>
#include <tuple>
#include <utility>
#include <string>
#include <iostream>
#include <cstdint>
#include <ostream>
#include <cmath>

// --- Constructions

int2025_t::int2025_t() {}

int2025_t::int2025_t(int64_t value) {
  bool swap_sgn = 0;
  if (value < 0) {
    swap_sgn = 1;
    value = -value;
  }
  int64_t copy_value = value;

  for (uint32_t index_byte = 0; index_byte < 8; index_byte++) {
    int64_t current = 0;

    for (uint32_t k = 0; k < 8; k++) {
      current += (copy_value & 1) << k;
      copy_value >>= 1;
    }

    SetChunk(index_byte, current);
  }

  if (swap_sgn) {
    RevSgn();
  }

  SetChunk(kSize - 1, GetChunk(kSize - 1) & 0b00000011);
}

int2025_t::int2025_t(int32_t value) {
  bool swap_sgn = 0;
  if (value < 0) {
    swap_sgn = 1;
    value = -value;
  }
  int32_t copy_value = value;

  for (uint32_t index_byte = 0; index_byte < 4; index_byte++) {
    int32_t current = 0;

    for (uint32_t k = 0; k < 8; k++) {
      current += (copy_value & 1) << k;
      copy_value >>= 1;
    }

    SetChunk(index_byte, current);
  }

  if (swap_sgn) {
    RevSgn();
  }

  SetChunk(kSize - 1, GetChunk(kSize - 1) & 0b00000011);
}

int2025_t::int2025_t(const char* str) {
  uint32_t index = 0;
  bool sgn = 0;
  if (str[index] == '-'){
    sgn = 1;
    ++index;
  }
  if (str[index] == '+'){
    ++index;
  }

  while (str[index] != '\0'){
    int64_t buff = 0;
    int64_t step = 1;

    while (str[index] != '\0' && step < 1e18){
      buff *= 10;
      buff += str[index] - '0';

      ++index;
      step *= 10;
    }

    *this *= step;
    *this += buff;
  }

  if (sgn && *this != 0){
    RevSgn();
  }
}

int2025_t::int2025_t(const int2025_t& other) { *this = other; }

// --- private Methods

const uint8_t& int2025_t::GetChunk(uint32_t index) const {
  return arrBytes[index];
}

void int2025_t::SetChunk(uint32_t index, uint8_t value) {
  arrBytes[index] = value;
}

void int2025_t::SetChunk(uint32_t index, const char* value) {
  uint32_t current = 0;
  for (uint32_t index = 0; index < 8; index++) {
    current <<= 1;
    if (value[index] == '1') {
      current += 1;
    }
  }

  SetChunk(index, current);
}

template <class T>
int2025_t int2025_t::BinaryOperation(const int2025_t& other, T f) const {
  int2025_t result;

  for (uint32_t index = 0; index < kSize; index++) {
    result.SetChunk(index, f(this->GetChunk(index), other.GetChunk(index)));
  }

  return result;
}

int2025_t& int2025_t::RevSgn() {
  *this = ~(*this);
  *this = (*this) + 1;

  return *this;
}

// --- public Logic Operations

int2025_t int2025_t::operator~() const {
  int2025_t result;

  for (uint32_t index = 0; index < int2025_t::kSize; index++) {
    result.SetChunk(index, ~this->GetChunk(index));
  }

  return result;
}

int2025_t int2025_t::operator-() const {
  int2025_t result = *this;
  return result.RevSgn();
}

int2025_t int2025_t::operator|(const int2025_t& other) const {
  return BinaryOperation(other, [](uint8_t a, uint8_t b) { return a | b; });
}

int2025_t int2025_t::operator&(const int2025_t& other) const {
  return BinaryOperation(other, [](uint8_t a, uint8_t b) { return a & b; });
}

int2025_t int2025_t::operator^(const int2025_t& other) const {
  return BinaryOperation(other, [](uint8_t a, uint8_t b) { return a ^ b; });
}

// --- public Arithmetics Operations

int2025_t int2025_t::operator+(const int2025_t& other) const {
  int2025_t result = *this;

  return result += other;
}

int2025_t int2025_t::operator-(const int2025_t& other) const {
  int2025_t result = other;
  result = *this + result.RevSgn();
  return result;
}

int2025_t int2025_t::operator*(const int2025_t& other) const {
  int2025_t my_copy = *this;
  int2025_t other_copy = other;
  uint8_t final_sgn = my_copy.GetSgn() ^ other_copy.GetSgn();
  if (my_copy.GetSgn()) {
    my_copy = my_copy.RevSgn();
  }
  if (other_copy.GetSgn()) {
    other_copy = other_copy.RevSgn();
  }
  int2025_t result = 0;

  while (other_copy.GetLowerPow() != -1) {
    if (other_copy.GetChunk(0) & 1) {
      result += my_copy;
    }

    my_copy.SelfLeftShift(1);
    other_copy.SelfRightShift(1);
  }

  if (final_sgn) {
    result = result.RevSgn();
  }
  result.SetChunk(kSize - 1, result.GetChunk(kSize - 1) & 0b00000011);
  return result;
}

int2025_t int2025_t::operator/(const int2025_t& other) const {
  int2025_t my_copy = *this;
  int2025_t other_copy = other;
  uint8_t final_sgn = my_copy.GetSgn() ^ other_copy.GetSgn();
  if (my_copy.GetSgn()) {
    my_copy = my_copy.RevSgn();
  }
  if (other_copy.GetSgn()) {
    other_copy = other_copy.RevSgn();
  }
  int2025_t result = 0;

  if (my_copy < other_copy) {
    return result;
  }
  if (other_copy == 0) {
    return 0;
  }

  if (my_copy >= other_copy) {
    int32_t dif = my_copy.GetHightPow() - other_copy.GetHightPow();
    int32_t dif_pos = dif;

    int2025_t hight_bit = 1;
    hight_bit.SelfLeftShift(dif);
    other_copy.SelfLeftShift(dif);

    while (dif_pos >= 0) {
      while (dif_pos >= 0 && other_copy > my_copy) {
        other_copy.SelfRightShift(1);
        hight_bit.SelfRightShift(1);
        --dif_pos;
      }

      if (dif_pos < 0) {
        break;
      }

      result += hight_bit;
      my_copy -= other_copy;
    }
  }

  if (final_sgn) {
    result.RevSgn();
  }
  return result;
}

int2025_t int2025_t::operator%(const int2025_t& other) const {
  return *this - (*this / other) * other;
}

int2025_t& int2025_t::operator|=(const int2025_t& other) {
  *this = *this | other;

  return *this;
}

int2025_t& int2025_t::operator&=(const int2025_t& other) {
  *this = *this & other;

  return *this;
}

int2025_t& int2025_t::operator^=(const int2025_t& other) {
  *this = *this ^ other;

  return *this;
}

int2025_t& int2025_t::operator+=(const int2025_t& other) {
  bool adds = 0;
  for (int index_byte = 0; index_byte < kSize; index_byte++) {
    uint8_t byte_a = this->GetChunk(index_byte);
    uint8_t byte_b = other.GetChunk(index_byte);
    uint8_t result_byte = byte_a + byte_b;

    bool next_adds = 0;
    if (result_byte < byte_a || result_byte < byte_b) {
      next_adds = 1;
    }

    if (adds && result_byte == 0b11111111) {
      next_adds = 1;
    }
    result_byte += adds & 1;
    adds = next_adds;

    this->SetChunk(index_byte, result_byte);
  }

  this->SetChunk(kSize - 1, this->GetChunk(kSize - 1) & 0b00000011);

  return *this;
}

int2025_t& int2025_t::operator-=(const int2025_t& other) {
  *this = *this - other;

  return *this;
}

int2025_t& int2025_t::operator*=(const int2025_t& other) {
  *this = *this * other;

  return *this;
}

int2025_t& int2025_t::operator/=(const int2025_t& other) {
  *this = *this / other;

  return *this;
}

int2025_t& int2025_t::operator%=(const int2025_t& other) {
  *this = *this % other;

  return *this;
}

// --- public Boolean Operations

bool int2025_t::operator==(const int2025_t& other) const {
  for (uint32_t index = 0; index + 1 < kSize; index++) {
    if (GetChunk(index) != other.GetChunk(index)) {
      return false;
    }
  }

  uint8_t byte_a = GetChunk(kSize - 1);
  uint8_t byte_b = other.GetChunk(kSize - 1);

  if ((byte_a & 1) != (byte_b & 1)) {
    return false;
  } else if ((byte_a & 2) != (byte_b & 2)) {
    return false;
  }

  return true;
}

bool int2025_t::operator!=(const int2025_t& other) const {
  return !(*this == other);
}

bool int2025_t::operator<(const int2025_t& other) const {
  if (GetSgn() != other.GetSgn()) {
    return GetSgn() == 1;
  }

  if (GetSgn() == 1) {
    uint8_t byte_a = GetChunk(kSize - 1);
    uint8_t byte_b = other.GetChunk(kSize - 1);

    if (byte_a & 1 > byte_b & 1) {
      return true;
    }

    for (int32_t index = kSize - 2; index >= 0; index--) {
      if (GetChunk(index) > other.GetChunk(index)) {
        return true;
      }
      else if (GetChunk(index) < other.GetChunk(index)) {
        return false;;
      }
    }

    

    return false;
  } else {
    uint8_t byte_a = GetChunk(kSize - 1);
    uint8_t byte_b = other.GetChunk(kSize - 1);

    if (byte_a & 1 < byte_b & 1) {
      return true;
    }

    for (int32_t index = kSize - 2; index >= 0; index--) {
      if (GetChunk(index) < other.GetChunk(index)) {
        return true;
      }
      if (GetChunk(index) > other.GetChunk(index)){
        return false;
      }
    }

    

    return false;
  }
}

bool int2025_t::operator>(const int2025_t& other) const {
  return !(*this < other) && *this != other;
}

bool int2025_t::operator<=(const int2025_t& other) const {
  return !(*this > other);
}

bool int2025_t::operator>=(const int2025_t& other) const {
  return !(*this < other);
}

// --- public Relation Operations

int2025_t& int2025_t::operator=(const int2025_t& other) {
  for (uint32_t index = 0; index < kSize; index++) {
    SetChunk(index, other.GetChunk(index));
  }

  return *this;
}

int2025_t& int2025_t::operator=(const int64_t& value) {
  return *this = int2025_t(value);
}

int2025_t& int2025_t::operator=(const char* str) {
  return *this = int2025_t(str);
}

// --- public Methods

char* int2025_t::ToBinString() const {
  char* result = new char[2025]{0};
  char rev_result[2025];

  for (uint32_t index_byte = 0; index_byte < kSize; index_byte++) {
    uint8_t byte = GetChunk(index_byte);
    for (uint32_t index_bit = 0; index_bit < 8 && index_byte * 8 + index_bit < 2025;
         index_bit++) {
      char bit = byte & 1;
      byte >>= 1;

      rev_result[index_byte * 8 + index_bit] = '0' + bit;
    }
  }

  for (uint32_t index = 0; index < 2025; index++) {
    result[index] = rev_result[2024 - index];
  }

  return result;
}

char* int2025_t::ToString() const {
  struct BigInt {
    char buff[1000]{48};
    uint32_t end = 1;

    BigInt() {
      for (uint32_t index = 0; index < 1000; index++) {
        buff[index] = '0';
      }
    }

    BigInt& operator+=(BigInt other) {
      bool next = 0;
      for (uint32_t index = 0; index < other.end || next; index++) {
        next = 0;
        buff[index] += other.buff[index] - '0';

        if (buff[index] > '9') {
          buff[index] -= 10;
          buff[index + 1]++;
          next = 1;
        }

        end = std::max(end, index + 1);
      }

      return *this;
    }
  };
  BigInt result;
  BigInt p;
  p.buff[p.end - 1] = '1';

  int2025_t my_copy = *this;

  uint8_t sgn = my_copy.GetSgn();
  if (sgn) {
    my_copy.RevSgn();
  }

  char* bits = my_copy.ToBinString();

  for (int32_t index = 2024; index >= 0; index--) {
    if (bits[index] == '1') {
      result += p;
    }

    p += p;
  }

  char* char_result;
  if (!sgn) {
    char_result = new char[result.end + 1];
    for (int32_t index_result = result.end - 1, index_char_result = 0;
         index_result >= 0; index_result--, index_char_result++) {
      char_result[index_char_result] = result.buff[index_result];
    }
    char_result[result.end] = '\0';
  } else {
    char_result = new char[result.end + 2];
    char_result[0] = '-';
    for (int32_t index_result = result.end - 1, index_char_result = 1;
         index_result >= 0; index_result--, index_char_result++) {
      char_result[index_char_result] = result.buff[index_result];
    }
    char_result[result.end + 1] = '\0';
  }

  delete[] bits;
  return char_result;
}

int64_t int2025_t::ToInt64() const {
  // Задать вопрос про реализацию через указатели
  int64_t result = 0;
  for (uint32_t k = 0; k < 8; k++) {
    result += (int64_t)GetChunk(k) << (8 * k);
  }

  int64_t sgn = GetSgn();

  result -= (result & ((int64_t)1 << 63));
  result += sgn << 63;

  return result;
}

uint8_t int2025_t::GetSgn() const { return (GetChunk(kSize - 1) & 2) >> 1; }

int32_t int2025_t::GetLowerPow() const {
  if (!GetSgn()) {
    for (uint32_t index = 0; index + 1 < kSize; index++) {
      uint8_t byte = GetChunk(index);

      for (uint32_t k = 0; k < 8; k++) {
        uint8_t bit = (1 << k) & byte;

        if (bit > 0) {
          return index * 8 + k;
        }
      }
    }

    if (GetChunk(kSize - 1) & 1) {
      return (int32_t)(kSize - 1) * 8;
    }

    return -1;
  } else {
    int2025_t copy_this = *this;
    copy_this.RevSgn();

    for (uint32_t index = 0; index + 1 < kSize; index++) {
      uint8_t byte = copy_this.GetChunk(index);

      for (uint32_t k = 0; k < 8; k++) {
        uint8_t bit = (1 << k) & byte;

        if (bit == 0) {
          return index * 8 + k;
        }
      }
    }

    if (!(copy_this.GetChunk(kSize - 1) & 1)) {
      return (int32_t)(kSize - 1) * 8;
    }

    return -1;
  }
}

int32_t int2025_t::GetHightPow() const {
  if (!GetSgn()) {
    if (GetChunk(kSize - 1) & 1) {
      return (int32_t)(kSize - 1) * 8;
    }

    for (int32_t index = kSize - 1; index >= 0; index--) {
      uint8_t byte = GetChunk(index);

      for (int32_t k = 7; k >= 0; k--) {
        uint8_t bit = (1 << k) & byte;

        if (bit > 0) {
          return index * 8 + k;
        }
      }
    }

    return -1;
  } else {
    if (!(GetChunk(kSize - 1) & 1)) {
      return (int32_t)(kSize - 1) * 8;
    }

    for (int32_t index = kSize - 1; index >= 0; index--) {
      uint8_t byte = GetChunk(index);

      for (int32_t k = 7; k >= 0; k--) {
        uint8_t bit = (1 << k) & byte;

        if (bit == 0) {
          return index * 8 + k;
        }
      }
    }

    return -1;
  }
}

int2025_t& int2025_t::SelfLeftShift(uint32_t k) {
  if (k == 0) {
    return *this;
  }

  uint32_t cnt_byte = k / 8;

  if (cnt_byte > 0) {
    for (int32_t index = kSize - 1; index >= 0; index--) {
      if (index + cnt_byte < kSize) {
        SetChunk(index + cnt_byte, GetChunk(index));
      }

      SetChunk(index, (uint8_t)0);
    }
  }

  k -= cnt_byte * 8;

  for (int32_t index = kSize - 1; index >= 0; index--) {
    SetChunk(index, GetChunk(index) << k);

    if (index > 0) {
      uint8_t left_bit = GetChunk(index - 1);

      uint8_t mask = left_bit >> (8 - k);
      SetChunk(index, GetChunk(index) + mask);
    }
  }

  return *this;
}

int2025_t& int2025_t::SelfRightShift(uint32_t k) {
  if (k == 0) {
    return *this;
  }

  uint32_t cnt_byte = k / 8;

  if (cnt_byte > 0) {
    for (int32_t index = 0; index < kSize; index++) {
      if (index - cnt_byte >= 0) {
        SetChunk(index - cnt_byte, GetChunk(index));
      }

      SetChunk(index, (uint8_t)0);
    }
  }

  k -= cnt_byte * 8;

  for (int32_t index = 0; index < kSize; index++) {
    SetChunk(index, GetChunk(index) >> k);

    if (index < kSize - 1) {
      uint8_t right_bit = GetChunk(index + 1);

      uint8_t mask = right_bit << (8 - k);
      SetChunk(index, GetChunk(index) + mask);
    }
  }

  return *this;
}

int2025_t int2025_t::LeftShift(uint32_t k) const {
  int2025_t result = *this;
  return result.SelfLeftShift(k);
}

int2025_t int2025_t::RightShift(uint32_t k) const {
  int2025_t result = *this;
  return result.SelfRightShift(k);
}

// --- External Operations

std::ostream& operator<<(std::ostream& stream, const int2025_t& value) {
  for (uint32_t index = 0; index < value.kSize; index++){
    stream << value.GetChunk(index);
  }

  return stream;
}

int2025_t from_string(const char* str) { return int2025_t(str); }

int2025_t from_int(uint32_t value) { return int2025_t((int32_t)value); }
