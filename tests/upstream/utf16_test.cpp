#include "helpers.hpp"

#include <string>

namespace iconv = polycpp::iconv_lite;
using iconv_lite_upstream_test::expect_round_trip;
using iconv_lite_upstream_test::from_hex;

TEST(iconv_lite_upstream, utf16_test_vectors) {
    // Adapted from upstream test/utf16-test.js.
    constexpr auto test = "1aя中文☃💩";
    constexpr auto sample = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<俄语>данные</俄语>";
    constexpr auto utf16be_hex = "00310061044f4e2d65872603d83ddca9";
    constexpr auto utf16le_hex = "310061004f042d4e876503263dd8a9dc";

    expect_round_trip(test, "UTF16-BE", utf16be_hex);
    EXPECT_EQ(iconv::decode(from_hex("006100"), "UTF16-BE"), "a");

    EXPECT_EQ(iconv::encode(test, "utf-16").toString("hex"), std::string("fffe") + utf16le_hex);
    iconv::EncodeOptions use_utf16le;
    use_utf16le.default_encoding = "UTF-16LE";
    EXPECT_EQ(iconv::encode(test, "utf-16", use_utf16le).toString("hex"), std::string("fffe") + utf16le_hex);

    EXPECT_EQ(iconv::decode(from_hex(std::string("fffe") + utf16le_hex), "utf-16"), test);
    EXPECT_EQ(iconv::decode(from_hex(std::string("feff") + utf16be_hex), "utf-16"), test);
    EXPECT_EQ(iconv::decode(iconv::Buffer::from({0x61}), "utf-16"), "");
    EXPECT_EQ(iconv::decode(iconv::encode(sample, "utf-16le"), "utf-16"), sample);
    EXPECT_EQ(iconv::decode(iconv::encode(sample, "utf-16be"), "utf-16"), sample);
    EXPECT_EQ(iconv::decode(from_hex(utf16le_hex), "utf-16"), test);

    iconv::DecodeOptions default_utf16le;
    default_utf16le.default_encoding = "utf-16le";
    EXPECT_EQ(iconv::decode(from_hex(utf16le_hex), "utf-16", default_utf16le), test);
}
