#ifndef MYSQL_ASIO_IMPL_QUERY_HPP
#define MYSQL_ASIO_IMPL_QUERY_HPP

#include "mysql/resultset.hpp"
#include "mysql/error.hpp"
#include "mysql/impl/capabilities.hpp"
#include <string_view>

namespace mysql
{
namespace detail
{

template <typename ChannelType>
using channel_stream_type = typename ChannelType::stream_type;

template <typename ChannelType>
using channel_resultset_type = resultset<channel_stream_type<ChannelType>>;

enum class fetch_result
{
	error,
	row,
	eof
};

template <typename ChannelType>
void execute_query(
	ChannelType& channel,
	std::string_view query,
	channel_resultset_type<ChannelType>& output,
	error_code& err,
	error_info& info
);

template <typename ChannelType, typename CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(error_code, error_info, channel_resultset_type<ChannelType>))
async_execute_query(
	ChannelType& channel,
	std::string_view query,
	CompletionToken&& token
);


template <typename ChannelType>
fetch_result fetch_text_row(
	ChannelType& channel,
	const std::vector<field_metadata>& meta,
	bytestring& buffer,
	std::vector<value>& output_values,
	ok_packet& output_ok_packet,
	error_code& err,
	error_info& info
);

template <typename ChannelType, typename CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(error_code, error_info, fetch_result))
async_fetch_text_row(
	ChannelType& channel,
	const std::vector<field_metadata>& meta,
	bytestring& buffer,
	std::vector<value>& output_values,
	ok_packet& output_ok_packet,
	CompletionToken&& token
);


}
}

#include "mysql/impl/query.ipp"

#endif
