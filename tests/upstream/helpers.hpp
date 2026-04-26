#pragma once

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <polycpp/iconv_lite/iconv_lite.hpp>

namespace iconv_lite_upstream_test {
namespace iconv = polycpp::iconv_lite;

inline uint8_t hex_value(char c) {
    if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(10 + c - 'a');
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(10 + c - 'A');
    throw std::runtime_error("invalid hex digit");
}

inline iconv::Buffer from_hex(std::string_view hex) {
    if (hex.size() % 2 != 0) throw std::runtime_error("hex input must have even length");
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        bytes.push_back(static_cast<uint8_t>((hex_value(hex[i]) << 4) | hex_value(hex[i + 1])));
    }
    return iconv::Buffer::from(bytes);
}

inline void expect_encoded(std::string_view text, std::string_view encoding, std::string_view expected_hex) {
    EXPECT_EQ(iconv::encode(text, encoding).toString("hex"), expected_hex) << encoding;
}

inline void expect_decoded(std::string_view input_hex, std::string_view encoding, std::string_view expected_text) {
    EXPECT_EQ(iconv::decode(from_hex(input_hex), encoding), expected_text) << encoding << " " << input_hex;
}

inline void expect_round_trip(std::string_view text, std::string_view encoding, std::string_view expected_hex) {
    expect_encoded(text, encoding, expected_hex);
    expect_decoded(expected_hex, encoding, text);
}

}  // namespace iconv_lite_upstream_test
