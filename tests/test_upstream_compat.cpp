#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <polycpp/core/error.hpp>
#include <polycpp/iconv_lite/iconv_lite.hpp>

namespace iconv = polycpp::iconv_lite;

namespace {

uint8_t hex_value(char c) {
    if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(10 + c - 'a');
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(10 + c - 'A');
    throw std::runtime_error("invalid hex digit");
}

iconv::Buffer from_hex(std::string_view hex) {
    if (hex.size() % 2 != 0) throw std::runtime_error("hex input must have even length");
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        bytes.push_back(static_cast<uint8_t>((hex_value(hex[i]) << 4) | hex_value(hex[i + 1])));
    }
    return iconv::Buffer::from(bytes);
}

void expect_encoded(std::string_view text, std::string_view encoding, std::string_view expected_hex) {
    EXPECT_EQ(iconv::encode(text, encoding).toString("hex"), expected_hex) << encoding;
}

void expect_decoded(std::string_view input_hex, std::string_view encoding, std::string_view expected_text) {
    EXPECT_EQ(iconv::decode(from_hex(input_hex), encoding), expected_text) << encoding << " " << input_hex;
}

void expect_round_trip(std::string_view text, std::string_view encoding, std::string_view expected_hex) {
    expect_encoded(text, encoding, expected_hex);
    expect_decoded(expected_hex, encoding, text);
}

}  // namespace

TEST(iconv_lite_upstream, main_test_core_api_and_aliases) {
    // Adapted from upstream test/main-test.js.
    constexpr auto test_string = "Hello123!";
    constexpr auto latin1_string = "Hello123!£Å÷×çþÿ¿®";

    EXPECT_FALSE(iconv::encodingExists("__proto__"));
    EXPECT_FALSE(iconv::encodingExists("constructor"));
    EXPECT_TRUE(iconv::encodingExists("utf8"));

    EXPECT_EQ(iconv::encode(test_string, "utf8").toString("hex"), "48656c6c6f31323321");
    EXPECT_EQ(iconv::decode(iconv::Buffer::from(test_string), "utf8"), test_string);
    EXPECT_EQ(iconv::encode("SGVsbG8xMjMh", "base64").toString("latin1"), test_string);
    EXPECT_EQ(iconv::decode(iconv::Buffer::from(test_string), "base64"), "SGVsbG8xMjMh");
    EXPECT_EQ(iconv::encode("48656c6c6f31323321", "hex").toString("latin1"), test_string);
    EXPECT_EQ(iconv::decode(iconv::Buffer::from(test_string), "hex"), "48656c6c6f31323321");
    EXPECT_EQ(iconv::encode(latin1_string, "latin1").toString("hex"),
              "48656c6c6f31323321a3c5f7d7e7feffbfae");
    EXPECT_EQ(iconv::toEncoding(test_string, "latin1").toString("hex"),
              iconv::encode(test_string, "latin1").toString("hex"));
    EXPECT_EQ(iconv::fromEncoding(iconv::Buffer::from(latin1_string), "latin1"),
              iconv::decode(iconv::Buffer::from(latin1_string), "latin1"));
    EXPECT_THROW((void)iconv::encode("a", "xxx"), polycpp::TypeError);
    EXPECT_THROW((void)iconv::decode(iconv::Buffer::from("a"), "xxx"), polycpp::TypeError);
    EXPECT_EQ(iconv::encode("外国人", "latin1").toString("latin1"), "???");
    EXPECT_EQ(iconv::_canonicalizeEncoding("ISO_8859-5:1988"), "iso88595");
}

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
    keep_bom.strip_bom = false;
    EXPECT_EQ(iconv::decode(iconv::Buffer::concat({utf8_bom, iconv::Buffer::from(sample)}), "utf8", keep_bom),
              std::string("\xEF\xBB\xBF", 3) + sample);
    EXPECT_EQ(iconv::decode(utf16le_body, "utf16", keep_bom), std::string("\xEF\xBB\xBF", 3) + sample);
    EXPECT_EQ(iconv::decode(utf16be_body, "utf16be", keep_bom), std::string("\xEF\xBB\xBF", 3) + sample);

    iconv::EncodeOptions add_bom;
    add_bom.add_bom = true;
    EXPECT_EQ(iconv::encode(sample, "utf8", add_bom).toString("hex"),
              iconv::Buffer::concat({utf8_bom, iconv::Buffer::from(sample)}).toString("hex"));
    EXPECT_EQ(iconv::encode(sample, "utf16le", add_bom).toString("hex"),
              iconv::Buffer::concat({utf16le_bom, iconv::encode(sample, "utf16le")}).toString("hex"));
    EXPECT_EQ(iconv::encode(sample, "utf16be", add_bom).toString("hex"),
              iconv::Buffer::concat({utf16be_bom, iconv::encode(sample, "utf16be")}).toString("hex"));

    const auto utf7_with_bom = iconv::encode(sample, "utf7", add_bom);
    EXPECT_NE(iconv::encode(sample, "utf7").toString("hex"), utf7_with_bom.toString("hex"));
    EXPECT_EQ(iconv::decode(utf7_with_bom, "utf7"), sample);

    iconv::EncodeOptions no_bom;
    no_bom.add_bom = false;
    EXPECT_EQ(iconv::encode(sample, "utf16", no_bom).toString("hex"),
              iconv::encode(sample, "utf16le").toString("hex"));
}

