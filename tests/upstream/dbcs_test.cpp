#include "helpers.hpp"

#include <string>

#include <polycpp/iconv_lite/detail/generated_tables.hpp>

namespace iconv = polycpp::iconv_lite;
namespace generated = polycpp::iconv_lite::generated;
using iconv_lite_upstream_test::expect_decoded;
using iconv_lite_upstream_test::expect_encoded;
using iconv_lite_upstream_test::expect_round_trip;

TEST(iconv_lite_upstream, dbcs_generated_entries_preserve_ascii) {
    // Complements upstream test/dbcs-test.js by touching every generated DBCS
    // codec entry. The package-specific tests below cover non-ASCII extension
    // and compatibility mappings.
    size_t checked = 0;
    for (const auto& entry : generated::ENCODING_ENTRIES) {
        if (entry.kind != generated::GeneratedKind::dbcs) continue;
        EXPECT_EQ(iconv::decode(iconv::Buffer::from("ABC"), entry.name), "ABC") << entry.name;
        EXPECT_EQ(iconv::encode("ABC", entry.name).toString("hex"), "414243") << entry.name;
        ++checked;
    }
    EXPECT_EQ(checked, 8u);
}

TEST(iconv_lite_upstream, dbcs_extension_and_compatibility_vectors) {
    // Representative vectors from upstream test/dbcs-test.js. Some decoded
    // extension bytes intentionally re-encode to canonical byte sequences.
    expect_decoded("adf0", "eucjp", "≒");
    expect_encoded("≒", "eucjp", "a2e2");
    expect_round_trip("①", "eucjp", "ada1");
    expect_round_trip("髙", "eucjp", "fce2");
    expect_round_trip("∥", "eucjp", "a1c2");

    expect_decoded("8e69", "big5hkscs", "箸");
    expect_encoded("箸", "big5hkscs", "bae6");
    expect_round_trip("‧", "big5hkscs", "a145");
    expect_round_trip("€", "big5hkscs", "a3e1");
    expect_encoded("砉", "big5hkscs", "cff1");

    expect_decoded("a3a0", "gbk", "　");
    expect_encoded("　", "gbk", "a1a1");
    expect_round_trip("⺁", "gbk", "fe50");
    expect_round_trip("⿰", "gbk", "a98a");
    expect_round_trip("ḿ", "gbk", "a8bc");

    expect_round_trip("￥", "cp950", "a244");
    expect_round_trip("안녕", "cp949", "bec8b3e7");
}

TEST(iconv_lite_upstream, dbcs_truncated_lead_bytes_decode_with_replacement) {
    struct Fixture {
        const char* encoding;
        const char* hex;
    };
    const Fixture fixtures[] = {
        {"eucjp", "8f"},
        {"shiftjis", "82"},
        {"big5hkscs", "88"},
        {"cp950", "88"},
        {"cp949", "81"},
        {"gbk", "81"},
        {"gb18030", "82"},
        {"cp936", "81"},
    };

    for (const auto& fixture : fixtures) {
        expect_decoded(fixture.hex, fixture.encoding, "�");
    }
}

