#include "helpers.hpp"

using iconv_lite_upstream_test::expect_encoded;
using iconv_lite_upstream_test::expect_round_trip;

TEST(iconv_lite_upstream, cesu8_test_vectors) {
    // Adapted from upstream test/cesu8-test.js.
    expect_round_trip("E", "cesu8", "45");
    expect_round_trip("¢", "cesu8", "c2a2");
    expect_round_trip("ȅ", "cesu8", "c885");
    expect_round_trip("€", "cesu8", "e282ac");
    expect_round_trip("𐐀", "cesu8", "eda081edb080");
    expect_round_trip("😱", "cesu8", "eda0bdedb8b1");
    expect_encoded("a😱a", "cesu8", "61eda0bdedb8b161");
    expect_encoded("😱😱", "cesu8", "eda0bdedb8b1eda0bdedb8b1");
}
