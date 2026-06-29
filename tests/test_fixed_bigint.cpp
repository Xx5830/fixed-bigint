#include <fbi/fixed_bigint.hpp>
#include <gtest/gtest.h>

using namespace fbi;

TEST(FixedBigIntTest, DefaultConstructorIsZero) {
    fixed_bigint<128> num;
    EXPECT_EQ(num.ToString(), "0");
}

TEST(FixedBigIntTest, IntConstructor) {
    fixed_bigint<256> a(42);
    fixed_bigint<256> b(-42);

    EXPECT_EQ(a.ToString(), "42");
    EXPECT_EQ(b.ToString(), "-42");
}

TEST(FixedBigIntTest, StringConstructor) {
    fixed_bigint<2025> num("123456789012345678901234567890");
    EXPECT_EQ(num.ToString(), "123456789012345678901234567890");
}

TEST(FixedBigIntTest, Addition) {
    fixed_bigint<128> a(100);
    fixed_bigint<128> b(250);
    auto c = a + b;
    EXPECT_EQ(c.ToString(), "350");
}

TEST(FixedBigIntTest, Subtraction) {
    fixed_bigint<128> a(10);
    fixed_bigint<128> b(25);
    auto c = a - b;
    EXPECT_EQ(c.ToString(), "-15");
}

TEST(FixedBigIntTest, Multiplication) {
    fixed_bigint<256> a("1000000000000000000");
    fixed_bigint<256> b(2);
    auto c = a * b;
    EXPECT_EQ(c.ToString(), "2000000000000000000");
}

TEST(FixedBigIntTest, Shifts) {
    fixed_bigint<128> a(1);
    a = a.LeftShift(5);
    EXPECT_EQ(a.ToString(), "32");

    a = a.RightShift(2);
    EXPECT_EQ(a.ToString(), "8");
}

TEST(FixedBigIntTest, ImplicitConversionHiddenFriends) {
    fixed_bigint<128> a(10);
    // Должно скомпилироваться благодаря Hidden Friends и неявному int->fixed_bigint
    auto b = a + 5;
    auto c = 15 + a;
    EXPECT_EQ(b.ToString(), "15");
    EXPECT_EQ(c.ToString(), "25");
}

TEST(FixedBigIntTest, ConstexprMath) {
    // Проверка, что вычисления возможны на этапе компиляции
    constexpr fixed_bigint<128> a(50);
    constexpr fixed_bigint<128> b(2);
    constexpr fixed_bigint<128> c = a * b;

    // В runtime только преобразуем в строку для проверки
    EXPECT_EQ(c.ToString(), "100");
}