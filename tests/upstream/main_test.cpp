#include "helpers.hpp"

#include <polycpp/core/error.hpp>

namespace iconv = polycpp::iconv_lite;

TEST(iconv_lite_upstream, main_test_core_api_and_aliases) {
    // Adapted from upstream test/main-test.js.
    constexpr auto test_string = "Hello123!";
    constexpr auto latin1_string = "Hello123!£Å÷×çþÿ¿®";

    EXPECT_FALSE(iconv::encodingExists("__proto__"));
    EXPECT_FALSE(iconv::encodingExists("constructor"));
    EXPECT_TRUE(iconv::encodingExists("utf8"));

    EXPECT_EQ(iconv::encode(test_string, "utf8").toString("hex"), "48656c6c6f31323321");
    EXPECT_EQ(iconv::decode(iconv::Buffer::from(test_string), "utf8"), test_string);
    EXPECT_EQ(iconv::encode("SGVsbG8xMjMh", "base64").toString("latin1"), test_string);
    EXPECT_EQ(iconv::decode(iconv::Buffer::from(test_string), "base64"), "SGVsbG8xMjMh");
    EXPECT_EQ(iconv::encode("48656c6c6f31323321", "hex").toString("latin1"), test_string);
    EXPECT_EQ(iconv::decode(iconv::Buffer::from(test_string), "hex"), "48656c6c6f31323321");
    EXPECT_EQ(iconv::encode(latin1_string, "latin1").toString("hex"),
              "48656c6c6f31323321a3c5f7d7e7feffbfae");
    EXPECT_EQ(iconv::toEncoding(test_string, "latin1").toString("hex"),
              iconv::encode(test_string, "latin1").toString("hex"));
    EXPECT_EQ(iconv::fromEncoding(iconv::Buffer::from(latin1_string), "latin1"),
              iconv::decode(iconv::Buffer::from(latin1_string), "latin1"));
    EXPECT_THROW((void)iconv::encode("a", "xxx"), polycpp::TypeError);
    EXPECT_THROW((void)iconv::decode(iconv::Buffer::from("a"), "xxx"), polycpp::TypeError);
    EXPECT_EQ(iconv::encode("外国人", "latin1").toString("latin1"), "???");
    EXPECT_EQ(iconv::_canonicalizeEncoding("ISO_8859-5:1988"), "iso88595");
}