TEST(iconv_lite_upstream, utf16_test_vectors) {
    // Adapted from upstream test/utf16-test.js.
    constexpr auto test = "1aя中文☃💩";
    constexpr auto sample = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<俄语>данные</俄语>";
    constexpr auto utf16be_hex = "00310061044f4e2d65872603d83ddca9";
    constexpr auto utf16le_hex = "310061004f042d4e876503263dd8a9dc";

    expect_round_trip(test, "UTF16-BE", utf16be_hex);
    EXPECT_EQ(iconv::decode(from_hex("006100"), "UTF16-BE"), "a");

    EXPECT_EQ(iconv::encode(test, "utf-16").toString("hex"), std::string("fffe") + utf16le_hex);
    iconv::EncodeOptions use_utf16le;
    use_utf16le.default_encoding = "UTF-16LE";
    EXPECT_EQ(iconv::encode(test, "utf-16", use_utf16le).toString("hex"), std::string("fffe") + utf16le_hex);

    EXPECT_EQ(iconv::decode(from_hex(std::string("fffe") + utf16le_hex), "utf-16"), test);
    EXPECT_EQ(iconv::decode(from_hex(std::string("feff") + utf16be_hex), "utf-16"), test);
    EXPECT_EQ(iconv::decode(iconv::Buffer::from({0x61}), "utf-16"), "");
    EXPECT_EQ(iconv::decode(iconv::encode(sample, "utf-16le"), "utf-16"), sample);
    EXPECT_EQ(iconv::decode(iconv::encode(sample, "utf-16be"), "utf-16"), sample);
    EXPECT_EQ(iconv::decode(from_hex(utf16le_hex), "utf-16"), test);

    iconv::DecodeOptions default_utf16le;
    default_utf16le.default_encoding = "utf-16le";
    EXPECT_EQ(iconv::decode(from_hex(utf16le_hex), "utf-16", default_utf16le), test);
}

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
    no_bom_be.add_bom = false;
    no_bom_be.default_encoding = "ucs4be";
    EXPECT_EQ(iconv::encode(test, "ucs4", no_bom_be).toString("hex"), utf32be_hex);

    EXPECT_EQ(iconv::decode(from_hex(std::string("fffe0000") + utf32le_hex), "utf-32"), test);
    EXPECT_EQ(iconv::decode(iconv::encode(sample, "utf-32-le"), "utf-32"), sample);
    EXPECT_EQ(iconv::decode(iconv::encode(sample, "utf-32-be"), "utf-32"), sample);

    iconv::DecodeOptions keep_bom;
    keep_bom.strip_bom = false;
    EXPECT_EQ(iconv::decode(from_hex(std::string("0000feff") + utf32be_hex), "utf-32", keep_bom),
              std::string("\xEF\xBB\xBF", 3) + test);
}

