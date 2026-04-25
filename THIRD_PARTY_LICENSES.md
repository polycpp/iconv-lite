# Third-Party Licenses

## Upstream package: iconv-lite

- Source: https://github.com/pillarjs/iconv-lite
- Version basis: 0.7.2
- License: MIT
- License evidence: upstream `package.json` and `LICENSE`
- Use in this repo: clean-room C++ port based on public behavior and tests; no upstream source is vendored

## npm dependency: safer-buffer

- Source: https://github.com/ChALkeR/safer-buffer
- npm version analyzed through `iconv-lite` dependency graph: 2.1.2
- License: MIT
- License evidence: npm package metadata
- Use in this repo: not linked or vendored; replaced by base `polycpp::buffer::Buffer`

## Native dependency: ICU

- Source: https://icu.unicode.org/
- License: Unicode/ICU License
- Use in this repo: linked as a system/CMake dependency for character set conversion; ICU source is not vendored in this repository
