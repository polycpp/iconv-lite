#include "helpers.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <polycpp/iconv_lite/detail/generated_tables.hpp>

namespace iconv = polycpp::iconv_lite;
namespace generated = polycpp::iconv_lite::generated;

namespace {

std::string utf8_from_u16(uint16_t unit) {
    if (unit < 0x80) return std::string(1, static_cast<char>(unit));
    if (unit < 0x800) {
        return std::string({
            static_cast<char>(0xC0 | (unit >> 6)),
            static_cast<char>(0x80 | (unit & 0x3F)),
        });
    }
    return std::string({
        static_cast<char>(0xE0 | (unit >> 12)),
        static_cast<char>(0x80 | ((unit >> 6) & 0x3F)),
        static_cast<char>(0x80 | (unit & 0x3F)),
    });
}

uint16_t expected_sbcs_unit(const generated::SbcsSpec& spec, uint8_t byte) {
    const auto offset = static_cast<size_t>(spec.chars.offset);
    const auto length = static_cast<size_t>(spec.chars.length);

    if (length == 128) {
        if (byte < 128) return byte;
        return generated::U16_DATA[offset + byte - 128];
    }

    if (byte < length) return generated::U16_DATA[offset + byte];
    return 0xFFFD;
}

}  // namespace

TEST(iconv_lite_upstream, sbcs_generated_tables_decode_all_bytes) {
    // Adapted from upstream test/sbcs-test.js without requiring native iconv:
    // every generated SBCS table is checked against the table data bundled into
    // this companion, including ASCII-preserving and full 256-entry tables.
    for (const auto& spec : generated::SBCS_SPECS) {
        std::vector<uint8_t> bytes;
        bytes.reserve(256);
        std::string expected;
        for (int value = 0; value <= 0xFF; ++value) {
            const auto byte = static_cast<uint8_t>(value);
            bytes.push_back(byte);
            expected += utf8_from_u16(expected_sbcs_unit(spec, byte));
        }

        EXPECT_EQ(iconv::decode(iconv::Buffer::from(bytes), spec.name), expected) << spec.name;
    }
}

TEST(iconv_lite_upstream, sbcs_generated_tables_encode_representable_units) {
    for (const auto& spec : generated::SBCS_SPECS) {
        std::unordered_map<uint16_t, uint8_t> last_byte_for_unit;
        for (int value = 0; value <= 0xFF; ++value) {
            const auto byte = static_cast<uint8_t>(value);
            const auto unit = expected_sbcs_unit(spec, byte);
            if (unit == 0xFFFD) continue;
            last_byte_for_unit[unit] = byte;
        }

        for (const auto& [unit, byte] : last_byte_for_unit) {
            const auto encoded = iconv::encode(utf8_from_u16(unit), spec.name);
            ASSERT_EQ(encoded.length(), 1u) << spec.name;
            EXPECT_EQ(encoded[0], byte) << spec.name << " U+" << std::hex << unit;
        }
    }
}

