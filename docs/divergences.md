# Divergences

## Intentional C++ API Shape

- Upstream accepts JavaScript strings and Node `Buffer`; this port accepts UTF-8 `std::string_view` and `polycpp::buffer::Buffer`.
- Upstream has mutable module globals and a dynamic codec registry; this port exposes deterministic functions and fixed options.
- Upstream implements conversion in pure JavaScript tables; this port uses ICU/native conversion to fit the polycpp ecosystem and avoid vendoring generated JS tables.

## Supported Behavior

- Batch `encode`, `decode`, `to_encoding`, `from_encoding`, `encoding_exists`, and `canonicalize_encoding`.
- UTF-8, UTF-16LE/BE/auto, UTF-32LE/BE/auto, CESU-8, latin1/binary, ASCII, base64, and hex.
- ICU-backed legacy encodings such as Windows-125x, ISO-8859-x, KOI8, Shift_JIS, GBK, GB18030, Big5, EUC-JP, and EUC-KR when ICU supports the label.
- BOM stripping and prepending for BOM-aware UTF encodings.
- Encode substitution for unrepresentable characters where ICU supports substitution.

## Deferred Behavior

- Node `encodeStream`, `decodeStream`, and `enableStreamingAPI` parity.
- Public `getCodec`, `getEncoder`, and `getDecoder` APIs.
- Runtime mutation of `defaultCharUnicode`, `defaultCharSingleByte`, or codec cache internals.
- Callback form of `stripBOM`.
- Browser bundling behavior.
- Exact parity for all 416 generated labels in `types/encodings.d.ts` when ICU does not support the label directly or through v0 alias rules.

## Unsupported Runtime-Specific Features

- JavaScript prototype pollution edge cases are not represented; C++ label lookup has no prototype chain.
- JavaScript object truthiness/coercion of arbitrary input values is not represented; C++ APIs are typed.
- Node stream backpressure and chunk-boundary behavior are not represented in v0.

## Compatibility Notes

- `binary` is treated as latin1, matching Node/iconv-lite internal behavior.
- `utf16` and `utf32` auto encoders add a BOM by default unless `add_bom=false`.
- Unknown labels throw `polycpp::TypeError` rather than returning partial output.
- ICU may support labels or mappings that differ in small edge cases from upstream generated tables; tests pin representative compatibility cases.
