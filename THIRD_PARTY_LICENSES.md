# Third-Party Licenses

## Upstream package: iconv-lite

- Source: https://github.com/pillarjs/iconv-lite
- Version basis: 0.7.2
- License: MIT
- License evidence: upstream `package.json` and `LICENSE`
- Use in this repo: clean-room C++ port based on public behavior, tests, and generated encoding table data from the published npm artifact; no upstream source files are vendored

## npm dependency: safer-buffer

- Source: https://github.com/ChALkeR/safer-buffer
- npm version analyzed through `iconv-lite` dependency graph: 2.1.2
- License: MIT
- License evidence: npm package metadata
- Use in this repo: not linked or vendored; replaced by base `polycpp::buffer::Buffer`
