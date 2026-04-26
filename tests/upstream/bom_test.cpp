#include "helpers.hpp"

#include <string>

namespace iconv = polycpp::iconv_lite;
using iconv_lite_upstream_test::from_hex;

TEST(iconv_lite_upstream, bom_test_vectors) {
    // Adapted from upstream test/bom-test.js.
    constexpr auto sample = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<俄语>данные</俄语>";
    const auto utf8_bom = from_hex("efbbbf");
    const auto utf16be_bom = from_hex("feff");
    const auto utf16le_bom = from_hex("fffe");

    EXPECT_EQ(iconv::decode(iconv::Buffer::concat({utf8_bom, iconv::Buffer::from(sample)}), "utf8"), sample);

    const auto utf16le_body = iconv::Buffer::concat({utf16le_bom, iconv::encode(sample, "utf16le")});
    EXPECT_EQ(iconv::decode(utf16le_body, "utf16"), sample);
    EXPECT_EQ(iconv::decode(utf16le_body, "utf16le"), sample);

    const auto utf16be_body = iconv::Buffer::concat({utf16be_bom, iconv::encode(sample, "utf16be")});
    EXPECT_EQ(iconv::decode(utf16be_body, "utf16"), sample);
    EXPECT_EQ(iconv::decode(utf16be_body, "utf16be"), sample);

    iconv::DecodeOptions keep_bom;
    keep_bom.stripBOM = false;
    EXPECT_EQ(iconv::decode(iconv::Buffer::concat({utf8_bom, iconv::Buffer::from(sample)}), "utf8", keep_bom),
              std::string("\xEF\xBB\xBF", 3) + sample);
    EXPECT_EQ(iconv::decode(utf16le_body, "utf16", keep_bom), std::string("\xEF\xBB\xBF", 3) + sample);
    EXPECT_EQ(iconv::decode(utf16be_body, "utf16be", keep_bom), std::string("\xEF\xBB\xBF", 3) + sample);

    iconv::EncodeOptions addBOM;
    addBOM.addBOM = true;
    EXPECT_EQ(iconv::encode(sample, "utf8", addBOM).toString("hex"),
              iconv::Buffer::concat({utf8_bom, iconv::Buffer::from(sample)}).toString("hex"));
    EXPECT_EQ(iconv::encode(sample, "utf16le", addBOM).toString("hex"),
              iconv::Buffer::concat({utf16le_bom, iconv::encode(sample, "utf16le")}).toString("hex"));
    EXPECT_EQ(iconv::encode(sample, "utf16be", addBOM).toString("hex"),
              iconv::Buffer::concat({utf16be_bom, iconv::encode(sample, "utf16be")}).toString("hex"));

    const auto utf7_with_bom = iconv::encode(sample, "utf7", addBOM);
    EXPECT_NE(iconv::encode(sample, "utf7").toString("hex"), utf7_with_bom.toString("hex"));
    EXPECT_EQ(iconv::decode(utf7_with_bom, "utf7"), sample);

    iconv::EncodeOptions no_bom;
    no_bom.addBOM = false;
    EXPECT_EQ(iconv::encode(sample, "utf16", no_bom).toString("hex"),
              iconv::encode(sample, "utf16le").toString("hex"));
}

TEST(iconv_lite_upstream, bom_test_strip_callback) {
    // Adapted from upstream test/bom-test.js.
    constexpr auto sample = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<俄语>данные</俄语>";
    const auto utf8_bom = from_hex("efbbbf");
    const auto with_bom = iconv::Buffer::concat({utf8_bom, iconv::Buffer::from(sample)});

    bool bom_stripped = false;
    iconv::DecodeOptions options;
    options.onBOMStripped = [&] {
        bom_stripped = true;
    };

    EXPECT_EQ(iconv::decode(with_bom, "utf8", options), sample);
    EXPECT_TRUE(bom_stripped);

    bom_stripped = false;
    EXPECT_EQ(iconv::decode(iconv::Buffer::from(sample), "utf8", options), sample);
    EXPECT_FALSE(bom_stripped);

    bom_stripped = false;
    options.stripBOM = false;
    EXPECT_EQ(iconv::decode(with_bom, "utf8", options), std::string("\xEF\xBB\xBF", 3) + sample);
    EXPECT_FALSE(bom_stripped);

    int stateful_calls = 0;
    iconv::DecodeOptions stateful_options;
    stateful_options.onBOMStripped = [&] {
        ++stateful_calls;
    };
    auto decoder = iconv::getDecoder("utf8", stateful_options);
    EXPECT_EQ(decoder.write(iconv::Buffer::from({0xEF})), "");
    EXPECT_EQ(decoder.write(iconv::Buffer::from({0xBB, 0xBF, 0x41})), "A");
    EXPECT_EQ(decoder.end(), "");
    EXPECT_EQ(stateful_calls, 1);
}
