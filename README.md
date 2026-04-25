# polycpp-iconv-lite

C++ companion port of [iconv-lite](https://www.npmjs.com/package/iconv-lite) for [polycpp](https://github.com/enricohuang/polycpp).

Port version: `0.1.0`

Initial port based on upstream version: `0.7.2`

## Status

Implemented:

- Batch `encode`, `decode`, `to_encoding`, `from_encoding`, `encoding_exists`, and `canonicalize_encoding`.
- UTF-8, CESU-8, UTF-7, UTF-7-IMAP, UTF-16LE/BE/auto, UTF-32LE/BE/auto.
- `base64`, `hex`, `binary`, ASCII, latin1, Windows-125x, ISO-8859-x, KOI8, Shift_JIS, GBK, GB18030, Big5, EUC-JP, and EUC-KR through upstream generated tables and aliases.
- BOM stripping and prepend behavior for BOM-aware encodings.
- `polycpp::buffer::Buffer` as the byte boundary.
- Stateful low-level `get_codec`/`get_encoder`/`get_decoder` APIs, with
  JavaScript-name aliases `getCodec`, `getEncoder`, and `getDecoder`.
- `EncodeStream`/`DecodeStream` transform streams through
  `polycpp::stream`, plus `encode_stream`, `decode_stream`, and
  `enable_streaming_api` compatibility hooks.
- Mutable replacement defaults through `set_default_char_unicode` and
  `set_default_char_single_byte`.

This repo does not imply full parity with upstream `iconv-lite`. Implemented and deferred behavior is tracked in `docs/research.md`, `docs/api-mapping.md`, and `docs/divergences.md`.

## Prerequisites

- C++20 compiler
- CMake 3.20+
- Ninja recommended
- A polycpp checkout or network access for CMake FetchContent

Standalone builds default embedded polycpp to `POLYCPP_UNICODE=builtin`
because this companion owns its iconv-lite compatibility tables and does not
need polycpp ICU/Intl. Consumers can pass `-DPOLYCPP_UNICODE=auto` or
`-DPOLYCPP_UNICODE=icu` if their wider application needs those polycpp
features.

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_ICONV_LITE_BUILD_EXAMPLES=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
./build/examples/convert
```

## Regenerating Encoding Tables

The committed generated table header is derived from the published
`iconv-lite@0.7.2` npm artifact. Regenerate it only when changing the upstream
version basis:

```bash
node tools/generate_iconv_tables.js .tmp/npm-package include/polycpp/iconv_lite/detail/generated_tables.hpp
```

The generated header is source-controlled so downstream C++ consumers do not
need Node/npm at build time.

## Usage

```cpp
#include <iostream>

#include <polycpp/iconv_lite/iconv_lite.hpp>

int main() {
    auto bytes = polycpp::iconv_lite::encode("Привет", "win1251");
    std::cout << bytes.toString("hex") << "\n";
    std::cout << polycpp::iconv_lite::decode(bytes, "cp1251") << "\n";
}
```

Expected output:

```text
cff0e8e2e5f2
Привет
```

## License

MIT. See `LICENSE` and `THIRD_PARTY_LICENSES.md`.
