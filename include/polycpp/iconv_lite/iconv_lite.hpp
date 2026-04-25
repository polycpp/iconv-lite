#pragma once

/**
 * @file iconv_lite/iconv_lite.hpp
 * @brief Batch character encoding conversion for polycpp, modeled after npm iconv-lite.
 */

#include <optional>
#include <string>
#include <string_view>

#include <polycpp/buffer.hpp>
#include <polycpp/core/error.hpp>
#include <polycpp/iconv_lite/detail/aggregator.hpp>

/**
 * @namespace polycpp::iconv_lite
 * @brief Batch character encoding conversion APIs for polycpp.
 */
namespace polycpp::iconv_lite {

/**
 * @brief Byte buffer type used by this companion library.
 *
 * The port deliberately reuses the base polycpp Buffer instead of defining a
 * local byte container or porting npm safer-buffer.
 */
using Buffer = polycpp::buffer::Buffer;

/**
 * @brief Options for encode().
 */
struct EncodeOptions {
    /**
     * @brief Whether to prepend a BOM for BOM-aware encodings.
     *
     * When unset, `utf16` and `utf32` auto encoders add a BOM by default,
     * matching iconv-lite. Other BOM-aware encodings only add a BOM when this
     * is explicitly `true`.
     */
    std::optional<bool> add_bom;

    /**
     * @brief Default endianness for auto encoders.
     *
     * Currently used by `utf32`; accepted values include `utf32le` and
     * `utf32be`. Invalid values fall back to iconv-lite's default.
     */
    std::string default_encoding;
};

/**
 * @brief Options for decode().
 */
struct DecodeOptions {
    /**
     * @brief Strip an initial decoded U+FEFF from BOM-aware encodings.
     */
    bool strip_bom = true;

    /**
     * @brief Fallback endianness for UTF-16/UTF-32 auto detection.
     *
     * Accepted values include `utf16le`, `utf16be`, `utf32le`, and `utf32be`.
     * Invalid values fall back to iconv-lite's default little-endian behavior.
     */
    std::string default_encoding;
};

/**
 * @brief Resolved encoding metadata for diagnostics and tests.
 */
struct EncodingInfo {
    /** @brief Original label passed by the caller. */
    std::string requested;

    /** @brief iconv-lite-style canonical label. */
    std::string canonical;

    /** @brief ICU converter name or local internal codec name. */
    std::string converter;

    /** @brief True when BOM strip/prepend behavior applies. */
    bool bom_aware = false;

    /** @brief True when handled by a local internal codec instead of ICU. */
    bool internal = false;
};

/**
 * @brief Canonicalize an encoding label the same way iconv-lite does.
 *
 * The label is lowercased, a trailing `:YYYY` suffix is removed, and all
 * non-alphanumeric characters are stripped.
 */
std::string canonicalize_encoding(std::string_view encoding);

/**
 * @brief Return true when an encoding label can be resolved by this port.
 */
bool encoding_exists(std::string_view encoding) noexcept;

/**
 * @brief Resolve an encoding label to diagnostic metadata.
 * @throws polycpp::TypeError if the label is not supported.
 */
EncodingInfo inspect_encoding(std::string_view encoding);

/**
 * @brief Encode UTF-8 text into a byte buffer using the requested encoding.
 *
 * @param input UTF-8 text.
 * @param encoding iconv-lite-style encoding label.
 * @param options encode options.
 * @return Encoded bytes as polycpp::buffer::Buffer.
 * @throws polycpp::TypeError for unsupported labels or conversion failures.
 */
Buffer encode(std::string_view input,
              std::string_view encoding,
              const EncodeOptions& options = {});

/**
 * @brief Decode bytes in the requested encoding into UTF-8 text.
 *
 * @param input Encoded bytes.
 * @param encoding iconv-lite-style encoding label.
 * @param options decode options.
 * @return UTF-8 text.
 * @throws polycpp::TypeError for unsupported labels or conversion failures.
 */
std::string decode(const Buffer& input,
                   std::string_view encoding,
                   const DecodeOptions& options = {});

/**
 * @brief Legacy alias for encode(), matching iconv-lite `toEncoding`.
 */
inline Buffer to_encoding(std::string_view input,
                          std::string_view encoding,
                          const EncodeOptions& options = {}) {
    return encode(input, encoding, options);
}

/**
 * @brief Legacy alias for decode(), matching iconv-lite `fromEncoding`.
 */
inline std::string from_encoding(const Buffer& input,
                                 std::string_view encoding,
                                 const DecodeOptions& options = {}) {
    return decode(input, encoding, options);
}

}  // namespace polycpp::iconv_lite
