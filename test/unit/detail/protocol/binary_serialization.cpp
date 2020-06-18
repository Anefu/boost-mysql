//
// Copyright (c) 2019-2020 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>
#include "test_common.hpp"
#include "serialization_test_common.hpp"
#include "boost/mysql/detail/protocol/binary_serialization.hpp"

using namespace boost::mysql::detail;
using namespace boost::mysql::test;
using namespace testing;
using boost::mysql::value;
using boost::mysql::error_code;
using boost::mysql::errc;

namespace
{

struct serialize_binary_value_testcase : public named
{
    value from;
    bytestring buffer;

    template <typename T>
    serialize_binary_value_testcase(
        std::string&& name,
        T&& from,
        bytestring&& buffer
    ) :
        named(std::move(name)),
        from(std::forward<T>(from)),
        buffer(std::move(buffer))
    {
    }
};

struct SerializeBinaryValueTest : TestWithParam<serialize_binary_value_testcase>
{
};

TEST_P(SerializeBinaryValueTest, GetBinaryValueSize_Trivial_ReturnsExpectedSize)
{
    serialization_context ctx (capabilities{});
    std::size_t size = get_binary_value_size(ctx, GetParam().from);
    EXPECT_EQ(size, GetParam().buffer.size());
}

TEST_P(SerializeBinaryValueTest, SerializeBinaryValue_Trivial_WritesToBuffer)
{
    do_serialize_test(GetParam().buffer, [](serialization_context& ctx) {
        serialize_binary_value(ctx, GetParam().from);
    });
}

// These just test that we call the right serialization function
// Extensive testing for strings is done in serialization.cpp
INSTANTIATE_TEST_SUITE_P(StringTypes, SerializeBinaryValueTest, Values(
    serialize_binary_value_testcase("regular", "abc", {0x03, 0x61, 0x62, 0x63})
), test_name_generator);

// Same comment applies for ints
INSTANTIATE_TEST_SUITE_P(IntTypes, SerializeBinaryValueTest, Values(
    serialize_binary_value_testcase("uint64", std::uint64_t(0xf8f9fafbfcfdfeff),
            {0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8}),
    serialize_binary_value_testcase("int64",  std::int64_t(-0x0706050403020101),
            {0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8})
), test_name_generator);

INSTANTIATE_TEST_SUITE_P(FLOAT, SerializeBinaryValueTest, Values(
    serialize_binary_value_testcase("fractional_negative", -4.2f, {0x66, 0x66, 0x86, 0xc0}),
    serialize_binary_value_testcase("fractional_positive", 4.2f, {0x66, 0x66, 0x86, 0x40}),
    serialize_binary_value_testcase("positive_exp_positive_fractional", 3.14e20f, {0x01, 0x2d, 0x88, 0x61}),
    serialize_binary_value_testcase("zero", 0.0f, {0x00, 0x00, 0x00, 0x00})
), test_name_generator);

INSTANTIATE_TEST_SUITE_P(DOUBLE, SerializeBinaryValueTest, Values(
    serialize_binary_value_testcase("fractional_negative", -4.2, {0xcd, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x10, 0xc0}),
    serialize_binary_value_testcase("fractional_positive", 4.2, {0xcd, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x10, 0x40}),
    serialize_binary_value_testcase("positive_exp_positive_fractional", 3.14e200, {0xce, 0x46, 0x3c, 0x76, 0x9c, 0x68, 0x90, 0x69}),
    serialize_binary_value_testcase("zero", 0.0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00})
), test_name_generator);

INSTANTIATE_TEST_SUITE_P(DATE, SerializeBinaryValueTest, Values(
    serialize_binary_value_testcase("regular", makedate(2010, 3, 28), {0x04, 0xda, 0x07, 0x03, 0x1c}),
    serialize_binary_value_testcase("min", makedate(1000, 1, 1), {0x04, 0xe8, 0x03, 0x01, 0x01}),
    serialize_binary_value_testcase("max", makedate(9999, 12, 31), {0x04, 0x0f, 0x27, 0x0c, 0x1f})
), test_name_generator);

INSTANTIATE_TEST_SUITE_P(DATETIME, SerializeBinaryValueTest, Values(
    serialize_binary_value_testcase("regular", makedt(2010, 1, 1, 23,  1,  59, 967510),
            {0x0b, 0xda, 0x07, 0x01, 0x01, 0x17, 0x01, 0x3b, 0x56, 0xc3, 0x0e, 0x00})
), test_name_generator);

INSTANTIATE_TEST_SUITE_P(TIME, SerializeBinaryValueTest, Values(
    serialize_binary_value_testcase("positive_u", maket(0, 0, 0, 321000),
        {0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe8, 0xe5, 0x04, 0x00}),
    serialize_binary_value_testcase("positive_hmsu", maket(838, 59, 58, 999000),
        {0x0c, 0x00, 0x22, 0x00, 0x00, 0x00, 0x16, 0x3b, 0x3a, 0x58, 0x3e, 0x0f, 0x00}),
    serialize_binary_value_testcase("negative_u", -maket(0, 0, 0, 321000),
        {0x0c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe8, 0xe5, 0x04, 0x00}),
    serialize_binary_value_testcase("negative_hmsu", -maket(838, 59, 58, 999000),
        {0x0c, 0x01, 0x22, 0x00, 0x00, 0x00, 0x16, 0x3b, 0x3a, 0x58, 0x3e, 0x0f, 0x00})
), test_name_generator);

// NULL is transmitted as the NULL bitmap, so nothing is expected as output
INSTANTIATE_TEST_SUITE_P(Null, SerializeBinaryValueTest, Values(
    serialize_binary_value_testcase("regular", nullptr, {})
), test_name_generator);

} // anon namespace
