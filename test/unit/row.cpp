//
// Copyright (c) 2019-2022 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/mysql/row.hpp>
#include <boost/mysql/row_view.hpp>
#include <boost/mysql/field_view.hpp>
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include "test_common.hpp"

using boost::mysql::row;
using boost::mysql::row_view;
using boost::mysql::field_view;
using boost::mysql::make_field_views;
using boost::mysql::test::makerow;


// at, operator[], size, empty, begin, end: same as row_view
// operator row_view
//    empty
//    non-empty
// operator==, !=: row-rv, rv-row, row-row


BOOST_AUTO_TEST_SUITE(test_row)

BOOST_AUTO_TEST_CASE(default_ctor)
{
    row r;
    BOOST_TEST(r.empty());
}

BOOST_AUTO_TEST_SUITE(ctor_from_row_view)
BOOST_AUTO_TEST_CASE(empty)
{
    row_view v;
    row r (v);
    BOOST_TEST(r.empty());
}

BOOST_AUTO_TEST_CASE(non_strings)
{
    auto fields = make_field_views(42, 5.0f);
    row r (row_view(fields.data(), fields.size()));

    // Fields still valid even when the original source of the view changed
    fields = make_field_views(90, 2.0);
    BOOST_TEST(r.size() == 2);
    BOOST_TEST(r[0] == field_view(42));
    BOOST_TEST(r[1] == field_view(5.0f));
}