TEST(iconv_lite_upstream, utf7_test_vectors) {
    // Adapted from upstream test/utf7-test.js.
    struct Fixture {
        const char* text;
        const char* encoding;
        const char* encoded;
    };
    const Fixture encode_fixtures[] = {
        {"A≢Α.", "utf-7", "A+ImIDkQ-."},
        {"日本語", "utf-7", "+ZeVnLIqe-"},
        {"Hi Mom -☺-!", "utf-7", "Hi Mom -+Jjo--+ACE-"},
        {"Item 3 is £1.", "utf-7", "Item 3 is +AKM-1."},
        {"Jyväskylä", "utf-7", "Jyv+AOQ-skyl+AOQ-"},
        {"'你好' heißt \"Hallo\"", "utf-7", "'+T2BZfQ-' hei+AN8-t +ACI-Hallo+ACI-"},
        {"Hot + Spicy + Fruity", "utf-7", "Hot +- Spicy +- Fruity"},
        {"￿顶吲῭", "utf-7", "+///typh2VDIf7Q-"},
        {"ä+ä+ä", "utf-7", "+AOQAKwDkACsA5A-"},
        {"A≢Α.", "utf-7-imap", "A&ImIDkQ-."},
        {"日本語", "utf-7-imap", "&ZeVnLIqe-"},
        {"Hi Mom -☺-!", "utf-7-imap", "Hi Mom -&Jjo--!"},
        {"Item 3 is £1.", "utf-7-imap", "Item 3 is &AKM-1."},
        {"Jyväskylä", "utf-7-imap", "Jyv&AOQ-skyl&AOQ-"},
        {"'你好' heißt \"Hallo\"", "utf-7-imap", "'&T2BZfQ-' hei&AN8-t \"Hallo\""},
        {"Hot & Spicy & Fruity", "utf-7-imap", "Hot &- Spicy &- Fruity"},
        {"￿顶吲῭", "utf-7-imap", "&,,,typh2VDIf7Q-"},
        {"ä&ä&ä", "utf-7-imap", "&AOQ-&-&AOQ-&-&AOQ-"},
    };

    for (const auto& fixture : encode_fixtures) {
        EXPECT_EQ(iconv::encode(fixture.text, fixture.encoding).toString("latin1"), fixture.encoded)
            << fixture.encoding << " " << fixture.text;
        EXPECT_EQ(iconv::decode(iconv::Buffer::from(fixture.encoded), fixture.encoding), fixture.text)
            << fixture.encoding << " " << fixture.encoded;
    }

    EXPECT_EQ(iconv::decode(iconv::Buffer::from("A+ImIDkQ."), "utf-7"), "A≢Α.");
    EXPECT_EQ(iconv::decode(iconv::Buffer::from("+ZeVnLIqe"), "utf-7"), "日本語");
    EXPECT_EQ(iconv::decode(iconv::Buffer::from("Hi+ACA-Mom+ACA--+Jjo--+ACE-"), "utf-7"), "Hi Mom -☺-!");
    EXPECT_EQ(iconv::decode(iconv::Buffer::from("Item+ACA-3+ACA-is+ACAAow-1."), "utf-7"), "Item 3 is £1.");
    EXPECT_EQ(iconv::decode(iconv::Buffer::from("x+--"), "utf-7"), "x+-");
    EXPECT_EQ(iconv::decode(iconv::Buffer::from("x+-y"), "utf-7"), "x+y");
    EXPECT_EQ(iconv::decode(iconv::Buffer::from("+DEE?"), "utf-7"), "ు?");
    EXPECT_EQ(iconv::decode(iconv::Buffer::from("+2ADcAA?"), "utf-7"), "𐀀?");
    EXPECT_EQ(iconv::decode(iconv::Buffer::from("+AMAA4A-Next"), "utf-7"), "ÀàNext");
}

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

TEST(iconv_lite_upstream, regional_sbcs_test_vectors) {
    // Adapted from upstream test/cyrillic-test.js, test/greek-test.js, and test/turkish-test.js.
    expect_round_trip("Привет!", "win1251", "cff0e8e2e5f221");
    expect_round_trip("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюя",
                      "win1251",
                      "c0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
                      "e0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff");
    expect_round_trip("Привет!", "koi8r", "f0d2c9d7c5d421");
    expect_round_trip("Привет!", "iso88595", "bfe0d8d2d5e221");
    expect_round_trip("Γειά!", "windows1253", "c3e5e9dc21");
    expect_round_trip("Γειά!", "iso88597", "c3e5e9dc21");
    expect_round_trip("Γειά!", "cp737", "829ca0e121");

    const std::string turkish =
        "€‚ƒ„…†‡ˆ‰Š‹Œ‘’“”•–—˜™š›œŸ¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ"
        "ĞÑÒÓÔÕÖ×ØÙÚÛÜİŞßàáâãäåæçèéêëìíîïğñòóôõö÷øùúûüışÿ";
    expect_round_trip(turkish,
                      "windows1254",
                      "8082838485868788898a8b8c9192939495969798999a9b9c9fa1a2a3a4a5a6a7a8a9aaabacaeaf"
                      "b0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0"
                      "d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeef"
                      "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff");

    EXPECT_EQ(iconv::encode("£Åçþÿ¿", "win1251").toString("latin1"), "??????");
    EXPECT_EQ(iconv::encode("Åçþÿ¿", "windows1253").toString("latin1"), "?????");
    EXPECT_EQ(iconv::encode(std::string("\xC2\x81\xC2\x8D\xC2\x8E\xC2\x8F\xC2\x90\xC2\x9D\xC2\x9E", 14),
                            "windows1254")
                  .toString("latin1"),
              "???????");

    const char* aliases[] = {"Windows-1251", "windows1251", "CP1251", "1251"};
    for (const auto* alias : aliases) {
        EXPECT_EQ(iconv::decode(from_hex("cff0e8e2e5f221"), alias), "Привет!") << alias;
        EXPECT_EQ(iconv::encode("Привет!", alias).toString("hex"), "cff0e8e2e5f221") << alias;
    }
}

