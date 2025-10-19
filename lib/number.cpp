#include "number.h"

#include <cmath>
#include <cstdint>
#include <iostream>

// --- Constructions

int2025_t::int2025_t() {}

int2025_t::int2025_t(int64_t value) {
  bool swap_sgn = 0;
  if (value < 0) {
    swap_sgn = 1;
    value = -value;
  }

  SetChunk(0, value);
  SetChunk(1, value >> 32);

  if (swap_sgn) {
    RevSgn();
  }
}

int2025_t::int2025_t(uint64_t value) {
  SetChunk(0, value);
  SetChunk(1, value >> 32);
}

int2025_t::int2025_t(const char* str) {
  uint32_t index = 0;
  bool sgn = 0;
  if (str[index] == '-') {
    sgn = 1;
    ++index;
  }
  if (str[index] == '+') {
    ++index;
  }

  while (str[index] != '\0') {
    uint64_t buff = 0;
    uint64_t step = 1;

    while (str[index] != '\0' && step < 1e18) {
      buff *= 10;
      buff += str[index] - '0';

      ++index;
      step *= 10;
    }

    *this *= step;
    *this += buff;
  }

  if (sgn && *this != 0) {
    RevSgn();
  }
}

int2025_t::int2025_t(const int2025_t& other) { *this = other; }

// --- private Methods

uint32_t int2025_t::GetChunk(uint32_t index) const {
  if (index + 1 == kSize) {
    return byte_;
  } else {
    return arrBytes_[index];
  }
}

void int2025_t::SetChunk(uint32_t index, uint32_t value) {
  if (index + 1 == kSize) {
    byte_ = (uint8_t)value;
  } else {
    arrBytes_[index] = value;
  }
}

void int2025_t::SetSgn(bool sgn) { sgn_ = sgn & 1; }

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
  result.SetSgn((bool)(1 - this->GetSgn()));

  return result;
}

int2025_t int2025_t::operator-() const {
  int2025_t result = *this;
  return result.RevSgn();
}

int2025_t int2025_t::operator|(const int2025_t& other) const {
  int2025_t result = *this;
  result |= other;

  return result;
}

int2025_t int2025_t::operator&(const int2025_t& other) const {
  int2025_t result = *this;
  result &= other;

  return result;
}

int2025_t int2025_t::operator^(const int2025_t& other) const {
  int2025_t result = *this;
  result ^= other;

  return result;
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

  int8_t where_dp[256];
  for (uint32_t index = 0; index < 256; index++){
    where_dp[index] = -1;
  }
  int2025_t *dp[256];
  uint32_t end_dp = 0;

  for (uint32_t index_chunk = 0; index_chunk < kSize; index_chunk++){
    uint32_t chunk = other_copy.GetChunk(index_chunk);

    for (uint32_t index_byte = 0; index_byte < 4; index_byte++){
      uint8_t byte = chunk & 0b11111111;
      chunk >>= 8;

      if (where_dp[byte] == -1){
        dp[end_dp] = new int2025_t;
        where_dp[byte] = end_dp;

        uint8_t copy_byte = byte;
        for (uint32_t index = 0; index < 8; index++){
          if (copy_byte & 1){
            *dp[end_dp] += my_copy.LeftShift(index);
          }
          copy_byte >>= 1;
        }

        ++end_dp;
      }  

      result += dp[where_dp[byte]]->LeftShift(index_chunk * 32 + index_byte * 8);
    }
  }

  if (final_sgn) {
    result = result.RevSgn();
  }

  for (uint32_t index = 0; index < end_dp; index++){
    delete dp[index];
  }
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
    int32_t dif = my_copy.HighestOneBit() - other_copy.HighestOneBit();
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
  for (uint32_t index = 0; index < kSize; index++) {
    this->SetChunk(index, this->GetChunk(index) | other.GetChunk(index));
  }
  this->SetSgn(this->GetSgn() | other.GetSgn());

  return *this;
}

int2025_t& int2025_t::operator&=(const int2025_t& other) {
  for (uint32_t index = 0; index < kSize; index++) {
    this->SetChunk(index, this->GetChunk(index) & other.GetChunk(index));
  }
  this->SetSgn(this->GetSgn() | other.GetSgn());

  return *this;
}

int2025_t& int2025_t::operator^=(const int2025_t& other) {
  for (uint32_t index = 0; index < kSize; index++) {
    this->SetChunk(index, this->GetChunk(index) ^ other.GetChunk(index));
  }
  this->SetSgn(this->GetSgn() | other.GetSgn());

  return *this;
}

