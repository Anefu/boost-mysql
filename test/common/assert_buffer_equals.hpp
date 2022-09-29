//
// Copyright (c) 2019-2022 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_COMMON_ASSERT_BUFFER_EQUALS_HPP
#define BOOST_MYSQL_TEST_COMMON_ASSERT_BUFFER_EQUALS_HPP

#include <boost/asio/buffer.hpp>
#include <boost/test/unit_test.hpp>
#include <cstring>

namespace boost {
namespace mysql {
namespace test {

struct buffer_printer
{
    boost::asio::const_buffer buff;
};

inline std::ostream& operator<<(std::ostream& os, buffer_printer buff)
{
    os << "{ ";
    for (std::size_t i = 0; i < buff.buff.size(); ++i)
    {
        os << static_cast<int>(static_cast<const std::uint8_t*>(buff.buff.data())[i]) << ", ";
    }
    return os << "}";
}

} // test
} // mysql
} // boost

#define BOOST_MYSQL_ASSERT_BUFFER_EQUALS(b1, b2) \
    BOOST_TEST( \
        ::std::memcmp(b1.data(), b2.data(), b1.size()) == 0, \
        #b1 " != " #b2 ": " << ::boost::mysql::test::buffer_printer{b1} << " != " << ::boost::mysql::test::buffer_printer{b2})

#endif