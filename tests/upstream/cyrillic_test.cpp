#include "helpers.hpp"

namespace iconv = polycpp::iconv_lite;
using iconv_lite_upstream_test::expect_round_trip;
using iconv_lite_upstream_test::from_hex;

TEST(iconv_lite_upstream, cyrillic_test_vectors) {
    // Adapted from upstream test/cyrillic-test.js.
    expect_round_trip("Привет!", "win1251", "cff0e8e2e5f221");
    expect_round_trip("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюя",
                      "win1251",
                      "c0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
                      "e0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff");
    expect_round_trip("Привет!", "koi8r", "f0d2c9d7c5d421");
    expect_round_trip("Привет!", "iso88595", "bfe0d8d2d5e221");

    EXPECT_EQ(iconv::encode("£Åçþÿ¿", "win1251").toString("latin1"), "??????");

    const char* aliases[] = {"Windows-1251", "windows1251", "CP1251", "1251"};
    for (const auto* alias : aliases) {
        EXPECT_EQ(iconv::decode(from_hex("cff0e8e2e5f221"), alias), "Привет!") << alias;
        EXPECT_EQ(iconv::encode("Привет!", alias).toString("hex"), "cff0e8e2e5f221") << alias;
    }
}
