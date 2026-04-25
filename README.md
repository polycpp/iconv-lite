# polycpp-iconv-lite

C++ companion port of [iconv-lite](https://www.npmjs.com/package/iconv-lite) for [polycpp](https://github.com/enricohuang/polycpp).

Port version: `0.1.0`

Initial port based on upstream version: `0.7.2`

## Status

Implemented:

- Batch `encode`, `decode`, `to_encoding`, `from_encoding`, `encoding_exists`, and `canonicalize_encoding`.
- UTF-8, CESU-8, UTF-7, UTF-7-IMAP, UTF-16LE/BE/auto, UTF-32LE/BE/auto.
- `base64`, `hex`, `binary`, ASCII, latin1, Windows-125x, ISO-8859-x, KOI8, Shift_JIS, GBK, GB18030, Big5, EUC-JP, and EUC-KR through explicit aliases and ICU.
- BOM stripping and prepend behavior for BOM-aware encodings.
- `polycpp::buffer::Buffer` as the byte boundary.

Deferred:

- Node stream APIs: `encodeStream`, `decodeStream`, and `enableStreamingAPI`.
- Dynamic codec registry APIs: `getCodec`, `getEncoder`, and `getDecoder`.
- Mutable module globals such as `defaultCharUnicode` and `defaultCharSingleByte`.
- Exact table parity for every upstream generated label when ICU does not expose equivalent mappings.

This repo does not imply full parity with upstream `iconv-lite`. Implemented and deferred behavior is tracked in `docs/research.md`, `docs/api-mapping.md`, and `docs/divergences.md`.

## Prerequisites

- C++20 compiler
- CMake 3.20+
- Ninja recommended
- ICU development libraries
- A polycpp checkout or network access for CMake FetchContent

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_ICONV_LITE_BUILD_EXAMPLES=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
./build/examples/convert
```

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
