#ifndef INCLUDE_MYSQL_STREAM_HPP_
#define INCLUDE_MYSQL_STREAM_HPP_

#include "mysql/error.hpp"
#include "mysql/impl/basic_types.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/async_result.hpp>
#include <array>

namespace mysql
{
namespace detail
{

template <typename AsyncStream>
class MysqlChannel
{
	// TODO: static asserts for AsyncStream concept
	// TODO: actually we also require it to be SyncStream, name misleading
	AsyncStream& next_layer_;
	std::uint8_t sequence_number_ {0};
	std::array<std::uint8_t, 4> header_buffer_ {}; // for async ops

	bool process_sequence_number(std::uint8_t got);
	std::uint8_t next_sequence_number() { return sequence_number_++; }

	error_code process_header_read(std::uint32_t& size_to_read); // reads from header_buffer_
	void process_header_write(std::uint32_t size_to_write); // writes to header_buffer_
public:
	MysqlChannel(AsyncStream& stream): next_layer_ {stream} {};

	template <typename DynamicBuffer>
	void read(DynamicBuffer& buffer, error_code& errc);

	void write(boost::asio::const_buffer buffer, error_code& errc);

	template <typename DynamicBuffer, typename CompletionToken>
	BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(error_code))
	async_read(DynamicBuffer& buffer, CompletionToken&& token);

	template <typename CompletionToken>
	BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(error_code))
	async_write(boost::asio::const_buffer buffer, CompletionToken&& token);

	void reset_sequence_number(std::uint8_t value = 0) { sequence_number_ = value; }
	std::uint8_t sequence_number() const { return sequence_number_; }
};

}
}

#include <mysql/impl/mysql_channel_impl.hpp>

#endif /* INCLUDE_MYSQL_STREAM_HPP_ */
