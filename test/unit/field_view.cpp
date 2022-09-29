//
// Copyright (c) 2019-2022 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/mysql/field_view.hpp>
#include <boost/mysql/field.hpp>
#include <boost/test/tools/context.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <vector>
#include "test_common.hpp"

BOOST_TEST_DONT_PRINT_LOG_VALUE(boost::mysql::date)
BOOST_TEST_DONT_PRINT_LOG_VALUE(boost::mysql::datetime)
BOOST_TEST_DONT_PRINT_LOG_VALUE(boost::mysql::time)

using namespace boost::mysql::test;
using boost::mysql::field_view;
using boost::mysql::field;
using boost::mysql::field_kind;

namespace
{

BOOST_AUTO_TEST_SUITE(test_field_view)

BOOST_AUTO_TEST_SUITE(constructors)

BOOST_AUTO_TEST_CASE(default_constructor)
{
    field_view v;
    BOOST_TEST(v.is_null());
}

BOOST_AUTO_TEST_CASE(copy)
{
    field_view v (32);
    field_view v2 (v);
    BOOST_TEST(v2.as_int64() == 32);
}

BOOST_AUTO_TEST_CASE(move)
{
    field_view v (field_view(32));
    BOOST_TEST(v.as_int64() == 32);
}

BOOST_AUTO_TEST_CASE(from_nullptr)
{
    field_view v(nullptr);
    BOOST_TEST(v.is_null());
}

BOOST_AUTO_TEST_CASE(from_u8)
{
    field_view v(std::uint8_t(0xfe));
    BOOST_TEST(v.as_uint64() == 0xfe);
}

BOOST_AUTO_TEST_CASE(from_u16)
{
    field_view v(std::uint16_t(0xfefe));
    BOOST_TEST(v.as_uint64() == 0xfefe);
}

BOOST_AUTO_TEST_CASE(from_u32)
{
    field_view v(std::uint32_t(0xfefefefe));
    BOOST_TEST(v.as_uint64() == 0xfefefefe);
}

BOOST_AUTO_TEST_CASE(from_u64)
{
    field_view v(std::uint64_t(0xfefefefefefefefe));
    BOOST_TEST(v.as_uint64() == 0xfefefefefefefefe);
}

BOOST_AUTO_TEST_CASE(from_s8)
{
    field_view v(std::int8_t(-1));
    BOOST_TEST(v.as_int64() == -1);
}

BOOST_AUTO_TEST_CASE(from_s16)
{
    field_view v(std::int16_t(-1));
    BOOST_TEST(v.as_int64() == -1);
}

BOOST_AUTO_TEST_CASE(from_s32)
{
    field_view v(std::int32_t(-1));
    BOOST_TEST(v.as_int64() == -1);
}

BOOST_AUTO_TEST_CASE(from_s64)
{
    field_view v(std::int64_t(-1));
    BOOST_TEST(v.as_int64() == -1);
}

BOOST_AUTO_TEST_CASE(from_char_array)
{
    field_view v("test");
    BOOST_TEST(v.as_string() == "test");
}

BOOST_AUTO_TEST_CASE(from_c_str)
{
    const char* str = "test";
    field_view v(str);
    BOOST_TEST(v.as_string() == "test");
}

BOOST_AUTO_TEST_CASE(from_string_view)
{
    boost::string_view sv ("test123", 4);
    field_view v(sv);
    BOOST_TEST(v.as_string() == "test");
}

BOOST_AUTO_TEST_CASE(from_float)
{
    field_view v(4.2f);
    BOOST_TEST(v.as_float() == 4.2f);
}

BOOST_AUTO_TEST_CASE(from_double)
{
    field_view v(4.2);
    BOOST_TEST(v.as_double() == 4.2);
}

BOOST_AUTO_TEST_CASE(from_date)
{
    auto d = makedate(2022, 4, 1);
    field_view v (d);
    BOOST_TEST(v.as_date() == d);
}

BOOST_AUTO_TEST_CASE(from_datetime)
{
    auto d = makedt(2022, 4, 1, 21);
    field_view v (d);
    BOOST_TEST(v.as_datetime() == d);
}

BOOST_AUTO_TEST_CASE(from_time)
{
    auto t = maket(20, 10, 1);
    field_view v (t);
    BOOST_TEST(v.as_time() == t);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(accesors)

// Owning fields, to create references to them in the table below
field f_null;
field f_int64 (50);
field f_uint64 (50u);
field f_string ("long_test_string");
field f_float (4.2f);
field f_double (5.0);
field f_date (makedate(2020, 1, 1));
field f_datetime (makedt(2019, 1, 1));
field f_time (maket(9, 1, 0));

struct
{
    const char* name;
    field_view field;
    field_kind expected_kind;
    bool is_null, is_int64, is_uint64, is_string, is_float, is_double, is_date, is_datetime, is_time;
} test_cases [] = {
    // name           field                             kind                  null,  i64    u64    str    float  double date   dt    time 
    { "null",         field_view(),                     field_kind::null,     true,  false, false, false, false, false, false, false, false },
    { "int64",        field_view(42),                   field_kind::int64,    false, true,  false, false, false, false, false, false, false },
    { "uint64",       field_view(42u),                  field_kind::uint64,   false, false, true,  false, false, false, false, false, false },
    { "string",       field_view("test"),               field_kind::string,   false, false, false, true,  false, false, false, false, false },
    { "float",        field_view(4.2f),                 field_kind::float_,   false, false, false, false, true,  false, false, false, false },
    { "double",       field_view(4.2),                  field_kind::double_,  false, false, false, false, false, true,  false, false, false },
    { "date",         field_view(makedate(2020, 1, 1)), field_kind::date,     false, false, false, false, false, false, true,  false, false },
    { "datetime",     field_view(makedt(2020, 1, 1)),   field_kind::datetime, false, false, false, false, false, false, false, true,  false },
    { "time",         field_view(maket(20, 1, 1)),      field_kind::time,     false, false, false, false, false, false, false, false, true },
    { "ref_null",     field_view(f_null),               field_kind::null,     true,  false, false, false, false, false, false, false, false },
    { "ref_int64",    field_view(f_int64),              field_kind::int64,    false, true,  false, false, false, false, false, false, false },
    { "ref_uint64",   field_view(f_uint64),             field_kind::uint64,   false, false, true,  false, false, false, false, false, false },
    { "ref_string",   field_view(f_string),             field_kind::string,   false, false, false, true,  false, false, false, false, false },
    { "ref_float",    field_view(f_float),              field_kind::float_,   false, false, false, false, true,  false, false, false, false },
    { "ref_double",   field_view(f_double),             field_kind::double_,  false, false, false, false, false, true,  false, false, false },
    { "ref_date",     field_view(f_date),               field_kind::date,     false, false, false, false, false, false, true,  false, false },
    { "ref_datetime", field_view(f_datetime),           field_kind::datetime, false, false, false, false, false, false, false, true,  false },
    { "ref_time",     field_view(f_time),               field_kind::time,     false, false, false, false, false, false, false, false, true },
};

BOOST_AUTO_TEST_CASE(kind)
{
    for (const auto& tc : test_cases)
    {
        BOOST_TEST(tc.field.kind() == tc.expected_kind, tc.name);
    }
}

BOOST_AUTO_TEST_CASE(is)
{
    for (const auto& tc : test_cases)
    {
        BOOST_TEST_CONTEXT(tc.name)
        {
            BOOST_TEST(tc.field.is_null() == tc.is_null);
            BOOST_TEST(tc.field.is_int64() == tc.is_int64);
            BOOST_TEST(tc.field.is_uint64() == tc.is_uint64);
            BOOST_TEST(tc.field.is_string() == tc.is_string);
            BOOST_TEST(tc.field.is_float() == tc.is_float);
            BOOST_TEST(tc.field.is_double() == tc.is_double);
            BOOST_TEST(tc.field.is_date() == tc.is_date);
            BOOST_TEST(tc.field.is_datetime() == tc.is_datetime);
            BOOST_TEST(tc.field.is_time() == tc.is_time);
        }
    }
}

BOOST_AUTO_TEST_CASE(as_exceptions)
{
    for (const auto& tc : test_cases)
    {
        BOOST_TEST_CONTEXT(tc.name)
        {
            if (tc.is_int64)
                BOOST_CHECK_NO_THROW(tc.field.as_int64());
            else
                BOOST_CHECK_THROW(tc.field.as_int64(), boost::mysql::bad_field_access);

            if (tc.is_uint64)
                BOOST_CHECK_NO_THROW(tc.field.as_uint64());
            else
                BOOST_CHECK_THROW(tc.field.as_uint64(), boost::mysql::bad_field_access);

            if (tc.is_string)
                BOOST_CHECK_NO_THROW(tc.field.as_string());
            else
                BOOST_CHECK_THROW(tc.field.as_string(), boost::mysql::bad_field_access);

            if (tc.is_float)
                BOOST_CHECK_NO_THROW(tc.field.as_float());
            else
                BOOST_CHECK_THROW(tc.field.as_float(), boost::mysql::bad_field_access);

            if (tc.is_double)
                BOOST_CHECK_NO_THROW(tc.field.as_double());
            else
                BOOST_CHECK_THROW(tc.field.as_double(), boost::mysql::bad_field_access);

            if (tc.is_date)
                BOOST_CHECK_NO_THROW(tc.field.as_date());
            else
                BOOST_CHECK_THROW(tc.field.as_date(), boost::mysql::bad_field_access);

            if (tc.is_datetime)
                BOOST_CHECK_NO_THROW(tc.field.as_datetime());
            else
                BOOST_CHECK_THROW(tc.field.as_datetime(), boost::mysql::bad_field_access);

            if (tc.is_time)
                BOOST_CHECK_NO_THROW(tc.field.as_time());
            else
                BOOST_CHECK_THROW(tc.field.as_time(), boost::mysql::bad_field_access);
        }
    }
}

// Success cases (the type matches the called function)
BOOST_AUTO_TEST_CASE(int64)
{
    field_view f (-1);
    BOOST_TEST(f.as_int64() == -1);
    BOOST_TEST(f.get_int64() == -1);
}

BOOST_AUTO_TEST_CASE(uint64)
{
    field_view f (42u);
    BOOST_TEST(f.as_uint64() == 42u);
    BOOST_TEST(f.get_uint64() == 42u);
}

BOOST_AUTO_TEST_CASE(string)
{
    field_view f ("test");
    BOOST_TEST(f.as_string() == "test");
    BOOST_TEST(f.get_string() == "test");
}

BOOST_AUTO_TEST_CASE(float_)
{
    field_view f (4.2f);
    BOOST_TEST(f.as_float() == 4.2f);
    BOOST_TEST(f.get_float() == 4.2f);
}

BOOST_AUTO_TEST_CASE(double_)
{
    field_view f (4.2);
    BOOST_TEST(f.as_double() == 4.2);
    BOOST_TEST(f.get_double() == 4.2);
}

BOOST_AUTO_TEST_CASE(date)
{
    auto d = makedate(2020, 1, 2);
    field_view f (d);
    BOOST_TEST(f.as_date() == d);
    BOOST_TEST(f.get_date() == d);
}

BOOST_AUTO_TEST_CASE(datetime)
{
    auto dt = makedt(2020, 1, 2);
    field_view f (dt);
    BOOST_TEST(f.as_datetime() == dt);
    BOOST_TEST(f.get_datetime() == dt);
}

BOOST_AUTO_TEST_CASE(time)
{
    auto t = maket(2020, 1, 2);
    field_view f (t);
    BOOST_TEST(f.as_time() == t);
    BOOST_TEST(f.get_time() == t);
}

BOOST_AUTO_TEST_CASE(ref_int64)
{
    field f (-1);
    field_view fv (f);
    BOOST_TEST(fv.as_int64() == -1);
    BOOST_TEST(fv.get_int64() == -1);
}

BOOST_AUTO_TEST_CASE(ref_uint64)
{
    field f (42u);
    field_view fv (f);
    BOOST_TEST(fv.as_uint64() == 42u);
    BOOST_TEST(fv.get_uint64() == 42u);
}

BOOST_AUTO_TEST_CASE(ref_string)
{
    field f ("test");
    field_view fv (f);
    BOOST_TEST(fv.as_string() == "test");
    BOOST_TEST(fv.get_string() == "test");
}

BOOST_AUTO_TEST_CASE(ref_float)
{
    field f (4.2f);
    field_view fv (f);
    BOOST_TEST(fv.as_float() == 4.2f);
    BOOST_TEST(fv.get_float() == 4.2f);
}

BOOST_AUTO_TEST_CASE(ref_double)
{
    field f (4.2);
    field_view fv (f);
    BOOST_TEST(fv.as_double() == 4.2);
    BOOST_TEST(fv.get_double() == 4.2);
}

BOOST_AUTO_TEST_CASE(ref_date)
{
    auto d = makedate(2020, 1, 2);
    field f (d);
    field_view fv (f);
    BOOST_TEST(fv.as_date() == d);
    BOOST_TEST(fv.get_date() == d);
}

BOOST_AUTO_TEST_CASE(ref_datetime)
{
    auto dt = makedt(2020, 1, 2);
    field f (dt);
    field_view fv (f);
    BOOST_TEST(fv.as_datetime() == dt);
    BOOST_TEST(fv.get_datetime() == dt);
}

BOOST_AUTO_TEST_CASE(ref_time)
{
    auto t = maket(2020, 1, 2);
    field f (t);
    field_view fv (t);
    BOOST_TEST(fv.as_time() == t);
    BOOST_TEST(fv.get_time() == t);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_CASE(operator_equals)
{
    struct
    {
        const char* name;
        field_view f1;
        field_view f2;
        bool is_equal;
    } test_cases [] = {
        { "null_null", field_view(), field_view(), true, },
        { "null_int64", field_view(), field_view(-1), false },
        { "null_uint64", field_view(), field_view(42), false },
        { "null_string", field_view(), field_view("<NULL>"), false },
        { "null_float", field_view(), field_view(4.2f), false },
        { "null_double", field_view(), field_view(4.3), false },
        { "null_date", field_view(), field_view(makedate(2020, 1, 2)), false },
        { "null_datetime", field_view(), field_view(makedt(2020, 1, 1)), false },
        { "null_time", field_view(), field_view(maket(23, 1, 1)), false },

        { "int64_int64_same", field_view(42), field_view(42), true },
        { "int64_int64_different", field_view(42), field_view(-1), false },
        { "int64_uint64_same", field_view(42), field_view(42u), true },
        { "int64_uint64_different", field_view(42), field_view(43u), false },
        { "int64_uint64_zero", field_view(0), field_view(0u), true },
        { "int64_uint64_lt0", field_view(-1), field_view(42u), false },
        { "int64_uint64_gtmax", field_view(42), field_view(0xffffffffffffffffu), false },
        { "int64_uint64_lt0gtmax", field_view(-1), field_view(0xffffffffffffffffu), false },
        { "int64_string", field_view(42), field_view("42"), false },
        { "int64_float", field_view(42), field_view(42.0f), false },
        { "int64_double", field_view(42), field_view(42.0), false },
        { "int64_date", field_view(42), field_view(makedate(2020, 1, 1)), false },
        { "int64_datetime", field_view(42), field_view(makedt(2020, 1, 1)), false },
        { "int64_time", field_view(42), field_view(maket(20, 1, 1)), false },

        { "uint64_uint64_same", field_view(0xffffffffffffffffu), field_view(0xffffffffffffffffu), true },
        { "uint64_uint64_different", field_view(42u), field_view(31u), false },
        { "uint64_string", field_view(42u), field_view("42"), false },
        { "uint64_float", field_view(42u), field_view(42.0f), false },
        { "uint64_double", field_view(42u), field_view(42.0), false },
        { "uint64_date", field_view(42u), field_view(makedate(2020, 1, 1)), false },
        { "uint64_datetime", field_view(42u), field_view(makedt(2020, 1, 1)), false },
        { "uint64_time", field_view(42u), field_view(maket(20, 1, 1)), false },

        { "string_string_same", field_view("test"), field_view("test"), true },
        { "string_string_different", field_view("test"), field_view("test2"), false },
        { "string_float", field_view("4.2"), field_view(4.2f), false },
        { "string_double", field_view("4.2"), field_view(4.2), false },
        { "string_date", field_view("2020-01-01"), field_view(makedate(2020, 1, 1)), false },
        { "string_datetime", field_view("test"), field_view(makedt(2020, 1, 1)), false },
        { "string_time", field_view("test"), field_view(maket(8, 1, 1)), false },

        { "float_float_same", field_view(4.2f), field_view(4.2f), true },
        { "float_float_different", field_view(4.2f), field_view(0.0f), false },
        { "float_double", field_view(4.2f), field_view(4.2), false },
        { "float_date", field_view(4.2f), field_view(makedate(2020, 1, 2)), false },
        { "float_datetime", field_view(4.2f), field_view(makedt(2020, 1, 2)), false },
        { "float_time", field_view(4.2f), field_view(maket(20, 1, 2)), false },

        { "double_double_same", field_view(4.2), field_view(4.2), true },
        { "double_double_different", field_view(4.2), field_view(-1.0), false },
        { "double_date", field_view(4.2), field_view(makedate(2020, 1, 1)), false },
        { "double_datetime", field_view(4.2), field_view(makedt(2020, 1, 1)), false },
        { "double_time", field_view(4.2), field_view(maket(9, 1, 1)), false },

        { "date_date_same", field_view(makedate(2020, 1, 1)), field_view(makedate(2020, 1, 1)), true },
        { "date_date_different", field_view(makedate(2020, 1, 1)), field_view(makedate(2019, 1, 1)), false },
        { "date_datetime", field_view(makedate(2020, 1, 1)), field_view(makedt(2020, 1, 1)), false },
        { "date_time", field_view(makedate(2020, 1, 1)), field_view(maket(9, 1, 1)), false },

        { "datetime_datetime_same", field_view(makedt(2020, 1, 1, 10)), field_view(makedt(2020, 1, 1, 10)), true },
        { "datetime_datetime_different", field_view(makedt(2020, 1, 1, 10)), field_view(makedt(2020, 1, 1, 9)), false },
        { "datetime_time", field_view(makedt(2020, 1, 1)), field_view(maket(20, 1, 1)), false },

        { "time_time_same", field_view(maket(20, 1, 1)), field_view(maket(20, 1, 1)), true },
        { "time_time_different", field_view(maket(20, 1, 1)), field_view(maket(20, 1, 1, 10)), false },
    };

    for (const auto& tc : test_cases)
    {
        BOOST_TEST_CONTEXT(tc.name)
        {
            // We compare regular field_view's and field_view's holding
            // pointers to fields, using the same cases to reduce duplication
            field owning_1 (tc.f1);
            field owning_2 (tc.f2);

            field_view f1 = tc.f1;
            field_view f2 = tc.f2;

            field_view fref1 (owning_1);
            field_view fref2 (owning_2);

            if (tc.is_equal)
            {
                BOOST_TEST(f1 == f2);
                BOOST_TEST(f2 == f1);
                BOOST_TEST(fref1 == fref2);
                BOOST_TEST(fref2 == fref1);
                BOOST_TEST(f1 == fref2);
                BOOST_TEST(fref2 == f1);

                BOOST_TEST(!(f1 != f2));
            }
            else
            {
                BOOST_TEST(!(f1 == f2));
                BOOST_TEST(!(f2 == f1));
                BOOST_TEST(!(fref1 == fref2));
                BOOST_TEST(!(fref2 == fref1));
                BOOST_TEST(!(f1 == fref2));
                BOOST_TEST(!(fref2 == f1));

                BOOST_TEST(tc.f1 != tc.f2);
                BOOST_TEST(tc.f2 != tc.f1);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(operator_equals_self_compare)
{
    struct
    {
        const char* name;
        field_view f;
    } test_cases [] = {
        { "null", field_view() },
        { "int64", field_view(40), },
        { "uint64", field_view(42u) },
        { "string", field_view("test") },
        { "float", field_view(4.2f) },
        { "double", field_view(5.0) },
        { "date", field_view(makedate(2020, 1, 1)) },
        { "datetime", field_view(makedt(2020, 1, 1)) },
        { "time", field_view(maket(8, 1, 1)) }
    };

    for (const auto& tc : test_cases)
    {
        // Regular field_view
        BOOST_TEST(tc.f == tc.f);

        // Reference to an owning field
        field owning_field (tc.f);
        field_view fref (owning_field);
        BOOST_TEST(fref == fref);
    }
}


// operator<<
struct stream_sample
{
    std::string name;
    field_view input;
    std::string expected;

    template <class T>
    stream_sample(std::string&& name, const T& input, std::string&& expected) :
        name(std::move(name)),
        input(input),
        expected(std::move(expected))
    {
    }
};

// Helper struct to define stream operations for date, datetime and time
// We will list the possibilities for each component (hours, minutes, days...) and will
// take the Cartessian product of all them
struct component_value
{
    const char* name;
    int v;
    const char* repr;
};

void add_date_samples(std::vector<stream_sample>& output)
{
    constexpr component_value year_values [] = {
        { "min", 0, "0000" },
        { "onedig",  1, "0001" },
        { "twodig", 98, "0098" },
        { "threedig", 789, "0789" },
        { "regular", 1999, "1999" },
        { "max", 9999, "9999" }
    };

    constexpr component_value month_values [] = {
        { "min", 1, "01" },
        { "max", 12, "12" }
    };

    constexpr component_value day_values [] = {
        { "min", 1, "01" },
        { "max", 31, "31" }
    };

    for (const auto& year : year_values)
    {
        for (const auto& month : month_values)
        {
            for (const auto& day : day_values)
            {
                std::string name = stringize(
                    "date_year", year.name,
                    "_month", month.name,
                    "_day", day.name
                );
                std::string str_val = stringize(year.repr, '-', month.repr, '-', day.repr);
                field_view val (makedate(year.v, static_cast<unsigned>(month.v), static_cast<unsigned>(day.v)));
                output.emplace_back(std::move(name), val, std::move(str_val));
            }
        }
    }
}

void add_datetime_samples(std::vector<stream_sample>& output)
{
    constexpr component_value year_values [] = {
        { "min", 0, "0000" },
        { "onedig",  1, "0001" },
        { "twodig", 98, "0098" },
        { "threedig", 789, "0789" },
        { "regular", 1999, "1999" },
        { "max", 9999, "9999" }
    };

    constexpr component_value month_values [] = {
        { "min", 1, "01" },
        { "max", 12, "12" }
    };

    constexpr component_value day_values [] = {
        { "min", 1, "01" },
        { "max", 31, "31" }
    };

    constexpr component_value hours_values [] = {
        { "zero", 0, "00" },
        { "onedigit", 5, "05" },
        { "max", 23, "23" }
    };

    constexpr component_value mins_secs_values [] = {
        { "zero", 0, "00" },
        { "onedigit", 5, "05" },
        { "twodigits", 59, "59" }
    };

    constexpr component_value micros_values [] = {
        { "zero", 0, "000000" },
        { "onedigit", 5, "000005" },
        { "twodigits", 50, "000050" },
        { "max", 999999, "999999" },
    };

    for (const auto& year : year_values)
    {
        for (const auto& month : month_values)
        {
            for (const auto& day : day_values)
            {
                for (const auto& hours: hours_values)
                {
                    for (const auto& mins: mins_secs_values)
                    {
                        for (const auto& secs: mins_secs_values)
                        {
                            for (const auto& micros: micros_values)
                            {
                                std::string name = stringize(
                                    "datetime_year", year.name,
                                    "_month", month.name,
                                    "_day", day.name,
                                    "_h", hours.name,
                                    "_m", mins.name,
                                    "_s", secs.name,
                                    "_u", micros.name
                                );
                                std::string str_val = stringize(
                                    year.repr, '-',
                                    month.repr, '-',
                                    day.repr, ' ',
                                    hours.repr, ':',
                                    mins.repr, ':',
                                    secs.repr, '.',
                                    micros.repr
                                );
                                auto val = makedt(
                                    year.v,
                                    static_cast<unsigned>(month.v),
                                    static_cast<unsigned>(day.v),
                                    hours.v,
                                    mins.v,
                                    secs.v,
                                    micros.v
                                );
                                output.emplace_back(std::move(name), field_view(val), std::move(str_val));
                            }
                        }
                    }
                }
            }
        }
    }
}

void add_time_samples(std::vector<stream_sample>& output)
{
    constexpr component_value sign_values [] = {
        { "positive", 1, "" },
        { "negative", -1, "-" }
    };

    constexpr component_value hours_values [] = {
        { "zero", 0, "00" },
        { "onedigit", 5, "05" },
        { "twodigits", 23, "23" },
        { "max", 838, "838" }
    };

    constexpr component_value mins_secs_values [] = {
        { "zero", 0, "00" },
        { "onedigit", 5, "05" },
        { "twodigits", 59, "59" }
    };

    constexpr component_value micros_values [] = {
        { "zero", 0, "000000" },
        { "onedigit", 5, "000005" },
        { "twodigits", 50, "000050" },
        { "max", 999999, "999999" },
    };

    for (const auto& sign: sign_values)
    {
        for (const auto& hours: hours_values)
        {
            for (const auto& mins: mins_secs_values)
            {
                for (const auto& secs: mins_secs_values)
                {
                    for (const auto& micros: micros_values)
                    {
                        std::string name = stringize(
                            "time_", sign.name,
                            "_h", hours.name,
                            "_m", mins.name,
                            "_s", secs.name,
                            "_u", micros.name
                        );
                        std::string str_val = stringize(
                            sign.repr,
                            hours.repr, ':',
                            mins.repr, ':',
                            secs.repr, '.',
                            micros.repr
                        );
                        auto val = sign.v * maket(hours.v,
                                mins.v, secs.v, micros.v);
                        if (sign.v == -1 && val == maket(0, 0, 0))
                            continue; // This case makes no sense, as negative zero is represented as zero
                        output.emplace_back(std::move(name), field_view(val), std::move(str_val));
                    }
                }
            }
        }
    }
}

// Samples made out of owning fields
void add_ref_samples(std::vector<stream_sample>& output)
{
    struct owning_fields_t
    {
        field f_null {};
        field f_int64 {-1};
        field f_uint64 {50u};
        field f_string {"long_test_string"};
        field f_float {4.2f};
        field f_double {5.1};
        field f_date {makedate(2020, 1, 1)};
        field f_datetime {makedt(2019, 1, 1, 21, 19, 1, 9)};
        field f_time {maket(9, 1, 0, 210)};
    };
    static owning_fields_t owning_fields;

    output.emplace_back("ref_null", owning_fields.f_null, "<NULL>");
    output.emplace_back("ref_int64", owning_fields.f_int64, "-1");
    output.emplace_back("ref_uint64", owning_fields.f_uint64, "50");
    output.emplace_back("ref_string", owning_fields.f_string, "long_test_string");
    output.emplace_back("ref_float", owning_fields.f_float, "4.2");
    output.emplace_back("ref_double", owning_fields.f_double, "5.1");
    output.emplace_back("ref_date", owning_fields.f_date, "2020-01-01");
    output.emplace_back("ref_datetime", owning_fields.f_datetime, "2019-01-01 21:19:01.000009");
    output.emplace_back("ref_time", owning_fields.f_time, "09:01:00.000210");
}

std::vector<stream_sample> make_stream_samples()
{
    std::vector<stream_sample> res {
        { "null", nullptr, "<NULL>" },
        { "i64_positive", std::int64_t(42), "42" },
        { "i64_negative", std::int64_t(-90), "-90" },
        { "i64_zero", std::int64_t(0), "0" },
        { "u64_positive", std::uint64_t(42), "42" },
        { "u64_zero", std::uint64_t(0), "0" },
        { "string_view", "a_string", "a_string" },
        { "float", 2.43f, "2.43" },
        { "double", 8.12, "8.12" },
    };
    add_date_samples(res);
    add_datetime_samples(res);
    add_time_samples(res);
    add_ref_samples(res);
    return res;
}

BOOST_AUTO_TEST_CASE(operator_stream)
{
    auto test_cases = make_stream_samples();
    for (const auto& tc : test_cases)
    {
        BOOST_TEST_CONTEXT(tc.name)
        {
            std::ostringstream ss;
            ss << tc.input;
            BOOST_TEST(ss.str() == tc.expected);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

}