int2025_t& int2025_t::operator+=(const int2025_t& other) {
  bool adds = 0;
  for (int32_t index_chunk = 0; index_chunk < kSize - 1; index_chunk++) {
    uint32_t chunk_a = this->GetChunk(index_chunk);
    uint32_t chunk_b = other.GetChunk(index_chunk);
    uint32_t result_chunk = chunk_a + chunk_b;

    bool next_adds = 0;
    if (result_chunk < chunk_a || result_chunk < chunk_b) {
      next_adds = 1;
    }

    if (adds && result_chunk == (((int64_t)1 << 32) - 1)) {
      next_adds = 1;
    }
    result_chunk += adds & 1;
    adds = next_adds;

    this->SetChunk(index_chunk, result_chunk);
  }

  uint8_t chunk_a = this->GetChunk(kSize - 1);
  uint8_t chunk_b = other.GetChunk(kSize - 1);
  uint8_t result_chunk = chunk_a + chunk_b;

  bool next_adds = 0;
  if (result_chunk < chunk_a || result_chunk < chunk_b) {
    next_adds = 1;
  }

  if (adds && result_chunk == ((1 << 8) - 1)) {
    next_adds = 1;
  }
  result_chunk += adds & 1;
  adds = next_adds;

  this->SetChunk(kSize - 1, result_chunk);

  SetSgn((GetSgn() + other.GetSgn() + adds) % 2);

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
  for (uint32_t index = 0; index < kSize; index++) {
    if (GetChunk(index) != other.GetChunk(index)) {
      return false;
    }
  }

  return this->GetSgn() == other.GetSgn();
}

bool int2025_t::operator!=(const int2025_t& other) const {
  return !(*this == other);
}