TEST(iconv_lite_upstream, dbcs_test_vectors) {
    // Adapted from upstream test/big5-test.js, test/gbk-test.js, and test/shiftjis-test.js.
    expect_round_trip("中国abc", "GBK", "d6d0b9fa616263");
    expect_round_trip("中国abc", "GB2312", "d6d0b9fa616263");
    expect_round_trip("中文abc", "big5", "a4a4a4e5616263");
    expect_round_trip("測試", "big5", "b4fab8d5");
    expect_round_trip("中文abc", "cp950", "a4a4a4e5616263");
    expect_round_trip("中文abc", "shiftjis", "928695b6616263");
    expect_round_trip("測試", "shiftjis", "91aa8e8e");
    expect_round_trip("·×", "big5", "a150a1d1");
    expect_round_trip("·×", "GBK", "a1a4a1c1");

    expect_round_trip("Ê̄", "big5", "8862");
    expect_round_trip("ê̌", "big5", "88a5");
    expect_round_trip("Ê", "big5", "8866");
    expect_round_trip("ÊÊ", "big5", "88668866");
    expect_round_trip("Ê𠕇", "big5", "8866fa40");
    expect_round_trip("十", "big5", "a451");
    expect_round_trip("起", "big5", "b05f");

    EXPECT_EQ(iconv::decode(from_hex("ed40eefceeef"), "shiftjis"), "纊＂ⅰ");
    EXPECT_EQ(iconv::decode(from_hex("f040f2fcf940"), "shiftjis"), "");
    EXPECT_EQ(iconv::encode("纊＂ⅰ", "shiftjis").toString("hex"), "fa5cfa57fa40");
    EXPECT_EQ(iconv::encode("", "shiftjis").toString("hex"), "3f3f3f");
    expect_round_trip("①", "shiftjis", "8740");
}

TEST(iconv_lite_upstream, gb18030_four_byte_and_incomplete_vectors) {
    // Adapted from upstream test/gbk-test.js.
    struct FourByteFixture {
        const char* text;
        const char* hex;
    };
    const FourByteFixture fixtures[] = {
        {"\u0080", "81308130"},
        {"\u0081", "81308131"},
        {"\u008B", "81308231"},
        {"ؕ", "81318231"},
        {"㦟", "82318231"},
        {"\xF4\x86\x99\xB7", "e0318231"},
    };

    for (const auto& fixture : fixtures) {
        expect_round_trip(fixture.text, "GB18030", fixture.hex);
    }

    struct DecodeFixture {
        const char* text;
        const char* hex;
    };
    const DecodeFixture incomplete[] = {
        {"�", "82"},
        {"�1", "8231"},
        {"�1�", "823182"},
        {"㦟", "82318231"},
        {"� ", "8220"},
        {"�1 ", "823120"},
        {"�1� ", "82318220"},
        {"㦟 ", "8231823120"},
        {"�1俛", "82318261"},
        {"�1倐a", "8231828261"},
        {"㦟俛", "823182318261"},
        {"�1倐1�1", "82318282318231"},
    };

    for (const auto& fixture : incomplete) {
        expect_decoded(fixture.hex, "GB18030", fixture.text);
    }

    EXPECT_EQ(iconv::decode(from_hex("a8bc008135f437"), "GB18030"), std::string("ḿ") + '\0' + "");
    EXPECT_EQ(iconv::encode(std::string("ḿ") + '\0' + "", "GB18030").toString("hex"), "a8bc008135f437");
    EXPECT_EQ(iconv::decode(from_hex("80"), "GBK"), "€");
    EXPECT_EQ(iconv::decode(from_hex("a2e3"), "GBK"), "€");
    EXPECT_EQ(iconv::encode("€", "GBK").toString("hex"), "80");
    EXPECT_EQ(iconv::encode("€", "GB18030").toString("hex"), "a2e3");
}

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
