#pragma once

/**
 * @file iconv_lite/iconv_lite.hpp
 * @brief Batch character encoding conversion for polycpp, modeled after npm iconv-lite.
 * @since 1.0.0
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
 * @since 1.0.0
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
 *
 * @since 1.0.0
 */
using Buffer = polycpp::buffer::Buffer;

/**
 * @brief Options for encode().
 * @since 1.0.0
 */
struct EncodeOptions {
    /**
     * @brief Whether to prepend a BOM for BOM-aware encodings.
     *
     * When unset, `utf16` and `utf32` auto encoders add a BOM by default,
     * matching iconv-lite. Other BOM-aware encodings only add a BOM when this
     * is explicitly `true`.
     *
     * @since 1.0.0
     */
    std::optional<bool> addBOM;

    /**
     * @brief Default endianness for auto encoders.
     *
     * Currently used by `utf32`; accepted values include `utf32le` and
     * `utf32be`. Invalid values fall back to iconv-lite's default.
     *
     * @since 1.0.0
     */
    std::string defaultEncoding;
};

/**
 * @brief Options for decode().
 * @since 1.0.0
 */
struct DecodeOptions {
    /**
     * @brief Strip an initial decoded U+FEFF from BOM-aware encodings.
     * @since 1.0.0
     */
    bool stripBOM = true;

    /**
     * @brief Optional observer called when an initial BOM is actually stripped.
     *
     * This models iconv-lite's `stripBOM` callback form while keeping the C++
     * option that controls stripping as an explicit boolean.
     *
     * @since 1.0.0
     */
    std::function<void()> onBOMStripped;

    /**
     * @brief Fallback endianness for UTF-16/UTF-32 auto detection.
     *
     * Accepted values include `utf16le`, `utf16be`, `utf32le`, and `utf32be`.
     * Invalid values fall back to iconv-lite's default little-endian behavior.
     *
     * @since 1.0.0
     */
    std::string defaultEncoding;
};

/**
 * @brief Resolved encoding metadata for diagnostics and tests.
 * @since 1.0.0
 */
struct EncodingInfo {
    /** @brief Original label passed by the caller. @since 1.0.0 */
    std::string requested;

    /** @brief iconv-lite-style canonical label. @since 1.0.0 */
    std::string canonical;

    /** @brief Resolved upstream table name or local internal codec name. @since 1.0.0 */
    std::string converter;

    /** @brief True when BOM strip/prepend behavior applies. @since 1.0.0 */
    bool bomAware = false;

    /**
     * @brief True when handled by a local internal codec instead of generated SBCS/DBCS tables.
     * @since 1.0.0
     */
    bool isInternal = false;
};

class Encoder;
class Decoder;

/**
 * @brief Resolved codec descriptor returned by getCodec().
 *
 * JavaScript iconv-lite exposes codec constructor objects through `getCodec`.
 * This C++ companion exposes deterministic metadata plus factories for the
 * stateful encoder and decoder objects that carry stream chunk-boundary state.
 *
 * @since 1.0.0
 */
struct Codec {
    /** @brief Resolved metadata for this codec. @since 1.0.0 */
    EncodingInfo info;

    /** @brief True when BOM strip/prepend wrappers apply. @since 1.0.0 */
    bool bomAware() const noexcept { return info.bomAware; }

    /**
     * @brief True when this is a local internal codec instead of generated table data.
     * @since 1.0.0
     */
    bool isInternal() const noexcept { return info.isInternal; }

    /** @brief Create a stateful encoder for this codec. @since 1.0.0 */
    Encoder encoder(const EncodeOptions& options = {}) const;

    /** @brief Create a stateful decoder for this codec. @since 1.0.0 */
    Decoder decoder(const DecodeOptions& options = {}) const;
};

/**
 * @brief Stateful low-level encoder, matching iconv-lite `getEncoder()`.
 *
 * `write()` may be called multiple times. `end()` flushes any pending bytes
 * for encodings whose upstream encoders keep chunk-boundary state, such as
 * base64, UTF-7-IMAP, GB18030/DBCS sequence state, and BOM prepending.
 *
 * @since 1.0.0
 */
class Encoder {
public:
    /** @brief Create a stateful encoder for an encoding label. @since 1.0.0 */
    Encoder(std::string_view encoding, const EncodeOptions& options = {});

