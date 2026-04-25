# API Mapping

| Upstream symbol | C++ symbol | Status | Notes |
|---|---|---|---|
| `iconv.encode(str, encoding, options?)` | `polycpp::iconv_lite::encode(std::string_view, std::string_view, const EncodeOptions&)` | adapted | Returns `polycpp::buffer::Buffer`; input string is UTF-8. |
| `iconv.decode(buf, encoding, options?)` | `polycpp::iconv_lite::decode(const Buffer&, std::string_view, const DecodeOptions&)` | adapted | Returns UTF-8 `std::string`. |
| `iconv.encodingExists(encoding)` | `polycpp::iconv_lite::encoding_exists(std::string_view)` | direct | Uses iconv-lite canonical aliases plus generated upstream table entries. |
| `iconv.toEncoding` | `polycpp::iconv_lite::to_encoding` | direct | Alias to `encode`. |
| `iconv.fromEncoding` | `polycpp::iconv_lite::from_encoding` | direct | Alias to `decode`. |
| `iconv._canonicalizeEncoding(encoding)` | `polycpp::iconv_lite::canonicalize_encoding(std::string_view)` | direct | Lowercases, removes non-alphanumeric characters, and strips trailing `:YYYY`. |
| `iconv.getCodec(encoding)` | `polycpp::iconv_lite::get_codec(std::string_view)` / `getCodec` | adapted | Returns resolved `Codec` metadata plus factories for C++ stateful encoder/decoder objects. |
| `iconv.getEncoder(encoding, options?)` | `polycpp::iconv_lite::get_encoder(std::string_view, const EncodeOptions&)` / `getEncoder` | adapted | Returns a stateful `Encoder` with `write(std::string_view)` and `end()`. |
| `iconv.getDecoder(encoding, options?)` | `polycpp::iconv_lite::get_decoder(std::string_view, const DecodeOptions&)` / `getDecoder` | adapted | Returns a stateful `Decoder` with `write(const Buffer&)` and `end()`. |
| `encodeStream`, `decodeStream`, `enableStreamingAPI` | `encode_stream`, `decode_stream`, `enable_streaming_api` plus JS-name aliases | adapted | Uses `polycpp::stream::Transform`; C++ stream chunks are byte buffers, so `DecodeStream` emits UTF-8 `Buffer` chunks and `collect()` exposes decoded text. |
| `DecodeOptions.stripBOM` | `DecodeOptions::strip_bom` | direct | Defaults to true. Callback form is deferred. |
| `DecodeOptions.defaultEncoding` | `DecodeOptions::default_encoding` | adapted | Used for UTF-16/UTF-32 auto-detection fallback. |
| `EncodeOptions.addBOM` | `EncodeOptions::add_bom` | direct | Optional boolean; `utf16` and `utf32` auto encoders default to BOM on. |
| `EncodeOptions.defaultEncoding` | `EncodeOptions::default_encoding` | adapted | Selects UTF-32 auto encoder endianness and future UTF auto behavior. |
| `defaultCharUnicode`, `defaultCharSingleByte` | `default_char_unicode`, `set_default_char_unicode`, `default_char_single_byte`, `set_default_char_single_byte` | adapted | C++ uses accessors/setters instead of mutable module properties. Future conversions observe current values. |
| `Buffer` from `safer-buffer` | `polycpp::buffer::Buffer` | adapted | Base polycpp byte container replaces Node Buffer shim. |
| `Buffer.from(str, enc)` | `polycpp::buffer::Buffer::from(...)` or generated table conversion | adapted | Internal/base64/hex encodings reuse Buffer; legacy encodings use generated upstream SBCS/DBCS tables. |
| `Buffer.concat(...)` | `polycpp::buffer::Buffer::concat(...)` | direct | Used for BOM prepend and tests. |
| UTF-7 and UTF-7-IMAP codecs | local batch UTF-7 helpers | adapted | Implemented locally from upstream UTF-7 logic. |
| `StringDecoder` | internal stateful decoder reuse/adaptation | adapted | UTF-8 streaming uses `polycpp::string_decoder`; UTF-16/UTF-32/DBCS/UTF-7 state is implemented locally to preserve iconv-lite chunk behavior. |

## Framework object boundary review

- Upstream reads or mutates framework/request/response/context objects: no.
- Upstream fields or methods read: analyzer hits on `res.0` and `res.length` are local variable false positives, not framework objects.
- Upstream fields or methods written: none.
- C++ adapter boundary: byte input/output uses `polycpp::buffer::Buffer`; text input/output uses UTF-8 `std::string`.
- Partial mutation risk on validation failure: `encode` and `decode` construct local output buffers/strings and throw before returning on unsupported labels; no caller-owned object is mutated.
