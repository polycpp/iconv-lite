#include "helpers.hpp"

namespace iconv = polycpp::iconv_lite;
using iconv_lite_upstream_test::expect_round_trip;

TEST(iconv_lite_upstream, greek_test_vectors) {
    // Adapted from upstream test/greek-test.js.
    expect_round_trip("Γειά!", "windows1253", "c3e5e9dc21");
    expect_round_trip("Γειά!", "iso88597", "c3e5e9dc21");
    expect_round_trip("Γειά!", "cp737", "829ca0e121");

    EXPECT_EQ(iconv::encode("Åçþÿ¿", "windows1253").toString("latin1"), "?????");
}