    /** @brief Create a stateful encoder from a resolved codec descriptor. @since 1.0.0 */
    Encoder(const Codec& codec, const EncodeOptions& options = {});
    Encoder(const Encoder&) = default;
    Encoder(Encoder&&) noexcept = default;
    Encoder& operator=(const Encoder&) = default;
    Encoder& operator=(Encoder&&) noexcept = default;
    ~Encoder();

    /** @brief Encode the next UTF-8 text chunk. @since 1.0.0 */
    Buffer write(std::string_view input);

    /** @brief Flush pending encoder state. @since 1.0.0 */
    Buffer end();

    /** @brief Resolved encoding metadata for this encoder. @since 1.0.0 */
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
 *
 * @since 1.0.0
 */
class Decoder {
public:
    /** @brief Create a stateful decoder for an encoding label. @since 1.0.0 */
    Decoder(std::string_view encoding, const DecodeOptions& options = {});

    /** @brief Create a stateful decoder from a resolved codec descriptor. @since 1.0.0 */
    Decoder(const Codec& codec, const DecodeOptions& options = {});
    Decoder(const Decoder&) = default;
    Decoder(Decoder&&) noexcept = default;
    Decoder& operator=(const Decoder&) = default;
    Decoder& operator=(Decoder&&) noexcept = default;
    ~Decoder();

    /** @brief Decode the next byte chunk into UTF-8 text. @since 1.0.0 */
    std::string write(const Buffer& input);

    /** @brief Flush pending decoder state. @since 1.0.0 */
    std::string end();

    /** @brief Resolved encoding metadata for this decoder. @since 1.0.0 */
    EncodingInfo info() const;

private:
    explicit Decoder(std::shared_ptr<detail::DecoderState> state);

    std::shared_ptr<detail::DecoderState> state_;

    friend struct Codec;
};

/**
 * @brief Stream construction options for iconv-lite transform streams.
 * @since 1.0.0
 */
struct IconvStreamOptions {
    /**
     * @brief Event context to bind the stream to; null uses polycpp's default event loop.
     * @since 1.0.0
     */
    EventContext* context = nullptr;

    /** @brief Read/write high-water mark in bytes. @since 1.0.0 */
    size_t highWaterMark = stream::kDefaultHighWaterMark;

    /** @brief Whether the stream emits close on destroy. @since 1.0.0 */
    bool emitClose = true;

    /** @brief Whether the stream auto-destroys after completion. @since 1.0.0 */
    bool autoDestroy = true;
};

/**
 * @brief Transform stream that encodes UTF-8 text chunks into encoded bytes.
 * @since 1.0.0
 */
class EncodeStream : public stream::Transform {
public:
    /** @brief Create an encoding transform stream for an encoding label. @since 1.0.0 */
    EncodeStream(std::string_view encoding,
                 const EncodeOptions& options = {},
                 const IconvStreamOptions& streamOptions = {});

    /** @brief Create an encoding transform stream from a stateful encoder. @since 1.0.0 */
    EncodeStream(Encoder encoder,
                 const IconvStreamOptions& streamOptions = {});

    /** @brief Create an encoding transform stream from an implementation object. @since 1.0.0 */
    explicit EncodeStream(std::shared_ptr<detail::EncodeStreamImpl> impl);
    ~EncodeStream() override;

    /**
     * @brief Collect emitted byte chunks and call the callback at end/error.
     *
     * The callback receives a non-null error on failure, otherwise the
     * concatenated output buffer.
     *
     * @since 1.0.0
     */
    EncodeStream& collect(std::function<void(Error::Ptr, Buffer)> callback);
};

/**
 * @brief Transform stream that decodes encoded byte chunks into UTF-8 text.
 *
 * The readable side emits UTF-8 buffers because polycpp streams are byte
 * oriented; `collect()` exposes the decoded text directly.
 *
 * @since 1.0.0
 */
class DecodeStream : public stream::Transform {
public:
    /** @brief Create a decoding transform stream for an encoding label. @since 1.0.0 */
    DecodeStream(std::string_view encoding,
                 const DecodeOptions& options = {},
                 const IconvStreamOptions& streamOptions = {});

    /** @brief Create a decoding transform stream from a stateful decoder. @since 1.0.0 */
    DecodeStream(Decoder decoder,
                 const IconvStreamOptions& streamOptions = {});

    /** @brief Create a decoding transform stream from an implementation object. @since 1.0.0 */
    explicit DecodeStream(std::shared_ptr<detail::DecodeStreamImpl> impl);
    ~DecodeStream() override;

