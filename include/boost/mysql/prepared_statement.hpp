//
// Copyright (c) 2019-2020 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_PREPARED_STATEMENT_HPP
#define BOOST_MYSQL_PREPARED_STATEMENT_HPP

#include "boost/mysql/resultset.hpp"
#include "boost/mysql/detail/protocol/channel.hpp"
#include "boost/mysql/detail/protocol/prepared_statement_messages.hpp"
#include <optional>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>

/**
 * \defgroup stmt Prepared statements
 * \brief Classes and functions related to prepared statements.
 */

namespace boost {
namespace mysql {

/**
 * \ingroup stmt
 * \brief Convenience constant to use within prepared_statement::execute with statements without parameters.
 */
constexpr std::array<value, 0> no_statement_params {};

/**
 * \ingroup stmt
 * \brief Represents a prepared statement.
 * \details This class is a handle to a server-side prepared statement.
 *
 * The main use of a prepared statement is executing it using prepared_statement::execute.
 * When calling this method, you must pass in **exactly** as many parameters as
 * the statement has. You may pass in the parameters as a collection of
 * boost::mysql::value's or as an iterator range pointing to boost::mysql::value's.
 *
 * Execution of a statement results in a mysql::resultset. The same considerations
 * as with connection::query apply. In particular, **you should read the entire
 * resultset** before calling any function involving communication with the server
 * over this connection.
 *
 * Prepared statements are default constructible. A default-constructed prepared statement
 * is considered invalid (prepared_statement::valid() will return false). Calling
 * any function other than assignment on an invalid statement results in
 * undefined behavior.
 *
 * Prepared statements are managed by the server in a per-connection basis:
 * once created, a prepared statement may be used as long as the parent
 * mysql::connection object (e.g. the connection on which
 * connection::prepare_statement was called) is alive and open. Calling any function on a
 * prepared_statement whose parent connection has been closed or destroyed results
 * in undefined behavior.
 *
 * After the connection with the server is closed, the server deallocates all
 * prepared statements associated with the connection. Prepared statements may
 * also be explicitly deallocated by calling prepared_statement::close.
 */
template <typename Stream>
class prepared_statement
{
    detail::channel<Stream>* channel_ {};
    detail::com_stmt_prepare_ok_packet stmt_msg_;

    template <typename ForwardIterator>
    void check_num_params(ForwardIterator first, ForwardIterator last, error_code& err, error_info& info) const;

    error_info& shared_info() noexcept { assert(channel_); return channel_->shared_info(); }
public:
    /// Default constructor.
    prepared_statement() = default;

    // Private. Do not use.
    prepared_statement(detail::channel<Stream>& chan, const detail::com_stmt_prepare_ok_packet& msg) noexcept:
        channel_(&chan), stmt_msg_(msg) {}

    /// The executor type associated to this object.
    using executor_type = typename Stream::executor_type;

    /// Retrieves the executor associated to this object.
    executor_type get_executor() { assert(channel_);  return channel_->get_executor(); }

    /// Retrieves the stream object associated with the underlying connection.
    Stream& next_layer() noexcept { assert(channel_); return channel_->next_layer(); }

    /// Retrieves the stream object associated with the underlying connection.
    const Stream& next_layer() const noexcept { assert(channel_); return channel_->next_layer(); }

    /// Returns true if the statement is not a default-constructed object.
    bool valid() const noexcept { return channel_ != nullptr; }

    /// Returns a server-side identifier for the statement (unique in a per-connection basis).
    unsigned id() const noexcept { assert(valid()); return stmt_msg_.statement_id.value; }

    /// Returns the number of parameters that should be passed to execute().
    unsigned num_params() const noexcept { assert(valid()); return stmt_msg_.num_params.value; }

    /**
     * \brief Executes a statement (collection, sync with error code version).
     * \details Collection should be a sequence for which std::begin() and
     * std::end() return forward iterators to a valid boost::mysql::value range.
     * Use no_statement_params to execute a statement with no params.
     */
    template <typename Collection>
    resultset<Stream> execute(const Collection& params, error_code& err, error_info& info)
    {
        return execute(std::begin(params), std::end(params), err, info);
    }

    /// Executes a statement (collection, sync with exceptions code version).
    template <typename Collection>
    resultset<Stream> execute(const Collection& params)
    {
        return execute(std::begin(params), std::end(params));
    }

    /**
     * \brief Executes a statement (collection, async without error_info version).
     * \details The handler signature for this operation is
     * `void(boost::mysql::error_code, boost::mysql::resultset<Stream>)`.
     *
     * It is **not** necessary to keep the collection of parameters or the
     * values they may point to alive.
     */
    template <
        typename Collection,
            BOOST_ASIO_COMPLETION_TOKEN_FOR(void(error_code, resultset<Stream>))
            CompletionToken
            BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(executor_type)
    >
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code, resultset<Stream>))
    async_execute(
        const Collection& params,
        CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(executor_type)
    )
    {
        return async_execute(
            std::begin(params),
            std::end(params),
            std::forward<CompletionToken>(token)
        );
    }

