#pragma once

/**
 * @file iconv_lite/iconv_lite.hpp
 * @brief Batch character encoding conversion for polycpp, modeled after npm iconv-lite.
 */

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <polycpp/buffer.hpp>
#include <polycpp/core/error.hpp>
#include <polycpp/iconv_lite/detail/aggregator.hpp>
#include <polycpp/stream.hpp>

/**
 * @namespace polycpp::iconv_lite
 * @brief Batch character encoding conversion APIs for polycpp.
 */
namespace polycpp::iconv_lite {

namespace detail {
class EncoderState;
class DecoderState;
class EncodeStreamImpl;
class DecodeStreamImpl;
}  // namespace detail

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
     * @brief Optional observer called when an initial BOM is actually stripped.
     *
     * This models iconv-lite's `stripBOM` callback form while keeping the C++
     * option that controls stripping as an explicit boolean.
     */
    std::function<void()> on_bom_stripped;

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

    /** @brief Resolved upstream table name or local internal codec name. */
    std::string converter;

    /** @brief True when BOM strip/prepend behavior applies. */
    bool bom_aware = false;

    /** @brief True when handled by a local internal codec instead of generated SBCS/DBCS tables. */
    bool internal = false;
};

class Encoder;
class Decoder;

/**
 * @brief Resolved codec descriptor returned by get_codec().
 *
 * JavaScript iconv-lite exposes codec constructor objects through `getCodec`.
 * This C++ companion exposes deterministic metadata plus factories for the
 * stateful encoder and decoder objects that carry stream chunk-boundary state.
 */
struct Codec {
    EncodingInfo info;

    /** @brief True when BOM strip/prepend wrappers apply. */
    bool bom_aware() const noexcept { return info.bom_aware; }

    /** @brief True when this is a local internal codec instead of generated table data. */
    bool internal() const noexcept { return info.internal; }

    /** @brief Create a stateful encoder for this codec. */
    Encoder encoder(const EncodeOptions& options = {}) const;

    /** @brief Create a stateful decoder for this codec. */
    Decoder decoder(const DecodeOptions& options = {}) const;
};

/**
 * @brief Stateful low-level encoder, matching iconv-lite `getEncoder()`.
 *
 * `write()` may be called multiple times. `end()` flushes any pending bytes
 * for encodings whose upstream encoders keep chunk-boundary state, such as
 * base64, UTF-7-IMAP, GB18030/DBCS sequence state, and BOM prepending.
 */
class Encoder {
public:
    Encoder(std::string_view encoding, const EncodeOptions& options = {});
    Encoder(const Codec& codec, const EncodeOptions& options = {});
    Encoder(const Encoder&) = default;
    Encoder(Encoder&&) noexcept = default;
    Encoder& operator=(const Encoder&) = default;
    Encoder& operator=(Encoder&&) noexcept = default;
    ~Encoder();

    /** @brief Encode the next UTF-8 text chunk. */
    Buffer write(std::string_view input);

    /** @brief Flush pending encoder state. */
    Buffer end();

    /** @brief Resolved encoding metadata for this encoder. */
    EncodingInfo info() const;

private:
    explicit Encoder(std::shared_ptr<detail::EncoderState> state);

    std::shared_ptr<detail::EncoderState> state_;

    friend struct Codec;
};

/**
 * @brief Stateful low-level decoder, matching iconv-lite `getDecoder()`.
 *
 * `write()` may be called multiple times. `end()` flushes pending incomplete
 * input according to iconv-lite behavior for DBCS, UTF-7, UTF-16, UTF-32, and
 * BOM stripping.
 */
class Decoder {
public:
    Decoder(std::string_view encoding, const DecodeOptions& options = {});
    Decoder(const Codec& codec, const DecodeOptions& options = {});
    Decoder(const Decoder&) = default;
    Decoder(Decoder&&) noexcept = default;
    Decoder& operator=(const Decoder&) = default;
    Decoder& operator=(Decoder&&) noexcept = default;
    ~Decoder();

    /** @brief Decode the next byte chunk into UTF-8 text. */
    std::string write(const Buffer& input);

    /** @brief Flush pending decoder state. */
    std::string end();

    /** @brief Resolved encoding metadata for this decoder. */
    EncodingInfo info() const;

private:
    explicit Decoder(std::shared_ptr<detail::DecoderState> state);

    std::shared_ptr<detail::DecoderState> state_;

    friend struct Codec;
};

/**
 * @brief Stream construction options for iconv-lite transform streams.
 */
struct IconvStreamOptions {
    /** @brief Event context to bind the stream to; null uses polycpp's default event loop. */
    EventContext* context = nullptr;

    /** @brief Read/write high-water mark in bytes. */
    size_t high_water_mark = stream::kDefaultHighWaterMark;

    /** @brief Whether the stream emits close on destroy. */
    bool emit_close = true;

    /** @brief Whether the stream auto-destroys after completion. */
    bool auto_destroy = true;
};

/**
 * @brief Transform stream that encodes UTF-8 text chunks into encoded bytes.
 */
class EncodeStream : public stream::Transform {
public:
    EncodeStream(std::string_view encoding,
                 const EncodeOptions& options = {},
                 const IconvStreamOptions& stream_options = {});
    EncodeStream(Encoder encoder,
                 const IconvStreamOptions& stream_options = {});
    explicit EncodeStream(std::shared_ptr<detail::EncodeStreamImpl> impl);
    ~EncodeStream() override;

