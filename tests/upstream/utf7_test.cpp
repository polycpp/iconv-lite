#include "helpers.hpp"

namespace iconv = polycpp::iconv_lite;

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
