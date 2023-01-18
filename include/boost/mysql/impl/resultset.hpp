//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_IMPL_RESULTSET_HPP
#define BOOST_MYSQL_IMPL_RESULTSET_HPP

#include <boost/mysql/detail/auxiliar/access_fwd.hpp>
#pragma once

#include <boost/mysql/resultset.hpp>

struct boost::mysql::detail::resultset_access
{
    static execution_state& get_state(resultset& result) noexcept { return result.st_; }
    static rows& get_rows(resultset& result) noexcept { return result.rows_; }
};

#endif
