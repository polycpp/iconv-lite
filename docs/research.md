# Research

- package: iconv-lite
- npm url: https://www.npmjs.com/package/iconv-lite
- source url: https://github.com/pillarjs/iconv-lite.git
- upstream version basis: 0.7.2
- upstream revision analyzed: 6cf4f0829ccd2292e71b55d768766a7bfc12c133
- upstream default branch: master
- license: MIT
- license evidence: package.json license field and upstream LICENSE
- category: character encoding conversion

## Package purpose

`iconv-lite` converts between JavaScript strings and byte buffers for UTF, single-byte, and multibyte legacy encodings. It also exposes encoding-existence checks and stream helpers.

## Runtime assumptions

- browser: upstream can run in bundled browser builds, but the C++ port targets native polycpp consumers.
- node.js: upstream uses Node `Buffer`, `stream`, and `string_decoder`; the C++ port must reuse polycpp equivalents where applicable.
- filesystem: no runtime filesystem dependency in the published package.
- network: none.
- crypto: none.
- terminal: none.

## Dependency summary

- package.json present: yes
- package main: `./lib/index.js`
- package types: `./lib/index.d.ts`
- hard dependencies: `safer-buffer`
- peer dependencies: none detected in package.json
- optional dependencies: none detected in package.json
- dependency analysis report: `docs/dependency-analysis.md`

## Upstream repo layout summary

Clone path used for analysis: `.tmp/upstream/iconv-lite`

Top files:

- `lib/index.js`
- `lib/bom-handling.js`
- `lib/streams.js`
- `lib/index.d.ts`
- `encodings/index.js`
- `encodings/internal.js`
- `encodings/utf16.js`
- `encodings/utf32.js`
- `encodings/utf7.js`
- `encodings/sbcs-codec.js`
- `encodings/sbcs-data.js`
- `encodings/sbcs-data-generated.js`
- `encodings/dbcs-codec.js`
- `encodings/dbcs-data.js`
- `encodings/tables/*.json`
- `test/*.js`
- `package.json`
- `README.md`
- `LICENSE`

Likely important implementation files:

- `lib/index.js`: public API, canonicalization, codec lookup, encode/decode entry points, stream enablement.
- `lib/bom-handling.js`: BOM prepend/strip wrappers.
- `encodings/internal.js`: Node-native encodings, base64/hex, UTF-8/CESU-8 handling.
- `encodings/utf16.js`: UTF-16BE and UTF-16 auto-detection/BOM behavior.
- `encodings/utf32.js`: UTF-32LE/BE and UTF-32 auto-detection/BOM behavior.
- `encodings/utf7.js`: UTF-7 and UTF-7-IMAP implementation.
- `encodings/sbcs-*` and `encodings/dbcs-*`: generated legacy encoding tables.
- `test/main-test.js`, `test/bom-test.js`, `test/utf16-test.js`, `test/utf32-test.js`, `test/cesu8-test.js`, and per-encoding tests: compatibility evidence.

## Entry points used by consumers

- `./lib/index.js`
- Type declarations: `./lib/index.d.ts` and `types/encodings.d.ts`

Tests, fixtures, examples, and docs directories:

- `test`: upstream compatibility tests for core APIs, BOM handling, Unicode encodings, SBCS/DBCS encodings, and streams.
- `generation`: table-generation scripts; useful as provenance, not runtime source for the C++ port.
- `performance`: benchmark-only.

## Important files and why they matter

- `lib/index.js`: source of truth for public API names and alias behavior.
- `types/encodings.d.ts`: generated list of accepted upstream labels; useful when choosing alias coverage.
- `README.md`: documents supported encoding families and BOM semantics.
- `test/main-test.js`: covers `encode`, `decode`, `encodingExists`, aliases, and error handling.
- `test/bom-test.js`: release-blocking behavior for BOM strip/prepend.
- `test/utf16-test.js` and `test/utf32-test.js`: endianness and auto-detection behavior.
- `test/cyrillic-test.js`, `test/gbk-test.js`, `test/big5-test.js`, `test/shiftjis-test.js`: legacy encoding compatibility samples.

## Files likely irrelevant to the C++ port

- `.github/*`, `eslint.config.js`, `tsconfig.json`: upstream CI/lint/typecheck setup.
- `performance/*`: benchmark harness only.
- `generation/*`: used by upstream to build JavaScript table files; the C++ port regenerates C++ table data from the published upstream package rather than delegating to a platform converter.
- `test/webpack/*`: browser bundling checks, not relevant to native C++.

## Test directories worth mining first

