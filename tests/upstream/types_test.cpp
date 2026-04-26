#include <array>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include <polycpp/iconv_lite/detail/generated_tables.hpp>
#include <polycpp/iconv_lite/iconv_lite.hpp>

namespace {
namespace iconv = polycpp::iconv_lite;

// iconv-lite 0.7.2 types/encodings.d.ts declares 412 literal labels. Most are
// generated table labels; these remaining labels are implemented as local
// internal codecs in the C++ port.
constexpr std::array<std::string_view, 13> kInternalTypedLabels = {
    "base64",
    "binary",
    "cesu8",
    "hex",
    "ucs2",
    "utf16",
    "utf16be",
    "utf32",
    "utf32be",
    "utf32le",
    "utf7",
    "utf7imap",
    "utf8",
};

}  // namespace

TEST(iconv_lite_upstream, types_encoding_union_labels_are_supported) {
    size_t checked = 0;

    for (const auto& entry : iconv::generated::ENCODING_ENTRIES) {
        EXPECT_TRUE(iconv::encodingExists(entry.name)) << std::string(entry.name);
        ++checked;
    }

    for (std::string_view label : kInternalTypedLabels) {
        EXPECT_TRUE(iconv::encodingExists(label)) << label;
        ++checked;
    }

    EXPECT_EQ(checked, 412u);
    EXPECT_FALSE(iconv::encodingExists("not_in_iconv_lite_types"));
}
