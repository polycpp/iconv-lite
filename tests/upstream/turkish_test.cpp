#include "helpers.hpp"

#include <string>

namespace iconv = polycpp::iconv_lite;
using iconv_lite_upstream_test::expect_round_trip;

TEST(iconv_lite_upstream, turkish_test_vectors) {
    // Adapted from upstream test/turkish-test.js.
    const std::string turkish =
        "€‚ƒ„…†‡ˆ‰Š‹Œ‘’“”•–—˜™š›œŸ¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ"
        "ĞÑÒÓÔÕÖ×ØÙÚÛÜİŞßàáâãäåæçèéêëìíîïğñòóôõö÷øùúûüışÿ";
    expect_round_trip(turkish,
                      "windows1254",
                      "8082838485868788898a8b8c9192939495969798999a9b9c9fa1a2a3a4a5a6a7a8a9aaabacaeaf"
                      "b0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0"
                      "d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeef"
                      "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff");

    EXPECT_EQ(iconv::encode(std::string("\xC2\x81\xC2\x8D\xC2\x8E\xC2\x8F\xC2\x90\xC2\x9D\xC2\x9E", 14),
                            "windows1254")
                  .toString("latin1"),
              "???????");
}
