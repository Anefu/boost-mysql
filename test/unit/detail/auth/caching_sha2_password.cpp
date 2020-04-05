
#include "boost/mysql/detail/auth/caching_sha2_password.hpp"
#include <gtest/gtest.h>
#include <array>

using namespace boost::mysql::detail;

TEST(CachingSha2Password, ComputeAuthString_NonEmptyPassword_ReturnsExpectedHash)
{
	// Values snooped using the MySQL Python connector
	std::array<std::uint8_t, caching_sha2_password::challenge_length> challenge {
		0x3e, 0x3b, 0x4, 0x55, 0x4, 0x70, 0x16, 0x3a,
		0x4c, 0x15, 0x35, 0x3, 0x15, 0x76, 0x73, 0x22,
		0x46, 0x8, 0x18, 0x1
	};
	std::array<std::uint8_t, caching_sha2_password::response_length> expected {
		0xa1, 0xc1, 0xe1, 0xe9, 0x1b, 0xb6, 0x54, 0x4b,
		0xa7, 0x37, 0x4b, 0x9c, 0x56, 0x6d, 0x69, 0x3e,
		0x6, 0xca, 0x7, 0x2, 0x98, 0xac, 0xd1, 0x6,
		0x18, 0xc6, 0x90, 0x38, 0x9d, 0x88, 0xe1, 0x20
	};
	std::array<std::uint8_t, caching_sha2_password::response_length> actual {};
	const char* password = "hola";
	caching_sha2_password::compute_auth_string(password, challenge.data(), actual.data());
	EXPECT_EQ(expected, actual);
}