bool int2025_t::operator<(const int2025_t& other) const {
  if (GetSgn() != other.GetSgn()) {
    return GetSgn();
  }

  if (GetSgn() == 1) {
    for (int32_t index = kSize - 1; index >= 0; index--) {
      uint32_t bytes1 = GetChunk(index);
      uint32_t bytes2 = other.GetChunk(index);

      if (bytes1 > bytes2) {
        return true;
      }
      if (bytes1 < bytes2) {
        return false;
      }
    }

    return false;
  } else {
    for (int32_t index = kSize - 1; index >= 0; index--) {
      uint32_t bytes1 = GetChunk(index);
      uint32_t bytes2 = other.GetChunk(index);

      if (bytes1 < bytes2) {
        return true;
      }
      if (bytes1 > bytes2) {
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
  SetSgn(other.GetSgn());

  return *this;
}

// --- public Methods

char* int2025_t::ToBinString() const {
  char* result = new char[2026];
  result[2025] = '\0';

  result[0] = '0' + GetSgn();

  uint32_t chunk = GetChunk(kSize - 1);
  for (int32_t index_bit = 7; index_bit >= 0; index_bit--) {
    uint8_t bit = (chunk & (1 << 7)) >> 7;
    chunk <<= 1;

    result[8 - index_bit] = '0' + bit;
  }

  for (int32_t index_chunk = kSize - 2, index_result = 9; index_chunk >= 0;
       index_chunk--) {
    uint32_t chunk = GetChunk(index_chunk);

    for (int32_t index_bit = 31; index_bit >= 0; index_bit--, index_result++) {
      uint8_t bit = (chunk & (1 << 31)) >> 31;
      chunk <<= 1;
      result[index_result] = '0' + bit;
    }
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

  for (int32_t index = 2024; index > 0; index--) {
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

char* int2025_t::ToHexString() const {
  if (GetSgn()) {
    int2025_t copy = *this;
    copy.RevSgn();

    uint32_t size_result = std::max(1, (copy.HighestOneBit() + 7) / 8);
    uint32_t cnt_chunk = (size_result + 3) / 4;
    char* result = new char[size_result + 2];

    result[0] = '-';
    for (int32_t index_chunk = cnt_chunk - 1, index_byte = size_result;
         index_chunk >= 0; index_chunk--) {
      uint32_t chunk = copy.GetChunk(index_chunk);

      for (int32_t part_chunk = 3; part_chunk >= 0; part_chunk--) {
        if (index_chunk == cnt_chunk - 1) {
          uint32_t need_chunk = size_result % 4;
          while (part_chunk > need_chunk) {
            chunk >>= 8;
            --part_chunk;
          }
        }
        uint8_t byte = chunk << 24;
        chunk >>= 8;

        uint8_t hex_a = byte & 0b00001111;
        uint8_t hex_b = byte >> 4;

        if (hex_a > 10) {
          result[index_byte] = 'A' + (hex_a - 10);
        } else {
          result[index_byte] = '0' + hex_a;
        }
        --index_byte;

        if (hex_b > 10) {
          result[index_byte] = 'A' + (hex_b - 10);
        } else {
          result[index_byte] = '0' + hex_b;
        }
        --index_byte;
      }
    }

    result[size_result] = '\0';
    return result;
  } else {
    uint32_t size_result = std::max(1, (HighestOneBit() + 7) / 8);
    uint32_t cnt_chunk = (size_result + 3) / 4;
    char* result = new char[size_result + 1];

    for (int32_t index_chunk = cnt_chunk - 1, index_byte = size_result - 1;
         index_chunk >= 0; index_chunk--) {
      uint32_t chunk = GetChunk(index_chunk);

      for (int32_t part_chunk = 3; part_chunk >= 0; part_chunk--) {
        if (index_chunk == cnt_chunk - 1) {
          uint32_t need_chunk = size_result % 4;
          while (part_chunk > need_chunk) {
            chunk >>= 8;
            --part_chunk;
          }
        }
        uint8_t byte = chunk << 24;
        chunk >>= 8;

        uint8_t hex_a = byte & 0b00001111;
        uint8_t hex_b = byte >> 4;

        if (hex_a > 10) {
          result[index_byte] = 'A' + (hex_a - 10);
        } else {
          result[index_byte] = '0' + hex_a;
        }
        --index_byte;

        if (hex_b > 10) {
          result[index_byte] = 'A' + (hex_b - 10);
        } else {
          result[index_byte] = '0' + hex_b;
        }
        --index_byte;
      }
    }

    result[size_result] = '\0';
    return result;
  }
}

//                               -improve
char* int2025_t::ToOctString() const {
  if (GetSgn()) {
    int2025_t copy_this = *this;
    copy_this.RevSgn();

    uint32_t size = copy_this.HighestOneBit();
    char* rev_buff = new char[(size + 2) / 3]{'\0'};
    char* str = copy_this.ToBinString();
    bool end = 0;

    for (uint32_t index = 0; index < size; index++) {
      uint8_t byte = copy_this.GetChunk(index);
      uint8_t bit1 = byte & 0b00001111;
      uint8_t bit2 = (byte & 0b11110000) >> 4;

      if (bit1 <= 9) {
        rev_buff[index * 2] = '0' + bit1;
      } else {
        rev_buff[index * 2] = 'A' + (bit1 - 10);
      }

      if (index != size - 1 || bit2 != 0) {
        if (bit2 <= 9) {
          rev_buff[index * 2 + 1] = '0' + bit2;
        } else {
          rev_buff[index * 2 + 1] = 'A' + (bit2 - 10);
        }

        end = 1;
      }
    }

    uint32_t result_size = size * 2 + end;
    char* result = new char[result_size + 1]{'\0'};
    result[0] = '-';
    for (int32_t index = 1; index < result_size; index++) {
      result[index] = rev_buff[result_size - index - 1];
    }
    result[result_size] = '\0';

    return result;
  } else {
    uint32_t size = std::max(1, (HighestOneBit() + 7) / 8);
    char* rev_buff = new char[size * 2]{'\0'};
    bool end = 0;

    for (uint32_t index = 0; index < size; index++) {
      uint8_t byte = GetChunk(index);
      uint8_t bit1 = byte & 0b00001111;
      uint8_t bit2 = (byte & 0b11110000) >> 4;

      if (bit1 <= 9) {
        rev_buff[index * 2] = '0' + bit1;
      } else {
        rev_buff[index * 2] = 'A' + (bit1 - 10);
      }

      if (index != size - 1 || bit2 != 0) {
        if (bit2 <= 9) {
          rev_buff[index * 2 + 1] = '0' + bit2;
        } else {
          rev_buff[index * 2 + 1] = 'A' + (bit2 - 10);
        }

        end = 1;
      }
    }

    uint32_t result_size = size * 2 - 1 + end;
    char* result = new char[result_size + 1];
    for (int32_t index = 0; index < result_size; index++) {
      result[index] = rev_buff[result_size - index - 1];
    }
    result[result_size] = '\0';

    return result;
  }
}

int64_t int2025_t::ToInt64() const {
  int64_t result = 0;
  result += GetChunk(0);
  result += (uint64_t)GetChunk(1) << 32;

  int64_t sgn = GetSgn();

  result -= (result & ((uint64_t)1 << 63));
  result += sgn << 63;

  return result;
}

uint8_t int2025_t::GetSgn() const { return sgn_; }

int32_t int2025_t::LowestOneBit(uint32_t left) const {
  for (uint32_t index = left; index < kSize; index++) {
    uint32_t byte = GetChunk(index);

    if (byte > 0) {
      uint32_t ost = 0;
      while ((byte & 1) == 0) {
        byte >>= 1;
        ++ost;
      }

      return index * 32 + ost;
    }
  }

  return -1;
}

int32_t int2025_t::HighestOneBit(uint32_t right) const {
  for (int32_t index = right; index >= 0; index--) {
    uint32_t byte = GetChunk(index);

    if (byte > 0) {
      uint32_t ost = 31;
      while ((byte & (1 << ost)) == 0) {
        --ost;
      }

      return index * 32 + ost;
    }
  }

  return -1;
}

int2025_t& int2025_t::SelfLeftShift(uint32_t k) {
  if (k == 0) {
    return *this;
  }

  uint32_t cnt_byte = k / 32;

  if (cnt_byte > 0) {
    SetChunk(kSize - 1, 0);
    SetSgn(0);

    for (int32_t index = kSize - 2; index >= 0; index--) {
      if (index + cnt_byte + 1 == kSize) {
        uint8_t byte = GetChunk(index) & 0b11111111;
        uint8_t bit = GetChunk(index) & 0b1000000000;
        SetSgn(bit);
        SetChunk(kSize - 1, byte);
      } else if (index + cnt_byte < kSize) {
        SetChunk(index + cnt_byte, GetChunk(index));
      }

      SetChunk(index, 0);
    }
  }

  k -= cnt_byte * 32;

  if (k > 0) {
    if (k <= 8) {
      uint8_t byte = GetChunk(kSize - 1);
      uint8_t index_bit = 8 - k;
      uint8_t bit = byte & (1 << index_bit);
      SetSgn(bit);
    }
    else{
      uint32_t chunk = GetChunk(kSize - 2);
      uint32_t index_bit = 32 - (k - 8);
      uint8_t bit = chunk & (1 << index_bit);
      SetSgn(bit);
    }

    for (int32_t index = kSize - 1; index >= 0; index--) {
      SetChunk(index, GetChunk(index) << k);

      if (index > 0) {
        uint32_t left_chunk = GetChunk(index - 1);

        uint32_t mask = left_chunk >> (32 - k);
        SetChunk(index, GetChunk(index) | mask);
      }
    }
  }

  return *this;
}

int2025_t& int2025_t::SelfRightShift(uint32_t k) {
  if (k == 0) {
    return *this;
  }

  uint32_t cnt_byte = k / 32;

  if (cnt_byte > 0) {
    for (int32_t index = 0; index < kSize; index++) {
      uint32_t chunk = GetChunk(index);
      if (index + 1 == kSize){
        chunk |= (GetSgn() << 8);
      }

      if (index - cnt_byte >= 0) {
        SetChunk(index - cnt_byte, chunk);
      }

      SetChunk(index, 0);
    }

    SetSgn(0);
  }

  k -= cnt_byte * 32;

  if (k > 0){
    for (int32_t index = 0; index < kSize; index++) {
      SetChunk(index, GetChunk(index) >> k);

      if (index == kSize - 1 && k <= 8){
        SetChunk(index, GetChunk(index) | (GetSgn() << (8 - k)));
      }

      if (index < kSize - 1) {
        uint32_t right_bit = GetChunk(index + 1);
        if (index + 1 == kSize - 1){
          right_bit |= GetSgn() << 8;
        }

        uint32_t mask = right_bit << (32 - k);
        SetChunk(index, GetChunk(index) | mask);
      }
    }

    SetSgn(0);
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
  uint32_t flag = stream.flags();
  static const uint32_t kOctFlag = 0b01000000;
  static const uint32_t kDecFlag = 0b00000010;
  static const uint32_t kHexFlag = 0b00001000;

  char* str;
  if ((flag & kDecFlag) ||
      ((flag & (kOctFlag | kHexFlag)) == (kOctFlag | kHexFlag))) {
    str = value.ToString();
  } else if (flag & kHexFlag) {
    str = value.ToHexString();
  } else if (flag & kOctFlag) {
    std::cout << "8" << std::endl;
  } else {
    str = value.ToString();
  }

  for (uint32_t index = 0; str[index] != '\0'; index++) {
    stream << str[index];
  }

  delete[] str;
  return stream;
}

int2025_t from_string(const char* str) { return (int2025_t)(str); }

int2025_t from_int(int64_t value) { return value; }
