#include "helpers.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace iconv = polycpp::iconv_lite;
using iconv_lite_upstream_test::expect_round_trip;
using iconv_lite_upstream_test::from_hex;

namespace {

void append_utf8(std::string& out, uint32_t cp) {
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

std::string utf8_from_codepoints(std::initializer_list<uint32_t> codepoints) {
    std::string out;
    for (uint32_t cp : codepoints) append_utf8(out, cp);
    return out;
}

void append_u32le(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back(static_cast<uint8_t>(value & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void append_u32be(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(value & 0xFF));
}

}  // namespace

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

TEST(iconv_lite_upstream, utf32_boundary_codepoints_and_invalid_scalars) {
    // Hardens upstream utf32-test.js boundary coverage without the huge
    // native-iconv exhaustive loop.
    const auto boundaries = utf8_from_codepoints({
        0x0000,
        0x0001,
        0xD7FF,
        0xE000,
        0xFFFF,
        0x10000,
        0x10FFFF,
    });

    std::vector<uint8_t> le_bytes;
    std::vector<uint8_t> be_bytes;
    for (uint32_t cp : {0x0000u, 0x0001u, 0xD7FFu, 0xE000u, 0xFFFFu, 0x10000u, 0x10FFFFu}) {
        append_u32le(le_bytes, cp);
        append_u32be(be_bytes, cp);
    }

    const auto le = iconv::Buffer::from(le_bytes);
    const auto be = iconv::Buffer::from(be_bytes);
    EXPECT_EQ(iconv::encode(boundaries, "utf-32le").toString("hex"), le.toString("hex"));
    EXPECT_EQ(iconv::encode(boundaries, "utf-32be").toString("hex"), be.toString("hex"));
    EXPECT_EQ(iconv::decode(le, "utf-32le"), boundaries);
    EXPECT_EQ(iconv::decode(be, "utf-32be"), boundaries);

    std::vector<uint8_t> invalid_le;
    for (uint32_t cp : {0xD800u, 0xDFFFu, 0x110000u, 0xFFFFFFFFu}) {
        append_u32le(invalid_le, cp);
    }
    EXPECT_EQ(iconv::decode(iconv::Buffer::from(invalid_le), "utf-32le"), "����");
}