    /**
     * @brief Collect emitted UTF-8 text chunks and call the callback at end/error.
     * @since 1.0.0
     */
    DecodeStream& collect(std::function<void(Error::Ptr, std::string)> callback);
};

/** @brief Compatibility alias for EncodeStream. @since 1.0.0 */
using IconvLiteEncoderStream = EncodeStream;

/** @brief Compatibility alias for DecodeStream. @since 1.0.0 */
using IconvLiteDecoderStream = DecodeStream;

/**
 * @brief Canonicalize an encoding label the same way iconv-lite does.
 *
 * The label is lowercased, a trailing `:YYYY` suffix is removed, and all
 * non-alphanumeric characters are stripped.
 *
 * @since 1.0.0
 */
std::string canonicalizeEncoding(std::string_view encoding);

/**
 * @brief Return true when an encoding label can be resolved by this port.
 * @since 1.0.0
 */
bool encodingExists(std::string_view encoding) noexcept;

/**
 * @brief Resolve an encoding label to diagnostic metadata.
 * @throws polycpp::TypeError if the label is not supported.
 * @since 1.0.0
 */
EncodingInfo inspectEncoding(std::string_view encoding);

/**
 * @brief Return the resolved codec descriptor for an encoding label.
 * @throws polycpp::TypeError if the label is not supported.
 * @since 1.0.0
 */
Codec getCodec(std::string_view encoding);

/**
 * @brief Create a stateful low-level encoder for an encoding label.
 * @since 1.0.0
 */
Encoder getEncoder(std::string_view encoding, const EncodeOptions& options = {});

/**
 * @brief Create a stateful low-level decoder for an encoding label.
 * @since 1.0.0
 */
Decoder getDecoder(std::string_view encoding, const DecodeOptions& options = {});

/**
 * @brief Current Unicode replacement string used by table decoders.
 * @since 1.0.0
 */
std::string defaultCharUnicode();

/**
 * @brief Set the Unicode replacement string used by future table decoders.
 * @since 1.0.0
 */
void setDefaultCharUnicode(std::string value);

/**
 * @brief Current single-byte replacement string used by table encoders.
 * @since 1.0.0
 */
std::string defaultCharSingleByte();

/**
 * @brief Set the single-byte replacement string used by future table encoders.
 * @since 1.0.0
 */
void setDefaultCharSingleByte(std::string value);

/**
 * @brief Encode UTF-8 text into a byte buffer using the requested encoding.
 *
 * @param input UTF-8 text.
 * @param encoding iconv-lite-style encoding label.
 * @param options encode options.
 * @return Encoded bytes as polycpp::buffer::Buffer.
 * @throws polycpp::TypeError for unsupported labels or conversion failures.
 * @since 1.0.0
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
 * @since 1.0.0
 */
std::string decode(const Buffer& input,
                   std::string_view encoding,
                   const DecodeOptions& options = {});

/**
 * @brief Streaming support is always available in this C++ companion.
 * @since 1.0.0
 */
bool supportsStreams() noexcept;

/**
 * @brief No-op compatibility hook matching iconv-lite enableStreamingAPI().
 * @since 1.0.0
 */
void enableStreamingAPI();

/**
 * @brief Create a transform stream that encodes UTF-8 text chunks.
 * @since 1.0.0
 */
EncodeStream encodeStream(std::string_view encoding,
                          const EncodeOptions& options = {},
                          const IconvStreamOptions& streamOptions = {});

/**
 * @brief Create a transform stream that decodes byte chunks into UTF-8 text.
 * @since 1.0.0
 */
DecodeStream decodeStream(std::string_view encoding,
                          const DecodeOptions& options = {},
                          const IconvStreamOptions& streamOptions = {});

/**
 * @brief Legacy upstream name for encode(), matching iconv-lite `toEncoding`.
 * @since 1.0.0
 */
inline Buffer toEncoding(std::string_view input,
                         std::string_view encoding,
                         const EncodeOptions& options = {}) {
    return encode(input, encoding, options);
}

/**
 * @brief Legacy upstream name for decode(), matching iconv-lite `fromEncoding`.
 * @since 1.0.0
 */
inline std::string fromEncoding(const Buffer& input,
                                std::string_view encoding,
                                const DecodeOptions& options = {}) {
    return decode(input, encoding, options);
}

}  // namespace polycpp::iconv_lite