BOOST_AUTO_TEST_CASE(strings)
{
    std::string s1 ("test"), s2 ("");
    auto fields = make_field_views(s1, s2, 50);
    row r (row_view(fields.data(), fields.size()));

    // Fields still valid even when the original strings changed
    s1 = "other";
    s2 = "abcdef";
    BOOST_TEST(r.size() == 3);
    BOOST_TEST(r[0] == field_view("test"));
    BOOST_TEST(r[1] == field_view(""));
    BOOST_TEST(r[2] == field_view(50));
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(copy_ctor)
BOOST_AUTO_TEST_CASE(empty)
{
    row r1;
    row r2 (r1);
    r1 = makerow(42, "test"); // r2 should be independent of r1

    BOOST_TEST(r2.empty());
}

BOOST_AUTO_TEST_CASE(non_strings)
{
    row r1 = makerow(42, 5.0f);
    row r2 (r1);
    r1 = makerow(42, "test"); // r2 should be independent of r1

    BOOST_TEST(r2.size() == 2);
    BOOST_TEST(r2[0] == field_view(42));
    BOOST_TEST(r2[1] == field_view(5.0f));
}

BOOST_AUTO_TEST_CASE(strings)
{
    row r1 = makerow("", 42, "test");
    row r2 (r1);
    r1 = makerow("another_string", 4.2f, ""); // r2 should be independent of r1

    BOOST_TEST(r2.size() == 3);
    BOOST_TEST(r2[0] == field_view(""));
    BOOST_TEST(r2[1] == field_view(42));
    BOOST_TEST(r2[2] == field_view("test"));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(move_ctor)
BOOST_AUTO_TEST_CASE(empty)
{
    row r1;
    row r2 (std::move(r1));
    r1 = makerow(42, "test"); // r2 should be independent of r1

    BOOST_TEST(r2.empty());
}

BOOST_AUTO_TEST_CASE(non_strings)
{
    row r1 = makerow(42, 5.0f);
    const field_view* begin_before = r1.begin(); // iterators are not invalidated by move
    row r2 (std::move(r1));
    r1 = makerow(42, "test"); // r2 should be independent of r1

    BOOST_TEST(r2.size() == 2);
    BOOST_TEST(r2[0] == field_view(42));
    BOOST_TEST(r2[1] == field_view(5.0f));
    BOOST_TEST(r2.begin() == begin_before);
}

BOOST_AUTO_TEST_CASE(strings)
{
    row r1 = makerow("", 42, "test");
    const char* str_begin_before = r1[2].as_string().data(); // pointers to strings are not invalidated by move
    row r2 (std::move(r1));
    r1 = makerow("another_string", 4.2f, ""); // r2 should be independent of r1

    BOOST_TEST(r2.size() == 3);
    BOOST_TEST(r2[0] == field_view(""));
    BOOST_TEST(r2[1] == field_view(42));
    BOOST_TEST(r2[2] == field_view("test"));
    BOOST_TEST(r2[2].as_string().data() == str_begin_before);
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(assignment_from_view)
BOOST_AUTO_TEST_CASE(empty)
{
    row r = makerow(42, "abcdef");
    r = row();
    BOOST_TEST(r.empty());
}

BOOST_AUTO_TEST_CASE(non_strings)
{
    row r = makerow(42, "abcdef");
    auto fields = make_field_views(90, nullptr);
    r = row_view(fields.data(), fields.size());
    fields = make_field_views("abc", 42u); // r should be independent of the original fields

    BOOST_TEST(r.size() == 2);
    BOOST_TEST(r[0] == field_view(90));
    BOOST_TEST(r[1] == field_view());
}

BOOST_AUTO_TEST_CASE(strings)
{
    std::string s1 ("a_very_long_string"), s2("");
    row r = makerow(42, "abcdef");
    auto fields = make_field_views(s1, nullptr, s2);
    r = row_view(fields.data(), fields.size());
    fields = make_field_views("abc", 42u, 9); // r should be independent of the original fields
    s1 = "another_string"; // r should be independent of the original strings
    s2 = "yet_another";

    BOOST_TEST(r.size() == 3);
    BOOST_TEST(r[0] == field_view("a_very_long_string"));
    BOOST_TEST(r[1] == field_view());
    BOOST_TEST(r[2] == field_view(""));
}

BOOST_AUTO_TEST_CASE(strings_empty_to)
{
    row r;
    auto fields = make_field_views("abc", nullptr, "bcd");
    r = row_view(fields.data(), fields.size());

    BOOST_TEST(r.size() == 3);
    BOOST_TEST(r[0] == field_view("abc"));
    BOOST_TEST(r[1] == field_view());
    BOOST_TEST(r[2] == field_view("bcd"));
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(copy_assignment)
BOOST_AUTO_TEST_CASE(empty)
{
    row r1 = makerow(42, "abcdef");
    row r2;
    r1 = r2;
    r2 = makerow(90, nullptr); // r1 is independent of r2
    BOOST_TEST(r1.empty());
}

BOOST_AUTO_TEST_CASE(non_strings)
{
    row r1 = makerow(42, "abcdef");
    row r2 = makerow(50.0f, nullptr, 80u);
    r1 = r2;
    r2 = makerow("abc", 80, nullptr); // r1 is independent of r2

    BOOST_TEST(r1.size() == 3);
    BOOST_TEST(r1[0] == field_view(50.0f));
    BOOST_TEST(r1[1] == field_view());
    BOOST_TEST(r1[2] == field_view(80u));
}

BOOST_AUTO_TEST_CASE(strings)
{
    row r1 = makerow(42, "abcdef");
    row r2 = makerow("a_very_long_string", nullptr, "");
    r1 = r2;
    r2 = makerow("another_string", 90, "yet_another"); // r1 is independent of r2

    BOOST_TEST(r1.size() == 3);
    BOOST_TEST(r1[0] == field_view("a_very_long_string"));
    BOOST_TEST(r1[1] == field_view());
    BOOST_TEST(r1[2] == field_view(""));
}

BOOST_AUTO_TEST_CASE(strings_empty_to)
{
    row r1;
    row r2 = makerow("abc", nullptr, "bcd");
    r1 = r2;

    BOOST_TEST(r1.size() == 3);
    BOOST_TEST(r1[0] == field_view("abc"));
    BOOST_TEST(r1[1] == field_view());
    BOOST_TEST(r1[2] == field_view("bcd"));
}

BOOST_AUTO_TEST_CASE(self_assignment_empty)
{
    row r;
    const row& ref = r;
    r = ref;

    BOOST_TEST(r.empty());
}

BOOST_AUTO_TEST_CASE(self_assignment_non_empty)
{
    row r = makerow("abc", 50u, "fgh");
    const row& ref = r;
    r = ref;

    BOOST_TEST(r.size() == 3);
    BOOST_TEST(r[0] == field_view("abc"));
    BOOST_TEST(r[1] == field_view(50u));
    BOOST_TEST(r[2] == field_view("fgh"));
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(move_assignment)
BOOST_AUTO_TEST_CASE(empty)
{
    row r1 = makerow(42, "abcdef");
    row r2;
    r1 = std::move(r2);
    r2 = makerow(90, nullptr); // r1 is independent of r2
    BOOST_TEST(r1.empty());
}

BOOST_AUTO_TEST_CASE(non_strings)
{
    row r1 = makerow(42, "abcdef");
    row r2 = makerow(50.0f, nullptr, 80u);
    r1 = std::move(r2);
    r2 = makerow("abc", 80, nullptr); // r1 is independent of r2

    BOOST_TEST(r1.size() == 3);
    BOOST_TEST(r1[0] == field_view(50.0f));
    BOOST_TEST(r1[1] == field_view());
    BOOST_TEST(r1[2] == field_view(80u));
}

BOOST_AUTO_TEST_CASE(strings)
{
    row r1 = makerow(42, "abcdef");
    row r2 = makerow("a_very_long_string", nullptr, "");
    r1 = std::move(r2);
    r2 = makerow("another_string", 90, "yet_another"); // r1 is independent of r2

    BOOST_TEST(r1.size() == 3);
    BOOST_TEST(r1[0] == field_view("a_very_long_string"));
    BOOST_TEST(r1[1] == field_view());
    BOOST_TEST(r1[2] == field_view(""));
}

BOOST_AUTO_TEST_CASE(strings_empty_to)
{
    row r1;
    row r2 = makerow("abc", nullptr, "bcd");
    r1 = std::move(r2);

    BOOST_TEST(r1.size() == 3);
    BOOST_TEST(r1[0] == field_view("abc"));
    BOOST_TEST(r1[1] == field_view());
    BOOST_TEST(r1[2] == field_view("bcd"));
}

BOOST_AUTO_TEST_CASE(self_assignment_empty)
{
    row r;
    row&& ref = std::move(r);
    r = std::move(ref);

    // r is in a valid but unspecified state; can be assigned to
    r = makerow("abcdef");
    BOOST_TEST(r.size() == 1);
    BOOST_TEST(r[0] == field_view("abcdef"));
}

BOOST_AUTO_TEST_CASE(self_assignment_non_empty)
{
    row r = makerow("abc", 50u, "fgh");
    row&& ref = std::move(r);
    r = std::move(ref); // this should leave r in a valid but unspecified state

    // r is in a valid but unspecified state; can be assigned to
    r = makerow("abcdef");
    BOOST_TEST(r.size() == 1);
    BOOST_TEST(r[0] == field_view("abcdef"));
}

BOOST_AUTO_TEST_SUITE_END()


// // Constructors
// BOOST_AUTO_TEST_SUITE(constructors)

// static std::vector<field_view> string_value_from_buffer(const bytestring& buffer)
// {
//     return make_value_vector(boost::string_view(reinterpret_cast<const char*>(buffer.data()), buffer.size()));
// }


// BOOST_AUTO_TEST_CASE(move_ctor)
// {
//     // Given a row with strings, after a move construction,
//     // the string still points to valid memory
//     bytestring buffer {'a', 'b', 'c', 'd'};
//     auto values = string_value_from_buffer(buffer);
//     row r (std::move(values), std::move(buffer));
//     row r2 (std::move(r));
//     r = row({}, {'e', 'f'});
//     BOOST_TEST(r2.values()[0] == field_view("abcd"));
// }

// BOOST_AUTO_TEST_CASE(move_assignment)
// {
//     // Given a row with strings, after a move assignment,
//     // the string still points to valid memory
//     bytestring buffer {'a', 'b', 'c', 'd'};
//     auto values = string_value_from_buffer(buffer);
//     row r (std::move(values), std::move(buffer));
//     row r2;
//     r2 = std::move(r);
//     r = row({}, {'e', 'f'});
//     BOOST_TEST(r2.values()[0] == field_view("abcd"));
// }

// BOOST_AUTO_TEST_SUITE_END()

// // Clear
// BOOST_AUTO_TEST_SUITE(clear)

// BOOST_AUTO_TEST_CASE(empty_row)
// {
//     row r;
//     r.clear();
//     BOOST_TEST(r.values().empty());
// }

// BOOST_AUTO_TEST_CASE(non_empty_row)
// {
//     row r = makerow("abc");
//     r.clear();
//     BOOST_TEST(r.values().empty());
// }

// BOOST_AUTO_TEST_SUITE_END()


// // Equality operators
// BOOST_AUTO_TEST_SUITE(operators_eq_ne)

// BOOST_AUTO_TEST_CASE(both_empty)
// {
//     BOOST_TEST(row() == row());
//     BOOST_TEST(!(row() != row()));
// }

// BOOST_AUTO_TEST_CASE(one_empty_other_not_empty)
// {
//     row empty_row;
//     row non_empty_row = makerow("a_value");
//     BOOST_TEST(!(empty_row == non_empty_row));
//     BOOST_TEST(empty_row != non_empty_row);
// }

// BOOST_AUTO_TEST_CASE(subset)
// {
//     row lhs = makerow("a_value", 42);
//     row rhs = makerow("a_value");
//     BOOST_TEST(!(lhs == rhs));
//     BOOST_TEST(lhs != rhs);
// }

// BOOST_AUTO_TEST_CASE(same_size_different_values)
// {
//     row lhs = makerow("a_value", 42);
//     row rhs = makerow("another_value", 42);
//     BOOST_TEST(!(lhs == rhs));
//     BOOST_TEST(lhs != rhs);
// }

// BOOST_AUTO_TEST_CASE(same_size_and_values)
// {
//     row lhs = makerow("a_value", 42);
//     row rhs = makerow("a_value", 42);
//     BOOST_TEST(lhs == rhs);
//     BOOST_TEST(!(lhs != rhs));
// }

// BOOST_AUTO_TEST_SUITE_END() // operators_eq_ne

// // Stream operators
// BOOST_AUTO_TEST_SUITE(operator_stream)

// BOOST_AUTO_TEST_CASE(empty_row)
// {
//     BOOST_TEST(stringize(row()) == "{}");
// }

// BOOST_AUTO_TEST_CASE(one_element)
// {
//     BOOST_TEST(stringize(makerow(42)) == "{42}");
// }

// BOOST_AUTO_TEST_CASE(two_elements)
// {
//     auto actual = stringize(makerow("value", nullptr));
//     BOOST_TEST(actual == "{value, <NULL>}");
// }

// BOOST_AUTO_TEST_CASE(three_elements)
// {
//     auto actual = stringize(makerow("value", std::uint32_t(2019), 3.14f));
//     BOOST_TEST(actual == "{value, 2019, 3.14}");
// }

// BOOST_AUTO_TEST_SUITE_END() // operator_stream

BOOST_AUTO_TEST_SUITE_END() // test_row

