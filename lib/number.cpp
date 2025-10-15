#include "number.h"

// --- Constructions

int2025_t::int2025_t(){}

int2025_t::int2025_t(int64_t value){
    if (value < 0){
        setChunk(kSize - 1, 1);
    }
    int64_t copyValue = value;

    for (uint32_t indexByte = 0; indexByte < 8; indexByte++){
        int64_t current = 0;

        for (uint32_t k = 0; k < 8; k++){
            current += (copyValue & 1) << k;
            copyValue >>=1;
        }

        setChunk(indexByte, current);
    }
}

int2025_t::int2025_t(int32_t value){
    if (value < 0){
        setChunk(kSize - 1, 1);
    }
    int32_t copyValue = value;

    for (uint32_t indexByte = 0; indexByte < 4; indexByte++){
        int32_t current = 0;

        for (uint32_t k = 0; k < 8; k++){
            current += (copyValue & 1) << k;
            copyValue >>=1;
        }

        setChunk(indexByte, current);
    }
    
}

int2025_t::int2025_t(const char* str){
    uint32_t sizeStr = 0;
    while (str[sizeStr] != '\0') {
        ++sizeStr;
    }

    for (uint32_t indexByte = 0; indexByte < kSize && indexByte * 8 < sizeStr; indexByte++){
        uint8_t current = 0;

        for (uint32_t indexStr = std::max(0, (int32_t)((int32_t)sizeStr - 8 * (indexByte + 1))); indexStr < (int32_t)sizeStr - 8 * indexByte; indexStr++){
            if (str[indexStr] < '0' || str[indexStr] > '1'){
                exit(-1);
            }

            current<<=1;
            if (str[indexStr] == '1'){
                current += 1;
            }
        }

        setChunk(indexByte, current);
    }
}

int2025_t::int2025_t(const int2025_t& other) {
    *this = other;
}

// --- private Methods

const uint8_t& int2025_t::getChunk(uint32_t index) const {
    return arrBytes[index];
}

void int2025_t::setChunk(uint32_t index, uint8_t value){
    arrBytes[index] = value;
}

void int2025_t::setChunk(uint32_t index, const char* value){
    uint32_t current = 0;
    for (uint32_t index = 0; index < 8; index++){
        current <<= 1;
        if (value[index] == '1'){
            current += 1; 
        }
    }

    setChunk(index, current);
}

template<class T>
int2025_t int2025_t::binaryOperation(const int2025_t &other, T f) const {
    int2025_t result;

    for (uint32_t index = 0; index < kSize; index++){
        result.setChunk(index, f(this->getChunk(index), other.getChunk(index)));
    }

    return result;
}

int2025_t& int2025_t::revSgn() {
    *this = ~(*this);
    *this = (*this) + 1;

    return *this;
}

// --- public Logic Operations

int2025_t int2025_t::operator~() const {
    int2025_t result;

    for (uint32_t index = 0; index < int2025_t::kSize; index++){
        result.setChunk(index, ~this->getChunk(index));
    }

    return result;
}

int2025_t int2025_t::operator|(const int2025_t &other)const {
    return binaryOperation(other, [](uint8_t a, uint8_t b){return a | b;});
}

int2025_t int2025_t::operator&(const int2025_t &other)const {
    return binaryOperation(other, [](uint8_t a, uint8_t b){return a & b;});
}

int2025_t int2025_t::operator^(const int2025_t &other)const {
    return binaryOperation(other, [](uint8_t a, uint8_t b){return a ^ b;});
}

// --- public Arithmetics Operations

int2025_t int2025_t::operator+(const int2025_t &other)const {
    return binaryOperation(other, [](uint8_t a, uint8_t b){return a + b;});
}

int2025_t int2025_t::operator-(const int2025_t &other)const {
    int2025_t result = other;
    result = *this - result.revSgn();
    return result;
}

int2025_t int2025_t::operator*(const int2025_t &other)const {
    int2025_t myCopy = *this;
    int2025_t otherCopy = other;
    uint8_t finalSgn = myCopy.getSgn() ^ otherCopy.getSgn();
    if (myCopy.getSgn()) {
        myCopy = myCopy.revSgn();
    }
    if (otherCopy.getSgn()){
        otherCopy = otherCopy.revSgn();
    }
    int2025_t result = 0;

    while (otherCopy.toInt64() != 0){
        if (otherCopy.getChunk(0) & 1){
            result += myCopy;
        }

        myCopy.selfLeftShift(1);
        otherCopy.selfRightShift(1);
    }

    if (finalSgn){
        result = result.revSgn();
    }
    return result;
}

