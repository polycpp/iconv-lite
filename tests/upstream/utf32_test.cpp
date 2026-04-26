#include "helpers.hpp"

#include <string>

namespace iconv = polycpp::iconv_lite;
using iconv_lite_upstream_test::expect_round_trip;
using iconv_lite_upstream_test::from_hex;

TEST(iconv_lite_upstream, utf32_test_vectors) {
    // Adapted from upstream test/utf32-test.js.
    constexpr auto test = "1aя中文☃💩";
    constexpr auto sample = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<俄语>данные</俄语>";
    constexpr auto utf32le_hex = "31000000610000004f0400002d4e00008765000003260000a9f40100";
    constexpr auto utf32be_hex = "00000031000000610000044f00004e2d00006587000026030001f4a9";

    expect_round_trip(test, "UTF32-LE", utf32le_hex);
    expect_round_trip(test, "UTF32-BE", utf32be_hex);
    EXPECT_EQ(iconv::decode(from_hex("6100000000"), "UTF32-LE"), "a");
    EXPECT_EQ(iconv::decode(from_hex("0000006100"), "UTF32-BE"), "a");
    EXPECT_EQ(iconv::decode(from_hex(std::string(utf32le_hex) + "12345678"), "utf-32le"), std::string(test) + "�");
    EXPECT_EQ(iconv::decode(from_hex(std::string(utf32be_hex) + "12345678"), "utf-32be"), std::string(test) + "�");

    EXPECT_EQ(iconv::encode(test, "utf-32").toString("hex"), std::string("fffe0000") + utf32le_hex);

    iconv::EncodeOptions no_bom_be;
    no_bom_be.addBOM = false;
    no_bom_be.defaultEncoding = "ucs4be";
    EXPECT_EQ(iconv::encode(test, "ucs4", no_bom_be).toString("hex"), utf32be_hex);

    EXPECT_EQ(iconv::decode(from_hex(std::string("fffe0000") + utf32le_hex), "utf-32"), test);
    EXPECT_EQ(iconv::decode(iconv::encode(sample, "utf-32-le"), "utf-32"), sample);
    EXPECT_EQ(iconv::decode(iconv::encode(sample, "utf-32-be"), "utf-32"), sample);

    iconv::DecodeOptions keep_bom;
    keep_bom.stripBOM = false;
    EXPECT_EQ(iconv::decode(from_hex(std::string("0000feff") + utf32be_hex), "utf-32", keep_bom),
              std::string("\xEF\xBB\xBF", 3) + test);
}
