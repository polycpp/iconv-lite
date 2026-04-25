#include <gtest/gtest.h>

#include <polycpp/core/error.hpp>
#include <polycpp/iconv_lite/iconv_lite.hpp>

namespace iconv = polycpp::iconv_lite;

TEST(iconv_lite, canonicalizes_encoding_labels) {
    EXPECT_EQ(iconv::canonicalize_encoding("ISO_8859-5:1988"), "iso88595");
    EXPECT_EQ(iconv::canonicalize_encoding("UTF-16LE"), "utf16le");
    EXPECT_EQ(iconv::canonicalize_encoding("__proto__"), "proto");
}

TEST(iconv_lite, reports_supported_and_unsupported_encodings) {
    EXPECT_TRUE(iconv::encoding_exists("utf8"));
    EXPECT_TRUE(iconv::encoding_exists("win1251"));
    EXPECT_TRUE(iconv::encoding_exists("1251"));
    EXPECT_TRUE(iconv::encoding_exists("gb18030"));
    EXPECT_TRUE(iconv::encoding_exists("big5hkscs"));
    EXPECT_TRUE(iconv::encoding_exists("utf7imap"));
    EXPECT_FALSE(iconv::encoding_exists("__proto__"));
    EXPECT_FALSE(iconv::encoding_exists("constructor"));
}

TEST(iconv_lite, round_trips_core_internal_encodings) {
    const auto ascii = iconv::encode("Hello123!", "utf8");
    EXPECT_EQ(ascii.toString("hex"), "48656c6c6f31323321");
    EXPECT_EQ(iconv::decode(ascii, "utf8"), "Hello123!");

    const auto from_base64 = iconv::encode("SGVsbG8xMjMh", "base64");
    EXPECT_EQ(from_base64.toString("hex"), "48656c6c6f31323321");
    EXPECT_EQ(iconv::decode(from_base64, "base64"), "SGVsbG8xMjMh");

    const auto from_hex = iconv::encode("48656c6c6f31323321", "hex");
    EXPECT_EQ(from_hex.toString("latin1"), "Hello123!");
    EXPECT_EQ(iconv::decode(from_hex, "hex"), "48656c6c6f31323321");
}

TEST(iconv_lite, matches_selected_single_byte_encodings) {
    const auto latin1 = iconv::encode("Hello123!£Å÷×çþÿ¿®", "latin1");
    EXPECT_EQ(latin1.toString("hex"), "48656c6c6f31323321a3c5f7d7e7feffbfae");
    EXPECT_EQ(iconv::decode(latin1, "latin1"), "Hello123!£Å÷×çþÿ¿®");

    EXPECT_EQ(iconv::encode("外国人", "latin1").toString("hex"), "3f3f3f");
    EXPECT_EQ(iconv::encode("外国人", "binary").toString("hex"), "16fdba");
    EXPECT_EQ(iconv::encode("é", "ascii").toString("hex"), "3f");
    EXPECT_EQ(iconv::encode("¢€", "iso885915").toString("hex"), "a2a4");
}

TEST(iconv_lite, matches_selected_legacy_multibyte_encodings) {
    const auto win1251 = iconv::encode("Привет", "win1251");
    EXPECT_EQ(win1251.toString("hex"), "cff0e8e2e5f2");
    EXPECT_EQ(iconv::decode(win1251, "1251"), "Привет");

    const auto gbk = iconv::encode("中文", "gbk");
    EXPECT_EQ(gbk.toString("hex"), "d6d0cec4");
    EXPECT_EQ(iconv::decode(gbk, "gb18030"), "中文");

    const auto shiftjis = iconv::encode("こんにちは", "shiftjis");
    EXPECT_EQ(shiftjis.toString("hex"), "82b182f182c982bf82cd");
    EXPECT_EQ(iconv::decode(shiftjis, "sjis"), "こんにちは");

    const auto euckr = iconv::encode("안녕", "euckr");
    EXPECT_EQ(euckr.toString("hex"), "bec8b3e7");
    EXPECT_EQ(iconv::decode(euckr, "korean"), "안녕");
}

TEST(iconv_lite, handles_utf7_and_cesu8_codecs) {
    const auto utf7 = iconv::encode("Hi + 你好 &", "utf7");
    EXPECT_EQ(utf7.toString("hex"), "4869202b2d202b5432425a66512d202b4143592d");
    EXPECT_EQ(iconv::decode(utf7, "utf7"), "Hi + 你好 &");

    const auto utf7imap = iconv::encode("Hi + 你好 &", "utf7imap");
    EXPECT_EQ(utf7imap.toString("hex"), "4869202b20265432425a66512d20262d");
    EXPECT_EQ(iconv::decode(utf7imap, "utf7imap"), "Hi + 你好 &");

    const auto cesu8 = iconv::encode("💩", "cesu8");
    EXPECT_EQ(cesu8.toString("hex"), "eda0bdedb2a9");
    EXPECT_EQ(iconv::decode(cesu8, "cesu8"), "💩");
}

TEST(iconv_lite, handles_utf16_and_utf32_bom_semantics) {
    constexpr auto sample = "1aя中文☃💩";

    const auto utf16be = iconv::encode(sample, "utf16be");
    EXPECT_EQ(utf16be.toString("hex"), "00310061044f4e2d65872603d83ddca9");
    EXPECT_EQ(iconv::decode(utf16be, "utf16be"), sample);

    const auto utf16 = iconv::encode(sample, "utf16");
    EXPECT_EQ(utf16.toString("hex"), "fffe310061004f042d4e876503263dd8a9dc");
    EXPECT_EQ(iconv::decode(utf16, "utf16"), sample);

    iconv::EncodeOptions no_bom;
    no_bom.add_bom = false;
    EXPECT_EQ(iconv::encode(sample, "utf16", no_bom).toString("hex"), "310061004f042d4e876503263dd8a9dc");

    const auto utf32 = iconv::encode(sample, "utf32");
    EXPECT_EQ(utf32.toString("hex"), "fffe000031000000610000004f0400002d4e00008765000003260000a9f40100");
    EXPECT_EQ(iconv::decode(utf32, "utf32"), sample);

    iconv::EncodeOptions utf32be_options;
    utf32be_options.default_encoding = "utf32be";
    EXPECT_EQ(iconv::encode(sample, "utf32", utf32be_options).toString("hex"),
              "0000feff00000031000000610000044f00004e2d00006587000026030001f4a9");
}

TEST(iconv_lite, strips_bom_by_default_for_bom_aware_codecs) {
    const auto utf8_bom = iconv::Buffer::from({0xEF, 0xBB, 0xBF, 0x41});
    EXPECT_EQ(iconv::decode(utf8_bom, "utf8"), "A");

    iconv::DecodeOptions keep_bom;
    keep_bom.strip_bom = false;
    EXPECT_EQ(iconv::decode(utf8_bom, "utf8", keep_bom), std::string("\xEF\xBB\xBF", 3) + "A");

    const auto latin1_bytes = iconv::Buffer::from({0xEF, 0xBB, 0xBF, 0x41});
    EXPECT_EQ(iconv::decode(latin1_bytes, "latin1"), "ï»¿A");
}

TEST(iconv_lite, throws_for_unknown_encoding) {
    EXPECT_THROW((void)iconv::encode("abc", "not-a-codec"), polycpp::TypeError);
    EXPECT_THROW((void)iconv::decode(iconv::Buffer::from("abc"), "constructor"), polycpp::TypeError);
}
