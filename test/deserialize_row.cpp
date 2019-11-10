/*
 * deserialize_row.cpp
 *
 *  Created on: Nov 8, 2019
 *      Author: ruben
 */

#include <gtest/gtest.h>
#include <boost/type_index.hpp>
#include "mysql/impl/deserialize_row.hpp"

using namespace mysql;
using namespace mysql::detail;
using namespace testing;
using namespace ::date::literals;

namespace
{

struct ValuePrinter
{
	std::ostream& os;

	template <typename T>
	void print(T value) const { os << value; }

	void print(nullptr_t) const { os << "NULL"; }
	void print(mysql::date v) const { ::date::operator<<(os, v); }
	void print(mysql::datetime v) const { print(std::chrono::time_point_cast<::date::days>(v)); }
	void print(mysql::time v) const { ::date::operator<<(os, v); }

	template <typename T>
	void operator()(T content) const
	{
		os << "<" << boost::typeindex::type_id<decltype(content)>().pretty_name() << ">(";
		print(content);
		os << ")";
	}
};

std::ostream& operator<<(std::ostream& os, const value& v)
{
	os << "mysql::value";
	std::visit(ValuePrinter{os}, v);
	return os;
}

struct TextValueParam
{
	std::string name;
	std::string_view from;
	value expected;
	field_type type;
	unsigned decimals;
	bool unsign;