    /// Executes a statement (collection, async with error_info version).
    template <
        typename Collection,
            BOOST_ASIO_COMPLETION_TOKEN_FOR(void(error_code, resultset<Stream>))
            CompletionToken
            BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(executor_type)
    >
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code, resultset<Stream>))
    async_execute(
        const Collection& params,
        error_info& output_info,
        CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(executor_type)
    )
    {
        return async_execute(
            std::begin(params),
            std::end(params),
            output_info,
            std::forward<CompletionToken>(token)
        );
    }


    /**
     * \brief Executes a statement (iterator, sync with error code version).
     * \details params_first and params_last should point to a valid range, identifying
     * the parameters to be used. They should be forward iterators, at least. The value_type
     * of these iterators should be convertible to boost::mysql::value.
     */
    template <typename ForwardIterator>
    resultset<Stream> execute(ForwardIterator params_first, ForwardIterator params_last,
            error_code&, error_info&);

    /// Executes a statement (iterator, sync with exceptions version).
    template <typename ForwardIterator>
    resultset<Stream> execute(ForwardIterator params_first, ForwardIterator params_last);

    /**
     * \brief Executes a statement (iterator, async without error_info version).
     * \details The handler signature for this operation is
     * `void(boost::mysql::error_code, boost::mysql::resultset<Stream>)`.
     *
     * The sequence [params_first, params_last) and the values that may be pointed
     * by the elements of the sequence need **not** be kept alive.
     */
    template <
        typename ForwardIterator,
            BOOST_ASIO_COMPLETION_TOKEN_FOR(void(error_code, resultset<Stream>))
            CompletionToken
            BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(executor_type)
    >
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code, resultset<Stream>))
    async_execute(
        ForwardIterator params_first,
        ForwardIterator params_last,
        CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(executor_type)
    )
    {
        return async_execute(params_first, params_last, shared_info(), std::forward<CompletionToken>(token));
    }

    /// Executes a statement (iterator, async with error_info version).
    template <
        typename ForwardIterator,
            BOOST_ASIO_COMPLETION_TOKEN_FOR(void(error_code, resultset<Stream>))
            CompletionToken
            BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(executor_type)
    >
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code, resultset<Stream>))
    async_execute(
        ForwardIterator params_first,
        ForwardIterator params_last,
        error_info& output_info,
        CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(executor_type)
    );

    /**
     * \brief Closes a prepared statement, deallocating it from the server (sync with error code version).
     * \details This function may be used to free resources from the server.
     * Closing the parent connection object causes all prepared statements associated
     * with the connection to also be deallocated from the server. Thus, you should be
     * using close() when reusing connection objects but creating and destroying prepared
     * statements. If you do not reuse connections, or if you just create your prepared
     * statements at application startup, it is not required to call this function.
     *
     * After calling this function, no further functions may be called on this prepared
     * statement, other than assignment. Failing to do so results in undefined behavior.
     */
    void close(error_code&, error_info&);

    /// Closes a prepared statement, deallocating it from the server (sync with exceptions version).
    void close();

    /**
     * \brief Closes a prepared statement, deallocating it from the server (async without error_info version).
     * \details The handler signature for this operation is
     * `void(boost::mysql::error_code)`.
     */
    template <
        BOOST_ASIO_COMPLETION_TOKEN_FOR(void(error_code))
        CompletionToken
        BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(executor_type)
    >
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code))
    async_close(CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(executor_type))
    {
        return async_close(shared_info(), std::forward<CompletionToken>(token));
    }

    /// Closes a prepared statement, deallocating it from the server (async with error_info version).
    template <
        BOOST_ASIO_COMPLETION_TOKEN_FOR(void(error_code))
        CompletionToken
        BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(executor_type)
    >
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code))
    async_close(
        error_info& output_info,
        CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(executor_type)
    );

    /// Rebinds the prepared statement type to another executor.
    template <typename Executor>
    struct rebind_executor
    {
        /// The prepared statement type when rebound to the specified executor.
        using other = prepared_statement<
            typename Stream:: template rebind_executor<Executor>::other
        >;
    };
};

/**
 * \ingroup stmt
 * \brief A prepared statement associated to a boost::mysql::tcp_connection.
 */
using tcp_prepared_statement = prepared_statement<boost::asio::ip::tcp::socket>;

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS) || defined(BOOST_MYSQL_DOXYGEN)

/**
 * \ingroup stmt
 * \brief A prepared statement associated to a boost::mysql::unix_connection.
 */
using unix_prepared_statement = prepared_statement<boost::asio::local::stream_protocol::socket>;

#endif

} // mysql
} // boost

#include "boost/mysql/impl/prepared_statement.hpp"

#endif /* INCLUDE_BOOST_MYSQL_PREPARED_STATEMENT_HPP_ */