int2025_t int2025_t::operator/(const int2025_t &other) const {
    if (*this < other){
        return int2025_t(0);
    }
    if (other == 0){
        return nullptr;
    }

    int2025_t myCopy = *this;
    int2025_t otherCopy = other;
    uint8_t finalSgn = myCopy.getSgn() ^ otherCopy.getSgn();
    if (myCopy.getSgn()) {
        myCopy = myCopy.revSgn();
    }
    if (otherCopy.getSgn()){
        otherCopy = otherCopy.revSgn();
    }
    int2025_t result = 0;

    while (*this >= other){
        
    }

    if (finalSgn){
        result.revSgn();
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
  *this = *this + other;

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
    for (uint32_t index = 0; index + 1 < other.kSize; index++){
        if (getChunk(index) != other.getChunk(index)){
            return false;
        }
    }

    uint8_t byteA = getChunk(kSize - 1);
    uint8_t byteB = other.getChunk(kSize - 1);

    if (byteA & 1 != byteB & 1){
        return false;
    }
    else if (byteA & 2 != byteB & 2){
        return false;
    }

    return true;
}

bool int2025_t::operator!=(const int2025_t& other) const {
     return !(*this == other);
}

bool int2025_t::operator<(const int2025_t& other) const {
     if (getSgn() != other.getSgn()){
        return getSgn() == 1;
     }

     if (getSgn() == 1){    
        for (uint32_t index = 0; index + 1 < other.kSize; index++){
            if (getChunk(index) > other.getChunk(index)){
                return true;
            }
        }

        uint8_t byteA = getChunk(kSize - 1);
        uint8_t byteB = other.getChunk(kSize - 1);

        if (byteA & 1 > byteB & 1){
            return true;
        }

        return false;
     }
     else{
        for (uint32_t index = 0; index + 1 < other.kSize; index++){
        if (getChunk(index) < other.getChunk(index)){
            return true;
        }
    }

        uint8_t byteA = getChunk(kSize - 1);
        uint8_t byteB = other.getChunk(kSize - 1);

        if (byteA & 1 < byteB & 1){
        return true;
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

int2025_t& int2025_t::operator=(const int2025_t &other){
    for (uint32_t index = 0; index < kSize; index++){
        setChunk(index, other.getChunk(index));
    }

    return *this;
}

int2025_t& int2025_t::operator=(const int64_t &value){
    return *this = int2025_t(value);
}

int2025_t& int2025_t::operator=(const char *str){
    return *this = int2025_t(str);
}

// --- public Methods

char* int2025_t::toBinString() const{
    char *result = new char[2025]{0};
    char revResult[2025];

    for (uint32_t indexByte = 0; indexByte < kSize; indexByte++){
        uint8_t byte = getChunk(indexByte);
        for (uint32_t indexBit = 0; indexBit < 8 && indexByte * 8 + indexBit < 2025; indexBit++){
            char bit = byte & 1;
            byte >>= 1;

            revResult[indexByte * 8 + indexBit] = '0' + bit;
        }
    }

    for (uint32_t index = 0; index < 2025; index++){
        result[index] = revResult[2024 - index];
    }

    return result;
}

char* int2025_t::toString() const {
    struct BigInt{
      char buff[1000]{48};
      uint32_t end = 1;  

      BigInt(){
        for (uint32_t index = 0; index < 1000; index++){
            buff[index] = '0';
        }
      }

      BigInt& operator += (BigInt other){
        bool next = 0;
        for (uint32_t index = 0; index < other.end || next; index++){
            next = 0;
            buff[index] += other.buff[index] - '0';

            if (buff[index] > '9'){
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

    char *bits = this->toBinString();
    uint8_t sgn = 0;

    if (bits[0] == '1'){
        sgn = 1;

        for (uint32_t index = 0; index < kSize; index++){
            bits[index] = '0' + ('1' - bits[index]);
        }

        bits[2024] += 1;
        for (uint32_t index = 2024; index >= 0 && bits[index] > '1'; index--){
            if (bits[index] > '1'){
                bits[index] -= 2;
                if (index > 0){
                    bits[index - 1] += 1;
                }
            }
        }
    }

    for (int32_t index = 2024; index >= 0; index--){
        if (bits[index] == '1'){
            result += p;
        }

        p += p;
    }
    
    char* charResult;
    if (!sgn){
        charResult = new char[result.end + 1];
        for (int32_t indexResult = result.end - 1, indexCharResult = 0; indexResult >= 0; indexResult--, indexCharResult++){
            charResult[indexCharResult] << result.buff[indexResult];
        }
        charResult[result.end] = '\0';
    }
    else{
        charResult = new char[result.end + 2];
        charResult[0] = '-';
        for (int32_t indexResult = result.end - 1, indexCharResult = 1; indexResult >= 0; indexResult--, indexCharResult++){
            charResult[indexCharResult] << result.buff[indexResult];
        }
        charResult[result.end + 1] = '\0';
    }

    delete[] bits;
    return charResult;
}

int64_t int2025_t::toInt64() const{
    //Задать вопрос про реализацию через указатели
    int64_t result = 0;
    for (uint32_t k = 0; k < 8; k++){
        result += (int64_t)getChunk(k) << (8 * k);
    }

    int64_t sgn = getSgn();

    result -= (result & ((int64_t)1 << 63));
    result += sgn << 63;

    return result;
}

uint8_t int2025_t::getSgn() const {
    return getChunk(kSize - 1) & 2;
}

int2025_t int2025_t::rightShift(uint32_t k) const {
     int2025_t result = *this;
     return result.selfRightShift(k);
}

uint8_t int2025_t::getFirstOneBit() const {    
    if (!getSgn()){
        for (uint32_t index = 0; index + 1 < kSize; index++){
            uint8_t byte = getChunk(index);

            for (uint32_t k = 0; k < 8; k++){
                uint8_t bit = (1 << k) & byte;

                if (bit > 0){
                    return index * 8 + k;
                }
            }
        }

        if (getChunk(kSize - 1) & 2) {
            return (kSize - 1) * 8 + 1; 
        }

        return -1;
    }
    else{
        int2025_t copyThis = *this;
        copyThis.revSgn();

        for (uint32_t index = 0; index + 1 < kSize; index++){
            uint8_t byte = copyThis.getChunk(index);

            for (uint32_t k = 0; k < 8; k++){
                uint8_t bit = (1 << k) & byte;

                if (bit == 0){
                    return index * 8 + k;
                }
            }
        }

        if (!(copyThis.getChunk(kSize - 1) & 2)) {
            return (kSize - 1) * 8 + 1; 
        }

        return -1;
    }
}

uint8_t int2025_t::getHightStepTwo() const {
    if (!getSgn()){
        if (getChunk(kSize - 1) & 2) {
            return (kSize - 1) * 8 + 1; 
        }

        for (int32_t index = kSize - 1; index >= 0; index--){
            uint8_t byte = getChunk(index);

            for (int32_t k = 7; k >= 0; k--){
                uint8_t bit = (1 << k) & byte;

                if (bit > 0){
                    return index * 8 + k;
                }
            }
        }

        return -1;
    }
    else{
        if (!(getChunk(kSize - 1) & 2)) {
            return (kSize - 1) * 8 + 1; 
        }

        for (int32_t index = kSize - 1; index >= 0; index--){
            uint8_t byte = getChunk(index);

            for (int32_t k = 7; k >= 0; k--){
                uint8_t bit = (1 << k) & byte;

                if (bit == 0){
                    return index * 8 + k;
                }
            }
        }

        return -1;
    }
}

int2025_t int2025_t::leftShift(uint32_t k) const {
    int2025_t result = *this;
    return result.selfLeftShift(k);
}

int2025_t& int2025_t::selfRightShift(uint32_t k) {
  uint32_t cntByte = k / 8;

  if (cntByte > 0){
    for (int32_t index = kSize - 1; index >= 0; index--){
        if (index + cntByte < kSize){
            setChunk(index + cntByte, getChunk(index));
        }

        setChunk(index, (uint8_t)0);
    }
  }

  k -= cntByte * 8;

  for (int32_t index = kSize - 1; index >= 0; index--){
    setChunk(index, getChunk(index) >> k);
    
    if (index > 0){
        uint8_t leftBit = getChunk(index - 1);

        uint8_t mask = leftBit << (8 - k);
        setChunk(index, getChunk(index) + mask);
    }
  }

  return *this;
}

int2025_t& int2025_t::selfLeftShift(uint32_t k) {
  uint32_t cntByte = k / 8;

  if (cntByte > 0){
    for (int32_t index = 0; index < kSize; index++){
        if (index - cntByte >= 0){
            setChunk(index - cntByte, getChunk(index));
        }

        setChunk(index, (uint8_t)0);
    }
  }

  k -= cntByte * 8;

  for (int32_t index = 0; index < kSize; index++){
    setChunk(index, getChunk(index) << k);
    
    if (index > 0){
        uint8_t rightBit = getChunk(index + 1);

        uint8_t mask = rightBit >> (8 - k);
        setChunk(index, getChunk(index) + mask);
    }
  }

  return *this;
}

// --- External Operations

std::ostream& operator<<(std::ostream& stream, const int2025_t& value){
    char* result = value.toString();

    uint32_t index = 0;
    while (result[index] != '\0'){
        stream << result;
    }
    
    return stream;
}