    /**
     * @brief Collect emitted byte chunks and call the callback at end/error.
     *
     * The callback receives a non-null error on failure, otherwise the
     * concatenated output buffer.
     */
    EncodeStream& collect(std::function<void(Error::Ptr, Buffer)> callback);
};

/**
 * @brief Transform stream that decodes encoded byte chunks into UTF-8 text.
 *
 * The readable side emits UTF-8 buffers because polycpp streams are byte
 * oriented; `collect()` exposes the decoded text directly.
 */
class DecodeStream : public stream::Transform {
public:
    DecodeStream(std::string_view encoding,
                 const DecodeOptions& options = {},
                 const IconvStreamOptions& stream_options = {});
    DecodeStream(Decoder decoder,
                 const IconvStreamOptions& stream_options = {});
    explicit DecodeStream(std::shared_ptr<detail::DecodeStreamImpl> impl);
    ~DecodeStream() override;

    /**
     * @brief Collect emitted UTF-8 text chunks and call the callback at end/error.
     */
    DecodeStream& collect(std::function<void(Error::Ptr, std::string)> callback);
};

using IconvLiteEncoderStream = EncodeStream;
using IconvLiteDecoderStream = DecodeStream;

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
 * @brief Return the resolved codec descriptor for an encoding label.
 * @throws polycpp::TypeError if the label is not supported.
 */
Codec get_codec(std::string_view encoding);

/**
 * @brief Create a stateful low-level encoder for an encoding label.
 */
Encoder get_encoder(std::string_view encoding, const EncodeOptions& options = {});

/**
 * @brief Create a stateful low-level decoder for an encoding label.
 */
Decoder get_decoder(std::string_view encoding, const DecodeOptions& options = {});

/**
 * @brief Current Unicode replacement string used by table decoders.
 */
std::string default_char_unicode();

/**
 * @brief Set the Unicode replacement string used by future table decoders.
 */
void set_default_char_unicode(std::string value);

/**
 * @brief Current single-byte replacement string used by table encoders.
 */
std::string default_char_single_byte();

/**
 * @brief Set the single-byte replacement string used by future table encoders.
 */
void set_default_char_single_byte(std::string value);

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
 * @brief Streaming support is always available in this C++ companion.
 */
bool supports_streams() noexcept;

/**
 * @brief No-op compatibility hook matching iconv-lite enableStreamingAPI().
 */
void enable_streaming_api();

/**
 * @brief Create a transform stream that encodes UTF-8 text chunks.
 */
EncodeStream encode_stream(std::string_view encoding,
                           const EncodeOptions& options = {},
                           const IconvStreamOptions& stream_options = {});

/**
 * @brief Create a transform stream that decodes byte chunks into UTF-8 text.
 */
DecodeStream decode_stream(std::string_view encoding,
                           const DecodeOptions& options = {},
                           const IconvStreamOptions& stream_options = {});

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

/** @brief JavaScript-name alias for encoding_exists(). */
inline bool encodingExists(std::string_view encoding) noexcept {
    return encoding_exists(encoding);
}

/** @brief JavaScript-name alias for to_encoding(). */
inline Buffer toEncoding(std::string_view input,
                         std::string_view encoding,
                         const EncodeOptions& options = {}) {
    return to_encoding(input, encoding, options);
}

/** @brief JavaScript-name alias for from_encoding(). */
inline std::string fromEncoding(const Buffer& input,
                                std::string_view encoding,
                                const DecodeOptions& options = {}) {
    return from_encoding(input, encoding, options);
}

/** @brief JavaScript-name alias for canonicalize_encoding(). */
inline std::string _canonicalizeEncoding(std::string_view encoding) {
    return canonicalize_encoding(encoding);
}

/** @brief JavaScript-name alias for get_codec(). */
inline Codec getCodec(std::string_view encoding) {
    return get_codec(encoding);
}

/** @brief JavaScript-name alias for get_encoder(). */
inline Encoder getEncoder(std::string_view encoding, const EncodeOptions& options = {}) {
    return get_encoder(encoding, options);
}

/** @brief JavaScript-name alias for get_decoder(). */
inline Decoder getDecoder(std::string_view encoding, const DecodeOptions& options = {}) {
    return get_decoder(encoding, options);
}

/** @brief JavaScript-name alias for supports_streams(). */
inline bool supportsStreams() noexcept {
    return supports_streams();
}

/** @brief JavaScript-name alias for enable_streaming_api(). */
inline void enableStreamingAPI() {
    enable_streaming_api();
}

/** @brief JavaScript-name alias for encode_stream(). */
inline EncodeStream encodeStream(std::string_view encoding,
                                 const EncodeOptions& options = {},
                                 const IconvStreamOptions& stream_options = {}) {
    return encode_stream(encoding, options, stream_options);
}

/** @brief JavaScript-name alias for decode_stream(). */
inline DecodeStream decodeStream(std::string_view encoding,
                                 const DecodeOptions& options = {},
                                 const IconvStreamOptions& stream_options = {}) {
    return decode_stream(encoding, options, stream_options);
}

}  // namespace polycpp::iconv_lite
