//
// Copyright (c) 2019-2022 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/mysql/rows_view.hpp>
#include <boost/mysql/field_view.hpp>
#include <boost/test/unit_test.hpp>
#include <stdexcept>
#include "test_common.hpp"

using boost::mysql::rows_view;
using boost::mysql::make_field_views;
using boost::mysql::test::makerow;

/**
iteration
    empty
    range iteration
    backwards iteration (?)
    prefix increment
    postfix increment
    prefix decrement
    postfix decrement
    += ptrdiff_t
    -= ptrdiff_t
    it + ptrdiff_t
    ptrdiff_t + it
    it - ptrdiff_t
    it2 - it1
    operator[]
    operator>
    operator<
    operator>=
    operator<=
 * 
 */

namespace
{

BOOST_AUTO_TEST_SUITE(test_rows_view)

BOOST_AUTO_TEST_CASE(default_ctor)
{
    rows_view v;
    BOOST_TEST(v.empty());
}


BOOST_AUTO_TEST_SUITE(at)
BOOST_AUTO_TEST_CASE(empty)
{
    rows_view v;
    BOOST_CHECK_THROW(v.at(0), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(one_column_one_row)
{
    auto fields = make_field_views(42u);
    rows_view v (fields.data(), 1, 1);
    BOOST_TEST(v.at(0) == makerow(42u));
    BOOST_CHECK_THROW(v.at(1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(one_column_several_rows)
{
    auto fields = make_field_views(42u, "abc");
    rows_view v (fields.data(), 2, 1);
    BOOST_TEST(v.at(0) == makerow(42u));
    BOOST_TEST(v.at(1) == makerow("abc"));
    BOOST_CHECK_THROW(v.at(2), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(several_columns_one_row)
{
    auto fields = make_field_views(42u, "abc");
    rows_view v (fields.data(), 2, 2);
    BOOST_TEST(v.at(0) == makerow(42u, "abc"));
    BOOST_CHECK_THROW(v.at(1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(several_columns_several_rows)
{
    auto fields = make_field_views(42u, "abc", nullptr, "bcd", 90u, nullptr);
    rows_view v (fields.data(), 6, 2);
    BOOST_TEST(v.at(0) == makerow(42u, "abc"));
    BOOST_TEST(v.at(1) == makerow(nullptr, "bcd"));
    BOOST_TEST(v.at(2) == makerow(90u, nullptr));
    BOOST_CHECK_THROW(v.at(3), std::out_of_range);
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(operator_square_brackets)
BOOST_AUTO_TEST_CASE(one_column_one_row)
{
    auto fields = make_field_views(42u);
    rows_view v (fields.data(), 1, 1);
    BOOST_TEST(v[0] == makerow(42u));
}

BOOST_AUTO_TEST_CASE(one_column_several_rows)
{
    auto fields = make_field_views(42u, "abc");
    rows_view v (fields.data(), 2, 1);
    BOOST_TEST(v[0] == makerow(42u));
    BOOST_TEST(v[1] == makerow("abc"));
}

BOOST_AUTO_TEST_CASE(several_columns_one_row)
{
    auto fields = make_field_views(42u, "abc");
    rows_view v (fields.data(), 2, 2);
    BOOST_TEST(v[0] == makerow(42u, "abc"));
}

BOOST_AUTO_TEST_CASE(several_columns_several_rows)
{
    auto fields = make_field_views(42u, "abc", nullptr, "bcd", 90u, nullptr);
    rows_view v (fields.data(), 6, 2);
    BOOST_TEST(v[0] == makerow(42u, "abc"));
    BOOST_TEST(v[1] == makerow(nullptr, "bcd"));
    BOOST_TEST(v[2] == makerow(90u, nullptr));
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_CASE(front)
{
    auto fields = make_field_views(42u, "abc", nullptr, "bcde");
    rows_view v (fields.data(), 4, 2);
    BOOST_TEST(v.front() == makerow(42u, "abc"));
}

BOOST_AUTO_TEST_CASE(back)
{
    auto fields = make_field_views(42u, "abc", nullptr, "bcde");
    rows_view v (fields.data(), 4, 2);
    BOOST_TEST(v.back() == makerow(nullptr, "bcde"));
}


BOOST_AUTO_TEST_CASE(empty)
{
    BOOST_TEST(rows_view().empty());

    auto fields = make_field_views(42u);
    BOOST_TEST(!rows_view(fields.data(), 1, 1).empty());
}


BOOST_AUTO_TEST_SUITE(size)
BOOST_AUTO_TEST_CASE(empty)
{
    rows_view v;
    BOOST_TEST(v.size() == 0);
}

BOOST_AUTO_TEST_CASE(one_column_one_row)
{
    auto fields = make_field_views(42u);
    rows_view v (fields.data(), 1, 1);
    BOOST_TEST(v.size() == 1);
}

BOOST_AUTO_TEST_CASE(one_column_several_rows)
{
    auto fields = make_field_views(42u, "abc");
    rows_view v (fields.data(), 2, 1);
    BOOST_TEST(v.size() == 2);
}

BOOST_AUTO_TEST_CASE(several_columns_one_row)
{
    auto fields = make_field_views(42u, "abc");
    rows_view v (fields.data(), 2, 2);
    BOOST_TEST(v.size() == 1);
}

BOOST_AUTO_TEST_CASE(several_columns_several_rows)
{
    auto fields = make_field_views(42u, "abc", nullptr, "bcd", 90u, nullptr);
    rows_view v (fields.data(), 6, 2);
    BOOST_TEST(v.size() == 3);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}