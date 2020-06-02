//
// Copyright (c) 2019-2020 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_EXECUTE_STATEMENT_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_EXECUTE_STATEMENT_HPP

#include "boost/mysql/detail/network_algorithms/common.hpp"
#include "boost/mysql/detail/network_algorithms/execute_generic.hpp"
#include "boost/mysql/resultset.hpp"
#include "boost/mysql/value.hpp"

namespace boost {
namespace mysql {
namespace detail {

template <typename StreamType, typename ForwardIterator>
void execute_statement(
    channel<StreamType>& channel,
    std::uint32_t statement_id,
    ForwardIterator params_begin,
    ForwardIterator params_end,
    resultset<StreamType>& output,
    error_code& err,
    error_info& info
);

template <typename StreamType, typename ForwardIterator, typename CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, execute_generic_signature<StreamType>)
async_execute_statement(
    channel<StreamType>& chan,
    std::uint32_t statement_id,
    ForwardIterator params_begin,
    ForwardIterator params_end,
    CompletionToken&& token,
    error_info* info
);

} // detail
} // mysql
} // boost

#include "boost/mysql/detail/network_algorithms/impl/execute_statement.hpp"

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_EXECUTE_STATEMENT_HPP_ */
