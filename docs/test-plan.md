# Test Plan

## Unit tests

- `canonicalize_encoding` strips punctuation and trailing `:YYYY` and lowercases labels.
- `encoding_exists` returns true for common aliases and false for `__proto__`, `constructor`, and unknown labels.
- UTF-8 encode/decode round trips ASCII and non-ASCII text.
- Base64 and hex match upstream direction semantics.
- `binary` maps the low byte of each code point; `latin1` substitutes unrepresentable characters with `?`.
- Windows-1251, ISO-8859-15, GBK/GB18030, Shift_JIS, EUC-KR, UTF-7, UTF-7-IMAP, and CESU-8 sample vectors match upstream-generated expected bytes.
- Untranslatable characters encode as `?` for single-byte encodings.
- Unknown encodings throw `polycpp::TypeError`.

## Integration tests

- Public API uses `polycpp::buffer::Buffer` without local byte containers.
- CMake config requires ICU and links with polycpp.
- Example target builds with `POLYCPP_ICONV_LITE_BUILD_EXAMPLES=ON`.
- Documentation build succeeds with Doxygen and Sphinx.

## Compatibility tests adapted from upstream

- Adapt `test/main-test.js` for core API, aliases, unknown labels, base64, hex, latin1, and untranslatable replacement.
- Adapt `test/bom-test.js` for UTF-8/UTF-16 BOM stripping and BOM prepend.
- Adapt `test/utf16-test.js` for UTF-16BE bytes, UTF-16 auto BOM detection, and odd-length handling.
- Adapt `test/utf32-test.js` for UTF-32LE/BE and auto BOM behavior.
- Adapt representative per-encoding tests for Cyrillic, Greek, Turkish, GBK, Big5, Shift_JIS, and EUC-JP.

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

Commands run on 2026-04-25:

- `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_ICONV_LITE_BUILD_EXAMPLES=ON`
- `cmake --build build -j$(nproc)`
- `ctest --test-dir build --output-on-failure`
- `./build/examples/convert`
- `python3 docs/build.py`
- `python3 <libgen>/scripts/check-port-validation.py --run-docs-build <iconv-lite>`
- `python3 <libgen>/scripts/check-public-readiness.py <iconv-lite>`

Results:

- Build passed with system ICU 70.1.
- 9 GoogleTest cases passed.
- Example output matched README: `cff0e8e2e5f2` followed by `Привет`.
- Documentation build, post-implementation validation, and public readiness checks passed.
