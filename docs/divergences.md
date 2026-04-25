# Divergences

## Intentional C++ API Shape

- Upstream accepts JavaScript strings and Node `Buffer`; this port accepts UTF-8 `std::string_view` and `polycpp::buffer::Buffer`.
- Upstream has mutable module properties and a lazy JavaScript codec registry; this port exposes typed `Codec`, `Encoder`, and `Decoder` objects plus replacement-character setters.
- Upstream implements conversion with generated JavaScript tables; this port generates equivalent C++ table data from the published npm artifact.
- Upstream streams emit JavaScript strings from `decodeStream`; `DecodeStream` emits UTF-8 `Buffer` chunks because polycpp streams are byte-oriented, and exposes decoded text through `collect()`.

## Supported Behavior

- Batch `encode`, `decode`, `to_encoding`, `from_encoding`, `encoding_exists`, and `canonicalize_encoding`.
- UTF-8, UTF-16LE/BE/auto, UTF-32LE/BE/auto, CESU-8, latin1/binary, ASCII, base64, and hex.
- Generated-table legacy encodings such as Windows-125x, ISO-8859-x, KOI8, Shift_JIS, GBK, GB18030, Big5, EUC-JP, and EUC-KR.
- UTF-7 and UTF-7-IMAP are implemented locally to match iconv-lite byte behavior for direct characters and modified base64 shifts.
- BOM stripping and prepending for BOM-aware UTF encodings.
- Encode substitution for unrepresentable characters follows upstream SBCS/DBCS default-character behavior.
- Stateful `get_codec`, `get_encoder`, and `get_decoder` APIs preserve chunk-boundary state for base64, DBCS, UTF-7, UTF-16, and UTF-32.
- `encode_stream` and `decode_stream` use `polycpp::stream::Transform`, with JavaScript-name aliases available for compatibility.
- Mutable replacement defaults are available through setter/getter functions.

## Deferred Behavior

- Callback form of `stripBOM`.
- Browser bundling behavior.
- Public mutation of codec cache internals such as `_codecDataCache`.
- Exact JavaScript object registry mutation through `iconv.encodings`; C++ codec definitions are compiled from generated tables.

## Unsupported Runtime-Specific Features

- JavaScript prototype pollution edge cases are not represented; C++ label lookup has no prototype chain.
- JavaScript object truthiness/coercion of arbitrary input values is not represented; C++ APIs are typed.
- Node object-mode or arbitrary JavaScript chunk typing is not represented; polycpp streams carry buffers/text, and `EncodeStream` interprets written chunks as UTF-8 text.

## Compatibility Notes

- `binary` follows Node/iconv-lite internal low-byte behavior and intentionally differs from `latin1` for characters outside ISO-8859-1.
- `utf16` and `utf32` auto encoders add a BOM by default unless `add_bom=false`.
- Unknown labels throw `polycpp::TypeError` rather than returning partial output.
- Generated tables are based on upstream `iconv-lite@0.7.2`; update the generator and fixtures when changing the upstream version basis.