- `test`: adapt core encode/decode, BOM, UTF-16/UTF-32, CESU-8, and common legacy encoding samples.
- Upstream tests with generated tables should be sampled rather than copied wholesale because the C++ port now uses upstream table data and should pin representative exact fixtures.

## Implementation risks discovered from the source layout

- Upstream supports hundreds of labels through generated JS tables; the C++ port generates those tables into a committed C++ header and resolves aliases through the upstream registry.
- `safer-buffer` and Node `Buffer` are upstream implementation dependencies; the C++ port must reuse `polycpp::buffer::Buffer`.
- Upstream stream APIs depend on Node streams; the C++ port adapts them to `polycpp::stream::Transform` while keeping batch encode/decode deterministic.
- Encoding aliases such as `win1251`, `1251`, `utf16le`, `utf32le`, `binary`, and `ucs2` need explicit normalization because generated table lookup must follow iconv-lite canonicalized aliases exactly.
- BOM behavior differs by encoding family and must be tested explicitly.
- Untranslatable characters should substitute `?` on encode where possible, matching upstream behavior.

## Companion repo alignment

- companion repos inspected: `content-type`, `vary`, `cors`, `jsonwebtoken`, `sequelize`
- CMake target and alias pattern: `polycpp_iconv_lite` with alias `polycpp::iconv_lite`
- public header layout: `include/polycpp/iconv_lite/iconv_lite.hpp`
- detail/private header strategy: keep implementation helpers in `src/iconv_lite.cpp`; only small template or inline helpers go in `detail/aggregator.hpp` if needed
- aggregator header strategy: keep main public header as the only required include; detail aggregator can remain minimal if no template adapters are needed
- examples strategy: provide a batch conversion example using Windows-1251 encode/decode behavior
- documentation site strategy: Doxygen plus Sphinx pages, with generated placeholder pages replaced before implementation validation
- deliberate deviations from existing companions: this port commits generated C++ data from upstream JavaScript encoding tables to preserve iconv-lite-specific compatibility without a native encoding SDK

## Polycpp ecosystem reuse analysis

- polycpp core paths inspected: `include/polycpp/buffer/buffer.hpp`, `include/polycpp/buffer/detail/buffer.hpp`, `include/polycpp/string_decoder/string_decoder.hpp`, and `include/polycpp/stream/stream.hpp`
- polycpp core types/functions selected: `polycpp::buffer::Buffer`, `Buffer::from`, `Buffer::toString`, `Buffer::concat`, `Buffer::data`, `Buffer::length`, `polycpp::string_decoder::StringDecoder`, `polycpp::stream::Transform`, and `polycpp::TypeError`
- polycpp core types/functions rejected: `polycpp::unicode::EncodingConverter`; this package needs iconv-lite-specific table parity rather than a platform converter abstraction
- companion libs inspected for reusable APIs: current companion libs under local polycpp companion checkout; no existing encoding-conversion companion exists
- companion libs selected for reuse: none
- companion libs rejected or deferred: no separate `safer-buffer` companion; its purpose is already covered by `polycpp::buffer::Buffer`
- new local abstractions introduced: `EncodeOptions`, `DecodeOptions`, `EncodingInfo`, `Codec`, `Encoder`, `Decoder`, `EncodeStream`, `DecodeStream`, `encode`, `decode`, `encoding_exists`, and `canonicalize_encoding`; these model iconv-lite policy, not a new binary buffer type
- reuse risks or integration gaps: this repo now owns generated table data; regenerate it when the upstream version basis changes and keep provenance documented

## External SDK and native driver strategy

- upstream external services/protocols: none
- native SDKs/client libraries to use: none for encoding conversion; generated table data comes from the upstream npm artifact
- SDKs/protocols explicitly not reimplemented: Node's JavaScript stream engine is not reimplemented; stream APIs are adapted to polycpp streams
- adapter/linking strategy: link `polycpp` only; do not add a direct ICU/iconv dependency for this companion; default embedded polycpp to `POLYCPP_UNICODE=builtin` unless the consumer explicitly selects another backend
- test environment needs: CMake and GoogleTest for normal validation; Node/npm only when regenerating upstream-derived tables or expected fixtures
- table regeneration command: `node tools/generate_iconv_tables.js .tmp/npm-package include/polycpp/iconv_lite/detail/generated_tables.hpp`

## Compatibility foundation review

