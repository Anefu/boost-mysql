//
// Copyright (c) 2019-2022 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUXILIAR_FIELD_TYPE_TRAITS_HPP
#define BOOST_MYSQL_DETAIL_AUXILIAR_FIELD_TYPE_TRAITS_HPP

#include <boost/mysql/detail/auxiliar/void_t.hpp>
#include <boost/mysql/field_view.hpp>

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace boost {
namespace mysql {
namespace detail {

template <typename T, typename = void>
struct is_field_view_forward_iterator : std::false_type
{
};

// clang-format off
template <typename T>
struct is_field_view_forward_iterator<T, void_t<
    typename std::enable_if<
        std::is_convertible<
            typename std::iterator_traits<T>::reference,
            ::boost::mysql::field_view
        >::value
    >::type,
    typename std::enable_if<
        std::is_base_of<
            std::forward_iterator_tag, 
            typename std::iterator_traits<T>::iterator_category
        >::value
    >::type
>> : std::true_type { };
// clang-format on

template <typename T, typename = void>
struct is_field_view_collection : std::false_type
{
};

// clang-format off
template <typename T>
struct is_field_view_collection<T, void_t<
    typename std::enable_if<
        is_field_view_forward_iterator<decltype(std::begin(std::declval<const T&>()))>::value
    >::type,
    typename std::enable_if<
        is_field_view_forward_iterator<decltype(std::end(std::declval<const T&>()))>::value
    >::type
>> : std::true_type {};
// clang-format on

// Helpers
template <typename T>
using enable_if_field_view_forward_iterator =
    typename std::enable_if<is_field_view_forward_iterator<T>::value>::type;

template <typename T>
using enable_if_field_view_collection =
    typename std::enable_if<is_field_view_collection<T>::value>::type;

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#endif