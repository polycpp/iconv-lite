# API Mapping

| Upstream symbol | C++ symbol | Status | Notes |
|---|---|---|---|
| `iconv.encode(str, encoding, options?)` | `polycpp::iconv_lite::encode(std::string_view, std::string_view, const EncodeOptions&)` | adapted | Returns `polycpp::buffer::Buffer`; input string is UTF-8. |
| `iconv.decode(buf, encoding, options?)` | `polycpp::iconv_lite::decode(const Buffer&, std::string_view, const DecodeOptions&)` | adapted | Returns UTF-8 `std::string`. |
| `iconv.encodingExists(encoding)` | `polycpp::iconv_lite::encoding_exists(std::string_view)` | direct | Uses explicit aliases plus ICU converter probing. |
| `iconv.toEncoding` | `polycpp::iconv_lite::to_encoding` | direct | Alias to `encode`. |
| `iconv.fromEncoding` | `polycpp::iconv_lite::from_encoding` | direct | Alias to `decode`. |
| `iconv._canonicalizeEncoding(encoding)` | `polycpp::iconv_lite::canonicalize_encoding(std::string_view)` | direct | Lowercases, removes non-alphanumeric characters, and strips trailing `:YYYY`. |
| `DecodeOptions.stripBOM` | `DecodeOptions::strip_bom` | direct | Defaults to true. Callback form is deferred. |
| `DecodeOptions.defaultEncoding` | `DecodeOptions::default_encoding` | adapted | Used for UTF-16/UTF-32 auto-detection fallback. |
| `EncodeOptions.addBOM` | `EncodeOptions::add_bom` | direct | Optional boolean; `utf16` and `utf32` auto encoders default to BOM on. |
| `EncodeOptions.defaultEncoding` | `EncodeOptions::default_encoding` | adapted | Selects UTF-32 auto encoder endianness and future UTF auto behavior. |
| `Buffer` from `safer-buffer` | `polycpp::buffer::Buffer` | adapted | Base polycpp byte container replaces Node Buffer shim. |
| `Buffer.from(str, enc)` | `polycpp::buffer::Buffer::from(...)` or ICU conversion | adapted | Internal/base64/hex encodings reuse Buffer; legacy encodings use ICU. |
| `Buffer.concat(...)` | `polycpp::buffer::Buffer::concat(...)` | direct | Used for BOM prepend and tests. |
| UTF-7 and UTF-7-IMAP codecs | local batch UTF-7 helpers | adapted | Implemented locally because ICU's UTF-7 direct-character policy differs from iconv-lite. |
| `StringDecoder` | no public v0 equivalent | deferred | Batch decode uses ICU; stream-safe incremental decoding is deferred. |
| `encodeStream`, `decodeStream`, `enableStreamingAPI` | none in v0 | deferred | Requires a C++ stream adapter strategy aligned with polycpp streams. |
| `getCodec`, `getEncoder`, `getDecoder` | none in v0 | deferred | Dynamic codec registry is a JavaScript implementation detail. |
| `defaultCharUnicode`, `defaultCharSingleByte` | fixed substitution behavior | adapted | C++ v0 uses U+FFFD on decode and `?` substitution on encode where ICU supports substitution. |

## Framework object boundary review

- Upstream reads or mutates framework/request/response/context objects: no.
- Upstream fields or methods read: analyzer hits on `res.0` and `res.length` are local variable false positives, not framework objects.
- Upstream fields or methods written: none.
- C++ adapter boundary: byte input/output uses `polycpp::buffer::Buffer`; text input/output uses UTF-8 `std::string`.
- Partial mutation risk on validation failure: `encode` and `decode` construct local output buffers/strings and throw before returning on unsupported labels; no caller-owned object is mutated.
