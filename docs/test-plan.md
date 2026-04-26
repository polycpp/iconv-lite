# Test Plan

## Unit tests

- `canonicalizeEncoding` strips punctuation and trailing `:YYYY` and lowercases labels.
- `encodingExists` returns true for common aliases and false for `__proto__`, `constructor`, and unknown labels.
- UTF-8 encode/decode round trips ASCII and non-ASCII text.
- Base64 and hex match upstream direction semantics.
- `binary` maps the low byte of each code point; `latin1` substitutes unrepresentable characters with `?`.
- Windows-1251, ISO-8859-15, GBK/GB18030, Shift_JIS, EUC-KR, UTF-7, UTF-7-IMAP, and CESU-8 sample vectors match upstream-generated expected bytes.
- Upstream-derived compatibility fixtures in `tests/upstream/*_test.cpp` cover the major upstream test clusters directly.
- `tests/upstream/sbcs_test.cpp` sweeps all generated SBCS tables for byte-to-text decode behavior and representable text-to-byte encode behavior.
- `tests/upstream/dbcs_test.cpp` touches every generated DBCS codec entry and checks representative extension, compatibility, and truncated-lead-byte vectors from upstream `dbcs-test.js`.
- `types/encodings.d.ts` coverage checks that all 412 literal TypeScript `Encoding` labels are accepted by `encodingExists`.
- Untranslatable characters encode as `?` for single-byte encodings.
- Unknown encodings throw `polycpp::TypeError`.
- `getCodec`, `getEncoder`, and `getDecoder` expose stateful conversion and preserve chunk-boundary state.
- `encodeStream` and `decodeStream` transform chunks through `polycpp::stream::Transform`.
- Mutable replacement defaults affect future conversions and can be restored.

## Integration tests

- Public API uses `polycpp::buffer::Buffer` without local byte containers.
- CMake config links only the companion target and polycpp; embedded polycpp defaults to `POLYCPP_UNICODE=builtin`, so no ICU/iconv link is required for standalone validation.
- Example target builds with `POLYCPP_ICONV_LITE_BUILD_EXAMPLES=ON`.
- Documentation build succeeds with Doxygen and Sphinx.
- Real service-backed e2e coverage: not applicable; this is not an external service, database, or network protocol client.
- Stateful parser/session-state coverage: low-level encoder/decoder tests cover base64 carry state, DBCS incomplete byte sequences, UTF-7-IMAP base64 state, and UTF-16/UTF-32 auto-detection chunks.

## Compatibility tests adapted from upstream

- Implemented locally under `tests/upstream/`; lower-level state and stream regressions also remain in `tests/test_smoke.cpp`.
- `test/main-test.js` -> `tests/upstream/main_test.cpp` for core API, aliases, unknown labels, base64, hex, latin1, and untranslatable replacement.
- `test/bom-test.js` -> `tests/upstream/bom_test.cpp` for UTF-8/UTF-16 BOM stripping, BOM-strip callbacks, and BOM prepend.
- `test/utf16-test.js` -> `tests/upstream/utf16_test.cpp` for UTF-16BE bytes, UTF-16 auto BOM detection, and odd-length handling.
- `test/utf32-test.js` -> `tests/upstream/utf32_test.cpp` for UTF-32LE/BE and auto BOM behavior.
- `test/utf7-test.js` -> `tests/upstream/utf7_test.cpp` for RFC/example vectors and shift handling.
- `test/cesu8-test.js` -> `tests/upstream/cesu8_test.cpp` for surrogate-byte behavior.
- `test/streams-test.js` -> `tests/upstream/streams_test.cpp` for chunk-boundary behavior and `collect()` helpers through the C++ stream API.
- `test/sbcs-test.js` -> `tests/upstream/sbcs_test.cpp` for full generated SBCS table sweeps without requiring native iconv.
- `test/dbcs-test.js` -> `tests/upstream/dbcs_test.cpp` for generated DBCS entry coverage and representative extension/error vectors without requiring native iconv.
- `types/encodings.d.ts` -> `tests/upstream/types_test.cpp` for full typed encoding-label coverage.
- `test/cyrillic-test.js` -> `tests/upstream/cyrillic_test.cpp` for Cyrillic encodings, aliases, and substitution.
- `test/greek-test.js` -> `tests/upstream/greek_test.cpp` for Greek encodings and substitution.
- `test/turkish-test.js` -> `tests/upstream/turkish_test.cpp` for Windows-1254 vectors and substitution.
- `test/big5-test.js` -> `tests/upstream/big5_test.cpp` for Big5 and CP950 vectors.
- `test/gbk-test.js` -> `tests/upstream/gbk_test.cpp` for GBK, GB18030 four-byte, incomplete-sequence, Euro, and GB18030:2005 mapping cases.
- `test/shiftjis-test.js` -> `tests/upstream/shiftjis_test.cpp` for Shift_JIS and extension vectors.

## Security and fail-closed tests

- Unsupported labels throw before returning output.
- Prototype-looking labels are not accepted.
- Invalid byte sequences decode with replacement in non-fatal behavior rather than reading out of bounds.
- Odd UTF-16 and UTF-32 byte lengths do not crash.
- Empty input returns empty output.

## Release-blocking behaviors

- Unit tests pass.
- Example compiles and runs.
- README examples match actual code.
- `python3 docs/build.py` passes.
- `python3 <libgen>/scripts/check-port-validation.py --run-docs-build <iconv-lite>` passes.
- Public readiness passes before repository visibility changes.

## Current validation

Commands run on 2026-04-26:

- `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_ICONV_LITE_BUILD_EXAMPLES=ON`
- `cmake --build build -j$(nproc)`
- `ctest --test-dir build --output-on-failure`
- `./build/examples/convert`
- `./build/examples/bom`
- `./build/examples/stream`
- `python3 docs/build.py`
- `python3 <libgen>/scripts/check-port-validation.py --run-docs-build <iconv-lite>`
- `python3 <libgen>/scripts/check-public-readiness.py <iconv-lite>`

Results:

- Build passed without a direct iconv-lite ICU/iconv dependency.
- 36 GoogleTest cases passed.
- A one-off `.d.ts` probe also confirmed all 412 labels from `.tmp/npm-package/types/encodings.d.ts` returned true from `encodingExists`.
- Example outputs matched README and Sphinx example docs.
- Documentation build, post-implementation validation, and public readiness checks passed.
