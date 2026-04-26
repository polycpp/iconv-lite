#include "helpers.hpp"

namespace iconv = polycpp::iconv_lite;
using iconv_lite_upstream_test::expect_round_trip;
using iconv_lite_upstream_test::from_hex;

TEST(iconv_lite_upstream, shiftjis_test_vectors) {
    // Adapted from upstream test/shiftjis-test.js.
    expect_round_trip("中文abc", "shiftjis", "928695b6616263");
    expect_round_trip("測試", "shiftjis", "91aa8e8e");

    EXPECT_EQ(iconv::decode(from_hex("ed40eefceeef"), "shiftjis"), "纊＂ⅰ");
    EXPECT_EQ(iconv::decode(from_hex("f040f2fcf940"), "shiftjis"), "");
    EXPECT_EQ(iconv::encode("纊＂ⅰ", "shiftjis").toString("hex"), "fa5cfa57fa40");
    EXPECT_EQ(iconv::encode("", "shiftjis").toString("hex"), "3f3f3f");
    expect_round_trip("①", "shiftjis", "8740");
}
