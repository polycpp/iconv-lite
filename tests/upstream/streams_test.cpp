#include "helpers.hpp"

#include <string>

#include <polycpp/core/error.hpp>

namespace iconv = polycpp::iconv_lite;

TEST(iconv_lite_upstream, streams_test_chunk_boundaries_and_collect) {
    // Adapted from upstream test/streams-test.js.
    auto encoder = iconv::encodeStream("us-ascii");
    encoder.write("hello ");
    encoder.write("world!");
    encoder.end();
    EXPECT_EQ(encoder.read().toString("latin1"), "hello world!");

    auto decoder = iconv::decodeStream("us-ascii");
    decoder.write(iconv::Buffer::from("hello "));
    decoder.write(iconv::Buffer::from("world!"));
    decoder.end();
    EXPECT_EQ(decoder.read().toString("utf8"), "hello world!");

    auto gbk = iconv::decodeStream("gbk");
    gbk.write(iconv::Buffer::from({0x61, 0x81}));
    gbk.write(iconv::Buffer::from({0x40, 0x61}));
    gbk.end();
    EXPECT_EQ(gbk.read().toString("utf8"), "a丂a");

    auto gbk_truncated = iconv::decodeStream("gbk");
    gbk_truncated.write(iconv::Buffer::from({0x61, 0x81}));
    gbk_truncated.end();
    EXPECT_EQ(gbk_truncated.read().toString("utf8"), "a�");

    auto utf16be = iconv::decodeStream("UTF-16BE");
    utf16be.write(iconv::Buffer::from({0x00, 0x61, 0x00}));
    utf16be.write(iconv::Buffer::from({0x62, 0x00, 0x63}));
    utf16be.end();
    EXPECT_EQ(utf16be.read().toString("utf8"), "abc");

    auto base64 = iconv::encodeStream("base64");
    base64.write("aGV");
    base64.write("sbG8gd2");
    base64.write("9ybGQ=");
    base64.end();
    EXPECT_EQ(base64.read().toString("latin1"), "hello world");

    auto utf7 = iconv::decodeStream("UTF-7");
    utf7.write(iconv::Buffer::from("+T2"));
    utf7.write(iconv::Buffer::from("BZf"));
    utf7.write(iconv::Buffer::from("Q hei+AN8-t"));
    utf7.end();
    EXPECT_EQ(utf7.read().toString("utf8"), "你好 heißt");

    auto utf7imap_encoder = iconv::encodeStream("UTF-7-IMAP");
    utf7imap_encoder.write("￿");
    utf7imap_encoder.write("");
    utf7imap_encoder.write("顶");
    utf7imap_encoder.write("吲");
    utf7imap_encoder.write("῭");
    utf7imap_encoder.end();
    EXPECT_EQ(utf7imap_encoder.read().toString("latin1"), "&,,,typh2VDIf7Q-");

    bool decode_collected = false;
    auto collected_decoder = iconv::decodeStream("gbk");
    collected_decoder.collect([&](polycpp::Error::Ptr err, std::string out) {
        EXPECT_FALSE(err);
        EXPECT_EQ(out, "a丂a");
        decode_collected = true;
    });
    collected_decoder.write(iconv::Buffer::from({0x61, 0x81}));
    collected_decoder.write(iconv::Buffer::from({0x40, 0x61}));
    collected_decoder.end();
    EXPECT_TRUE(decode_collected);

    bool encode_collected = false;
    auto collected_encoder = iconv::encodeStream("windows-1251");
    collected_encoder.collect([&](polycpp::Error::Ptr err, iconv::Buffer out) {
        EXPECT_FALSE(err);
        EXPECT_EQ(out.toString("hex"), "e0e1e2e3e4e5");
        encode_collected = true;
    });
    collected_encoder.write("абв");
    collected_encoder.write("где");
    collected_encoder.end();
    EXPECT_TRUE(encode_collected);
}
