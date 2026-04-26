#include "helpers.hpp"

#include <string>

namespace iconv = polycpp::iconv_lite;
using iconv_lite_upstream_test::expect_decoded;
using iconv_lite_upstream_test::expect_round_trip;
using iconv_lite_upstream_test::from_hex;

TEST(iconv_lite_upstream, gbk_test_vectors) {
    // Adapted from upstream test/gbk-test.js.
    expect_round_trip("中国abc", "GBK", "d6d0b9fa616263");
    expect_round_trip("中国abc", "GB2312", "d6d0b9fa616263");
    expect_round_trip("·×", "GBK", "a1a4a1c1");
    EXPECT_EQ(iconv::decode(from_hex("80"), "GBK"), "€");
    EXPECT_EQ(iconv::decode(from_hex("a2e3"), "GBK"), "€");
    EXPECT_EQ(iconv::encode("€", "GBK").toString("hex"), "80");
}

TEST(iconv_lite_upstream, gb18030_four_byte_and_incomplete_vectors) {
    // Adapted from upstream test/gbk-test.js.
    struct FourByteFixture {
        const char* text;
        const char* hex;
    };
    const FourByteFixture fixtures[] = {
#ifdef _MSC_VER
        // MSVC converts \u00XX escapes through the execution character set
        // (code page 1252), which corrupts C1 control characters (U+0080-
        // U+009F).  Use explicit UTF-8 byte sequences instead.
        {"\xC2\x80", "81308130"},  // U+0080
        {"\xC2\x81", "81308131"},  // U+0081
        {"\xC2\x8B", "81308231"},  // U+008B
#else
        {"\u0080", "81308130"},
        {"\u0081", "81308131"},
        {"\u008B", "81308231"},
#endif
        {"ؕ", "81318231"},
        {"㦟", "82318231"},
        {"\xF4\x86\x99\xB7", "e0318231"},
    };

    for (const auto& fixture : fixtures) {
        expect_round_trip(fixture.text, "GB18030", fixture.hex);
    }

    struct DecodeFixture {
        const char* text;
        const char* hex;
    };
    const DecodeFixture incomplete[] = {
        {"�", "82"},
        {"�1", "8231"},
        {"�1�", "823182"},
        {"㦟", "82318231"},
        {"� ", "8220"},
        {"�1 ", "823120"},
        {"�1� ", "82318220"},
        {"㦟 ", "8231823120"},
        {"�1俛", "82318261"},
        {"�1倐a", "8231828261"},
        {"㦟俛", "823182318261"},
        {"�1倐1�1", "82318282318231"},
    };

    for (const auto& fixture : incomplete) {
        expect_decoded(fixture.hex, "GB18030", fixture.text);
    }

    EXPECT_EQ(iconv::decode(from_hex("a8bc008135f437"), "GB18030"), std::string("ḿ") + '\0' + "");
    EXPECT_EQ(iconv::encode(std::string("ḿ") + '\0' + "", "GB18030").toString("hex"), "a8bc008135f437");
    EXPECT_EQ(iconv::encode("€", "GB18030").toString("hex"), "a2e3");
}