	template <typename T>
	TextValueParam(
		std::string name,
		std::string_view from,
		T&& expected_value,
		field_type type,
		bool unsign=false,
		unsigned decimals=0
	):
		name(std::move(name)),
		from(from),
		expected(std::forward<T>(expected_value)),
		type(type),
		decimals(decimals),
		unsign(unsign)
	{
	};
};

std::ostream& operator<<(std::ostream& os, const TextValueParam& value) { return os << value.name; }

struct DeserializeTextValueTest : public TestWithParam<TextValueParam>
{
};

TEST_P(DeserializeTextValueTest, CorrectFormat_SetsOutputValueReturnsTrue)
{
	msgs::column_definition coldef;
	coldef.type = GetParam().type;
	coldef.decimals.value = static_cast<std::uint8_t>(GetParam().decimals);
	coldef.flags.value = GetParam().unsign ? column_flags::unsigned_ : 0;
	field_metadata meta (coldef);
	value actual_value;
	auto err = deserialize_text_value(GetParam().from, meta, actual_value);
	EXPECT_EQ(err, Error::ok);
	EXPECT_EQ(actual_value, GetParam().expected);
}

INSTANTIATE_TEST_SUITE_P(VARCHAR, DeserializeTextValueTest, Values(
	TextValueParam("non-empty", "string", "string", field_type::var_string),
	TextValueParam("empty", "", "", field_type::var_string)
));

INSTANTIATE_TEST_SUITE_P(TINYINT, DeserializeTextValueTest, Values(
	TextValueParam("signed", "20", std::int32_t(20), field_type::tiny),
	TextValueParam("signed max", "127", std::int32_t(127), field_type::tiny),
	TextValueParam("signed negative", "-20", std::int32_t(-20), field_type::tiny),
	TextValueParam("signed negative max", "-128", std::int32_t(-128), field_type::tiny),
	TextValueParam("unsigned", "20", std::uint32_t(20), field_type::tiny, true),
	TextValueParam("usigned min", "0", std::uint32_t(0), field_type::tiny, true),
	TextValueParam("usigned max", "255", std::uint32_t(255), field_type::tiny, true),
	TextValueParam("usigned zerofill", "010", std::uint32_t(10), field_type::tiny, true)
));

INSTANTIATE_TEST_SUITE_P(SMALLINT, DeserializeTextValueTest, Values(
	TextValueParam("signed", "20", std::int32_t(20), field_type::short_),
	TextValueParam("signed max", "32767", std::int32_t(32767), field_type::short_),
	TextValueParam("signed negative", "-20", std::int32_t(-20), field_type::short_),
	TextValueParam("signed negative max", "-32768", std::int32_t(-32768), field_type::short_),
	TextValueParam("unsigned", "20", std::uint32_t(20), field_type::short_, true),
	TextValueParam("usigned min", "0", std::uint32_t(0), field_type::short_, true),
	TextValueParam("usigned max", "65535", std::uint32_t(65535), field_type::short_, true),
	TextValueParam("usigned zerofill", "00535", std::uint32_t(535), field_type::short_, true)
));

INSTANTIATE_TEST_SUITE_P(MEDIUMINT, DeserializeTextValueTest, Values(
	TextValueParam("signed", "20", std::int32_t(20), field_type::int24),
	TextValueParam("signed max", "8388607", std::int32_t(8388607), field_type::int24),
	TextValueParam("signed negative", "-20", std::int32_t(-20), field_type::int24),
	TextValueParam("signed negative max", "-8388607", std::int32_t(-8388607), field_type::int24),
	TextValueParam("unsigned", "20", std::uint32_t(20), field_type::int24, true),
	TextValueParam("usigned min", "0", std::uint32_t(0), field_type::int24, true),
	TextValueParam("usigned max", "16777215", std::uint32_t(16777215), field_type::int24, true),
	TextValueParam("usigned zerofill", "00007215", std::uint32_t(7215), field_type::int24, true)
));

INSTANTIATE_TEST_SUITE_P(INT, DeserializeTextValueTest, Values(
	TextValueParam("signed", "20", std::int32_t(20), field_type::long_),
	TextValueParam("signed max", "2147483647", std::int32_t(2147483647), field_type::long_),
	TextValueParam("signed negative", "-20", std::int32_t(-20), field_type::long_),
	TextValueParam("signed negative max", "-2147483648", std::int32_t(-2147483648), field_type::long_),
	TextValueParam("unsigned", "20", std::uint32_t(20), field_type::long_, true),
	TextValueParam("usigned min", "0", std::uint32_t(0), field_type::long_, true),
	TextValueParam("usigned max", "4294967295", std::uint32_t(4294967295), field_type::long_, true),
	TextValueParam("usigned zerofill", "0000067295", std::uint32_t(67295), field_type::long_, true)
));

INSTANTIATE_TEST_SUITE_P(BIGINT, DeserializeTextValueTest, Values(
	TextValueParam("signed", "20", std::int64_t(20), field_type::longlong),
	TextValueParam("signed max", "9223372036854775807", std::int64_t(9223372036854775807), field_type::longlong),
	TextValueParam("signed negative", "-20", std::int64_t(-20), field_type::longlong),
	TextValueParam("signed negative max", "-9223372036854775808", std::numeric_limits<std::int64_t>::min(), field_type::longlong),
	TextValueParam("unsigned", "20", std::uint64_t(20), field_type::longlong, true),
	TextValueParam("usigned min", "0", std::uint64_t(0), field_type::longlong, true),
	TextValueParam("usigned max", "18446744073709551615", std::uint64_t(18446744073709551615ULL), field_type::longlong, true),
	TextValueParam("usigned max", "000615", std::uint64_t(615), field_type::longlong, true)
));

INSTANTIATE_TEST_SUITE_P(FLOAT, DeserializeTextValueTest, Values(
	TextValueParam("zero", "0", 0.0f, field_type::float_),
	TextValueParam("integer positive", "4", 4.0f, field_type::float_),
	TextValueParam("integer negative", "-5", -5.0f, field_type::float_),
	TextValueParam("fractional positive", "3.147", 3.147f, field_type::float_),
	TextValueParam("fractional negative", "-3.147", -3.147f, field_type::float_),
	TextValueParam("positive exponent positive integer", "3e20", 3e20f, field_type::float_),
	TextValueParam("positive exponent negative integer", "-3e20", -3e20f, field_type::float_),
	TextValueParam("positive exponent positive fractional", "3.14e20", 3.14e20f, field_type::float_),
	TextValueParam("positive exponent negative fractional", "-3.45e20", -3.45e20f, field_type::float_),
	TextValueParam("negative exponent positive integer", "3e-20", 3e-20f, field_type::float_),
	TextValueParam("negative exponent negative integer", "-3e-20", -3e-20f, field_type::float_),
	TextValueParam("negative exponent positive fractional", "3.14e-20", 3.14e-20f, field_type::float_),
	TextValueParam("negative exponent negative fractional", "-3.45e-20", -3.45e-20f, field_type::float_)
));

INSTANTIATE_TEST_SUITE_P(DOUBLE, DeserializeTextValueTest, Values(
	TextValueParam("zero", "0", 0.0, field_type::double_),
	TextValueParam("integer positive", "4", 4.0, field_type::double_),
	TextValueParam("integer negative", "-5", -5.0, field_type::double_),
	TextValueParam("fractional positive", "3.147", 3.147, field_type::double_),
	TextValueParam("fractional negative", "-3.147", -3.147, field_type::double_),
	TextValueParam("positive exponent positive integer", "3e20", 3e20, field_type::double_),
	TextValueParam("positive exponent negative integer", "-3e20", -3e20, field_type::double_),
	TextValueParam("positive exponent positive fractional", "3.14e20", 3.14e20, field_type::double_),
	TextValueParam("positive exponent negative fractional", "-3.45e20", -3.45e20, field_type::double_),
	TextValueParam("negative exponent positive integer", "3e-20", 3e-20, field_type::double_),
	TextValueParam("negative exponent negative integer", "-3e-20", -3e-20, field_type::double_),
	TextValueParam("negative exponent positive fractional", "3.14e-20", 3.14e-20, field_type::double_),
	TextValueParam("negative exponent negative fractional", "-3.45e-20", -3.45e-20, field_type::double_)
));

INSTANTIATE_TEST_SUITE_P(DATE, DeserializeTextValueTest, Values(
	TextValueParam("regular date", "2019-02-28", mysql::date(2019_y/2/28), field_type::date),
	TextValueParam("leap year", "1788-02-29", mysql::date(1788_y/2/29), field_type::date),
	TextValueParam("min", "1000-01-01", mysql::date(1000_y/1/1), field_type::date),
	TextValueParam("max", "9999-12-31", mysql::date(9999_y/12/31), field_type::date),
	TextValueParam("unofficial min", "0100-01-01", mysql::date(100_y/1/1), field_type::date)
));

datetime makedt(int years, int months, int days, int hours=0, int mins=0, int secs=0, int micros=0)
{
	return mysql::date(::date::year(years)/months/days) +
		   std::chrono::hours(hours) + std::chrono::minutes(mins) +
		   std::chrono::seconds(secs) + std::chrono::microseconds(micros);
}

INSTANTIATE_TEST_SUITE_P(DATETIME, DeserializeTextValueTest, Values(
	TextValueParam("0 decimals, only date", "2010-02-15 00:00:00", makedt(2010, 2, 15), field_type::datetime),
	TextValueParam("0 decimals, date, h", "2010-02-15 02:00:00", makedt(2010, 2, 15, 2), field_type::datetime),
	TextValueParam("0 decimals, date, hm", "2010-02-15 02:05:00", makedt(2010, 2, 15, 2, 5), field_type::datetime),
	TextValueParam("0 decimals, date, hms", "2010-02-15 02:05:30", makedt(2010, 2, 15, 2, 5, 30), field_type::datetime),
	TextValueParam("0 decimals, min", "1000-01-01 00:00:00", makedt(1000, 1, 1), field_type::datetime),
	TextValueParam("0 decimals, max", "9999-12-31 23:59:59", makedt(9999, 12, 31, 23, 59, 59), field_type::datetime),

	TextValueParam("1 decimals, only date", "2010-02-15 00:00:00.0", makedt(2010, 2, 15), field_type::datetime, false, 1),
	TextValueParam("1 decimals, date, h", "2010-02-15 02:00:00.0", makedt(2010, 2, 15, 2), field_type::datetime, false, 1),
	TextValueParam("1 decimals, date, hm", "2010-02-15 02:05:00.0", makedt(2010, 2, 15, 2, 5), field_type::datetime, false, 1),
	TextValueParam("1 decimals, date, hms", "2010-02-15 02:05:30.0", makedt(2010, 2, 15, 2, 5, 30), field_type::datetime, false, 1),
	TextValueParam("1 decimals, date, hmsu", "2010-02-15 02:05:30.5", makedt(2010, 2, 15, 2, 5, 30, 500000), field_type::datetime, false, 1),
	TextValueParam("1 decimals, min", "1000-01-01 00:00:00.0", makedt(1000, 1, 1), field_type::datetime, false, 1),
	TextValueParam("1 decimals, max", "9999-12-31 23:59:59.9", makedt(9999, 12, 31, 23, 59, 59, 900000), field_type::datetime, false, 1),

	TextValueParam("2 decimals, date, hms", "2010-02-15 02:05:30.00", makedt(2010, 2, 15, 2, 5, 30), field_type::datetime, false, 2),
	TextValueParam("2 decimals, date, hmsu", "2010-02-15 02:05:30.05", makedt(2010, 2, 15, 2, 5, 30, 50000), field_type::datetime, false, 2),
	TextValueParam("2 decimals, min", "1000-01-01 00:00:00.00", makedt(1000, 1, 1), field_type::datetime, false, 2),
	TextValueParam("2 decimals, max", "9999-12-31 23:59:59.99", makedt(9999, 12, 31, 23, 59, 59, 990000), field_type::datetime, false, 2),

	TextValueParam("3 decimals, date, hms", "2010-02-15 02:05:30.000", makedt(2010, 2, 15, 2, 5, 30), field_type::datetime, false, 3),
	TextValueParam("3 decimals, date, hmsu", "2010-02-15 02:05:30.420", makedt(2010, 2, 15, 2, 5, 30, 420000), field_type::datetime, false, 3),
	TextValueParam("3 decimals, min", "1000-01-01 00:00:00.000", makedt(1000, 1, 1), field_type::datetime, false, 3),
	TextValueParam("3 decimals, max", "9999-12-31 23:59:59.999", makedt(9999, 12, 31, 23, 59, 59, 999000), field_type::datetime, false, 3),

	TextValueParam("4 decimals, date, hms", "2010-02-15 02:05:30.0000", makedt(2010, 2, 15, 2, 5, 30), field_type::datetime, false, 4),
	TextValueParam("4 decimals, date, hmsu", "2010-02-15 02:05:30.4267", makedt(2010, 2, 15, 2, 5, 30, 426700), field_type::datetime, false, 4),
	TextValueParam("4 decimals, min", "1000-01-01 00:00:00.0000", makedt(1000, 1, 1), field_type::datetime, false, 4),
	TextValueParam("4 decimals, max", "9999-12-31 23:59:59.9999", makedt(9999, 12, 31, 23, 59, 59, 999900), field_type::datetime, false, 4),

	TextValueParam("5 decimals, date, hms", "2010-02-15 02:05:30.00000", makedt(2010, 2, 15, 2, 5, 30), field_type::datetime, false, 5),
	TextValueParam("5 decimals, date, hmsu", "2010-02-15 02:05:30.00239", makedt(2010, 2, 15, 2, 5, 30, 2390), field_type::datetime, false, 5),
	TextValueParam("5 decimals, min", "1000-01-01 00:00:00.00000", makedt(1000, 1, 1), field_type::datetime, false, 5),
	TextValueParam("5 decimals, max", "9999-12-31 23:59:59.99999", makedt(9999, 12, 31, 23, 59, 59, 999990), field_type::datetime, false, 5),

	TextValueParam("6 decimals, date, hms", "2010-02-15 02:05:30.000000", makedt(2010, 2, 15, 2, 5, 30), field_type::datetime, false, 6),
	TextValueParam("6 decimals, date, hmsu", "2010-02-15 02:05:30.002395", makedt(2010, 2, 15, 2, 5, 30, 2395), field_type::datetime, false, 6),
	TextValueParam("6 decimals, min", "1000-01-01 00:00:00.000000", makedt(1000, 1, 1), field_type::datetime, false, 6),
	TextValueParam("6 decimals, max", "9999-12-31 23:59:59.999999", makedt(9999, 12, 31, 23, 59, 59, 999999), field_type::datetime, false, 6)
));


INSTANTIATE_TEST_SUITE_P(YEAR, DeserializeTextValueTest, Values(
	TextValueParam("regular value", "1999", year(1999), field_type::year),
	TextValueParam("min", "1901", year(1901), field_type::year),
	TextValueParam("max", "2155", year(2155), field_type::year),
	TextValueParam("zero", "0000", year(0), field_type::year)
));

}

