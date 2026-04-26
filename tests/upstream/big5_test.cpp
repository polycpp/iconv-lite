#include "helpers.hpp"

using iconv_lite_upstream_test::expect_round_trip;

TEST(iconv_lite_upstream, big5_test_vectors) {
    // Adapted from upstream test/big5-test.js.
    expect_round_trip("中文abc", "big5", "a4a4a4e5616263");
    expect_round_trip("測試", "big5", "b4fab8d5");
    expect_round_trip("中文abc", "cp950", "a4a4a4e5616263");
    expect_round_trip("·×", "big5", "a150a1d1");

    expect_round_trip("Ê̄", "big5", "8862");
    expect_round_trip("ê̌", "big5", "88a5");
    expect_round_trip("Ê", "big5", "8866");
    expect_round_trip("ÊÊ", "big5", "88668866");
    expect_round_trip("Ê𠕇", "big5", "8866fa40");
    expect_round_trip("十", "big5", "a451");
    expect_round_trip("起", "big5", "b05f");
}