- downstream dependency role: foundational compatibility package; many npm packages use iconv-lite transitively for package-specific encoding label and byte conversion behavior
- native substitution risk: high; a platform converter can differ from iconv-lite aliases, UTF-7 behavior, GB18030 mapping details, surrogate handling, and substitution rules
- upstream implementation data to preserve: generated SBCS/DBCS tables, encoding alias registry, BOM defaults, UTF-7/UTF-7-IMAP logic, CESU-8 behavior, and default `?` encode substitution
- generated or vendored data plan: commit generated C++ table data derived from the published `iconv-lite@0.7.2` npm artifact; do not vendor upstream JavaScript source files
- compatibility fixture strategy: keep byte-level fixtures generated from upstream for representative SBCS, DBCS, UTF, BOM, alias, unknown-label, untranslatable-character, stream chunking, and GB18030 edge behavior; `tests/upstream/*_test.cpp` files mirror the upstream test files each cluster was adapted from

## Security and fail-closed review

- security-sensitive behavior: medium; decoding untrusted bytes can affect text interpretation, but this package does not enforce authentication, authorization, crypto, or HTML sanitization
- trust boundary: input bytes and requested encoding labels are caller-controlled
- supported protocol or algorithm matrix: UTF-8, UTF-16LE/BE/auto, UTF-32LE/BE/auto, CESU-8, base64, hex, binary/latin1, ASCII, Windows-125x, ISO-8859-x, KOI8, Shift_JIS, GBK/GB18030/Big5/EUC-JP/EUC-KR from upstream generated table data
- stateful parser/session-state policy, if protocol client/server: not a protocol client/server; stateful converter policy is limited to chunk-boundary state for encoders/decoders and is covered by low-level and stream tests
- binary payload type-mapping policy, if protocol client: not a protocol client; binary conversion input/output uses `polycpp::buffer::Buffer` and decoded text uses UTF-8 `std::string`
- unsupported behavior and fail-closed policy: unsupported labels throw `polycpp::TypeError`; browser bundling and JavaScript registry mutation internals are absent rather than partially emulated
- key, secret, credential, or user-controlled input handling: no secrets; invalid labels and invalid byte sequences are tested, and conversion APIs do not execute code
- misuse cases that must be tested: unknown encodings, prototype-looking labels such as `__proto__`, invalid/incomplete UTF-16/UTF-32 byte lengths, untranslatable encode characters, BOM stripping disabled, and common alias normalization

## Core use cases

- Decode a byte buffer in a legacy encoding into UTF-8 text.
- Encode UTF-8 text into a legacy byte buffer.
- Check whether an encoding label is supported before converting.
- Preserve upstream BOM strip/prepend behavior for UTF encodings.
- Use `polycpp::buffer::Buffer` as the binary boundary.

## Key features to port first

- `encode`, `decode`, `encoding_exists`, `to_encoding`, `from_encoding`, and `canonicalize_encoding`.
- `get_codec`, `get_encoder`, `get_decoder`, `encode_stream`, `decode_stream`, and mutable replacement defaults using typed C++ APIs.
- `EncodeOptions::add_bom` and `DecodeOptions::strip_bom` / `default_encoding`.
- Explicit aliases from iconv-lite generated registry data.
- Internal encodings: UTF-8, UTF-16LE/BE/auto, UTF-32LE/BE/auto, CESU-8, latin1/binary, ASCII, base64, and hex.
- Generated SBCS/DBCS table codecs with upstream substitution behavior.

## Features to defer

- Vendoring upstream JavaScript source files.
- Browser bundling behavior.
- Callback form of `stripBOM`.
- Public mutation of JavaScript registry/cache internals such as `iconv.encodings` and `_codecDataCache`.

## v0 scope

- port version: 0.1.0
- versioning note: port version is independent from upstream versioning
- supported APIs: `EncodeOptions`, `DecodeOptions`, `EncodingInfo`, `Codec`, `Encoder`, `Decoder`, `EncodeStream`, `DecodeStream`, `canonicalize_encoding`, `encoding_exists`, `get_codec`, `get_encoder`, `get_decoder`, `encode_stream`, `decode_stream`, `enable_streaming_api`, `encode`, `decode`, `to_encoding`, `from_encoding`, replacement-default getters/setters, and JavaScript-name aliases for the upstream API names
- unsupported APIs: callback-style BOM hooks, browser webpack behavior, and public mutation of JavaScript codec registry/cache internals
- dependency plan: use `polycpp::buffer::Buffer` instead of `safer-buffer`; use generated C++ data from upstream JavaScript tables
- polycpp modules to use: `polycpp::buffer::Buffer`, `polycpp::stream::Transform`, `polycpp::string_decoder::StringDecoder`, and `polycpp::TypeError`
- missing polycpp primitives: none blocking; this package intentionally owns iconv-lite table parity inside the companion
