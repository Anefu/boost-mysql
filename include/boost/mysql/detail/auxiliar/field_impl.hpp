//
// Copyright (c) 2019-2022 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUXILIAR_FIELD_IMPL_HPP
#define BOOST_MYSQL_DETAIL_AUXILIAR_FIELD_IMPL_HPP

#include <boost/mysql/field_kind.hpp>
#include <boost/mysql/datetime_types.hpp>
#include <boost/mysql/bad_field_access.hpp>
#include <boost/variant2/variant.hpp>
#include <boost/mp11.hpp>
#include <string>
#include <type_traits>

namespace boost {
namespace mysql {
namespace detail {

// Breaks a circular dependency between field_view and field
struct field_impl
{
    using null_t = boost::variant2::monostate;
    
    using variant_type = boost::variant2::variant<
        null_t,            // Any of the below when the value is NULL
        std::int64_t,      // signed TINYINT, SMALLINT, MEDIUMINT, INT, BIGINT
        std::uint64_t,     // unsigned TINYINT, SMALLINT, MEDIUMINT, INT, BIGINT, YEAR, BIT
        std::string,       // CHAR, VARCHAR, BINARY, VARBINARY, TEXT (all sizes), BLOB (all sizes), ENUM, SET, DECIMAL, GEOMTRY
        float,             // FLOAT
        double,            // DOUBLE
        date,              // DATE
        datetime,          // DATETIME, TIMESTAMP
        time               // TIME
    >;

    variant_type data;

    field_impl() = default;

    template <typename Arg>
    field_impl(Arg&& arg) noexcept(std::is_nothrow_constructible<variant_type, Arg>::value) : data (std::forward<Arg>(arg)) {}

    field_kind kind() const noexcept { return static_cast<field_kind>(data.index()); }

    template <typename T>
    const T& as() const
    {
        const T* res = boost::variant2::get_if<T>(&data);
        if (!res)
            throw bad_field_access();
        return *res;
    }

    template <typename T>
    T& as()
    {
        T* res = boost::variant2::get_if<T>(&data);
        if (!res)
            throw bad_field_access();
        return *res;
    }

    template <typename T>
    const T& get() const noexcept
    {
        constexpr auto I = mp11::mp_find<variant_type, T>::value;
        return boost::variant2::unsafe_get<I>(data);
    }

    template <typename T>
    T& get() noexcept
    {
        constexpr auto I = mp11::mp_find<variant_type, T>::value;
        return boost::variant2::unsafe_get<I>(data);
    }
};

} // detail
} // mysql
} // boost

#endif