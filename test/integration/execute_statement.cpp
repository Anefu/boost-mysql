//
// Copyright (c) 2019-2020 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "integration_test_common.hpp"
#include <forward_list>

using namespace boost::mysql::test;
using boost::mysql::value;
using boost::mysql::error_code;
using boost::mysql::error_info;
using boost::mysql::errc;
using boost::mysql::prepared_statement;

namespace
{

template <typename Stream>
struct ExecuteStatementTest : public NetworkTest<Stream>
{
    network_functions<Stream>* net {this->GetParam().net};

    ExecuteStatementTest()
    {
        this->connect(this->GetParam().ssl);
    }

    prepared_statement<Stream> do_prepare(boost::string_view stmt)
    {
        auto res = this->GetParam().net->prepare_statement(this->conn, stmt);
        res.validate_no_error();
        return std::move(res.value);
    }

    // Iterator version
    void Iterator_OkNoParams()
    {
        std::forward_list<value> params;
        auto stmt = do_prepare("SELECT * FROM empty_table");
        auto result = net->execute_statement(stmt, params.begin(), params.end()); // execute
        result.validate_no_error();
        EXPECT_TRUE(result.value.valid());
    }

    void Iterator_OkWithParams()
    {
        std::forward_list<value> params { value("item"), value(42) };
        auto stmt = do_prepare("SELECT * FROM empty_table WHERE id IN (?, ?)");
        auto result = net->execute_statement(stmt, params.begin(), params.end());
        result.validate_no_error();
        EXPECT_TRUE(result.value.valid());
    }

    void Iterator_MismatchedNumParams()
    {
        std::forward_list<value> params { value("item") };
        auto stmt = do_prepare("SELECT * FROM empty_table WHERE id IN (?, ?)");
        auto result = net->execute_statement(stmt, params.begin(), params.end());
        result.validate_error(errc::wrong_num_params, {"param", "2", "1", "statement", "execute"});
        EXPECT_FALSE(result.value.valid());
    }

    void Iterator_ServerError()
    {
        this->start_transaction();
        std::forward_list<value> params { value("f0"), value("bad_date") };
        auto stmt = do_prepare("INSERT INTO inserts_table (field_varchar, field_date) VALUES (?, ?)");
        auto result = net->execute_statement(stmt, params.begin(), params.end());
        result.validate_error(errc::truncated_wrong_value, {"field_date", "bad_date", "incorrect date value"});
        EXPECT_FALSE(result.value.valid());
    }

    // Container version
    void Container_OkNoParams()
    {
        auto stmt = do_prepare("SELECT * FROM empty_table");
        auto result = net->execute_statement(stmt, std::vector<value>()); // execute
        result.validate_no_error();
        EXPECT_TRUE(result.value.valid());
    }

    void Container_OkWithParams()
    {
        std::vector<value> params { value("item"), value(42) };
        auto stmt = do_prepare("SELECT * FROM empty_table WHERE id IN (?, ?)");
        auto result = net->execute_statement(stmt, params);
        result.validate_no_error();
        EXPECT_TRUE(result.value.valid());
    }

    void Container_MismatchedNumParams()
    {
        std::vector<value> params { value("item") };
        auto stmt = do_prepare("SELECT * FROM empty_table WHERE id IN (?, ?)");
        auto result = net->execute_statement(stmt, params);
        result.validate_error(errc::wrong_num_params, {"param", "2", "1", "statement", "execute"});
        EXPECT_FALSE(result.value.valid());
    }

    void Container_ServerError()
    {
        this->start_transaction();
        auto stmt = do_prepare("INSERT INTO inserts_table (field_varchar, field_date) VALUES (?, ?)");
        auto result = net->execute_statement(stmt, makevalues("f0", "bad_date"));
        result.validate_error(errc::truncated_wrong_value, {"field_date", "bad_date", "incorrect date value"});
        EXPECT_FALSE(result.value.valid());
    }
};

BOOST_MYSQL_NETWORK_TEST_SUITE(ExecuteStatementTest)

BOOST_MYSQL_NETWORK_TEST(ExecuteStatementTest, Iterator_OkNoParams)
BOOST_MYSQL_NETWORK_TEST(ExecuteStatementTest, Iterator_OkWithParams)
BOOST_MYSQL_NETWORK_TEST(ExecuteStatementTest, Iterator_MismatchedNumParams)
BOOST_MYSQL_NETWORK_TEST(ExecuteStatementTest, Iterator_ServerError)
BOOST_MYSQL_NETWORK_TEST(ExecuteStatementTest, Container_OkNoParams)
BOOST_MYSQL_NETWORK_TEST(ExecuteStatementTest, Container_OkWithParams)
BOOST_MYSQL_NETWORK_TEST(ExecuteStatementTest, Container_MismatchedNumParams)
BOOST_MYSQL_NETWORK_TEST(ExecuteStatementTest, Container_ServerError)



// Other containers
struct ExecuteStatementOtherContainersTest : IntegTest<boost::asio::ip::tcp::socket>
{
    ExecuteStatementOtherContainersTest()
    {
        connect(boost::mysql::ssl_mode::disable);
    }
};

TEST_F(ExecuteStatementOtherContainersTest, NoParams_CanUseNoStatementParamsVariable)
{
    auto stmt = conn.prepare_statement("SELECT * FROM empty_table");
    auto result = stmt.execute(boost::mysql::no_statement_params);
    EXPECT_TRUE(result.valid());
}

TEST_F(ExecuteStatementOtherContainersTest, CArray)
{
    value arr [] = { value("hola"), value(10) };
    auto stmt = conn.prepare_statement("SELECT * FROM empty_table WHERE id IN (?, ?)");
    auto result = stmt.execute(arr);
    EXPECT_TRUE(result.valid());
}

}
