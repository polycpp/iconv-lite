# Dependency and JavaScript API Analysis

- package: iconv-lite
- package version: 0.7.2
- package root: `.tmp/npm-package`
- analyzer json: `.tmp/dependency-analysis.json`
- published npm artifact path: `.tmp/npm-package`
- published npm artifact analyzed: yes
- include dev dependencies: no
- dependency source install used: yes, analyzer installed runtime dependencies in a temporary npm analysis directory
- companion root checked: local polycpp companion checkout

## Package entry metadata

- main: `./lib/index.js`
- module: none
- types: `./lib/index.d.ts`
- exports: none
- bin: none
- missing declared entries in repo clone: none
- TypeScript source files detected: none in runtime package; `.d.ts` files are shipped
- source-vs-published artifact decision: use the published npm artifact as runtime source of truth; `diff -qr` showed the source clone and published artifact match for `lib/`, `encodings/`, and `package.json` at `v0.7.2`

## Direct dependencies

- `safer-buffer`: safe Node `Buffer` shim used by upstream for old Node versions.

## Dependency ownership decisions

| Package | Kind | Requested | Installed | License | License evidence | License impact | License strategy | Affects repo license | Deps | Source files | Node API calls | JS API calls | Recommendation | Rationale |
|---|---|---|---|---|---|---|---|---|---:|---:|---:|---:|---|---|
| safer-buffer | hard | >= 2.1.2 < 3.0.0 | 2.1.2 | MIT | package.json license field and npm metadata review | permissive | clean-room replacement | no | 0 | 3 | 0 | 0 | use base polycpp Buffer instead of a companion | Upstream only needs safer Buffer constructors; `polycpp::buffer::Buffer` already provides the binary boundary, allocation, construction, concatenation, and string conversion APIs needed by the C++ port. No safer-buffer source is vendored. |

## License impact summary

- upstream package license: MIT
- repo license decision: MIT with `polycpp contributors` as copyright holder
- GPL/AGPL dependencies: none detected
- LGPL/MPL dependencies: none detected in npm runtime dependencies
- permissive dependencies requiring notices: upstream `iconv-lite` MIT notice and `safer-buffer` MIT notice
- dev/test-only dependencies excluded from shipped artifacts: `@arethetypeswrong/cli`, `@types/node`, `async`, `bench-node`, `eslint`, `iconv`, `mocha`, `nyc`, `request`, `typescript`, `unorm`, and webpack test dependencies
- dependency license notices to add to `THIRD_PARTY_LICENSES.md`: upstream `iconv-lite` and npm dependency `safer-buffer`
- native dependency note: ICU is used as a build/runtime library through CMake/polycpp; ICU uses Unicode/ICU license terms and is not vendored in this repo

## Transitive dependency summary

- `safer-buffer` has no runtime dependencies.
- Analyzer-reported runtime dependency license is permissive.
- No npm dependency source is vendored into this C++ port.

## Runtime API usage

### Target package

- entry points analyzed: `lib/index.js`
- source files analyzed by analyzer: 14 target files
- source files manually inspected: `lib/index.js`, `lib/bom-handling.js`, `lib/streams.js`, `encodings/index.js`, `encodings/internal.js`, `encodings/utf16.js`, `encodings/utf32.js`, `encodings/utf7.js`, `test/main-test.js`, `test/bom-test.js`, `test/utf16-test.js`, `test/utf32-test.js`, and per-encoding tests
- external imports seen from target: `safer-buffer`, `stream`, and `string_decoder`

### Analyzer porting gates

- polycpp reuse hints consumed: `Buffer.alloc`, `Buffer.concat`, `Buffer.from`, and `Buffer.isBuffer` map to `polycpp::buffer::Buffer`; this is recorded in `docs/research.md`
- security hints consumed: analyzer did not classify the package as security-sensitive; manual review treats text conversion as medium-risk input parsing and requires invalid-input tests
- security-sensitive package: no for auth/crypto; yes for robust untrusted byte handling and fail-closed labels

### Node.js API usage

- `Buffer.from`, `Buffer.alloc`, `Buffer.concat`, and `Buffer.isBuffer` map to `polycpp::buffer::Buffer` APIs.
- `string_decoder.StringDecoder` behavior is not exposed directly in v0; batch decode uses ICU and explicit BOM handling.
- `stream.Transform` maps to deferred C++ stream integration; no local stream type is introduced.

### JavaScript API usage

- `Array.prototype.push` and `slice` map to `std::vector` and `std::string` operations.
- `String.prototype.charCodeAt` and `String.fromCharCode` behavior maps to UTF-8/UTF-16 conversion through ICU/polycpp.
- `RegExp.prototype.test` in canonicalization maps to deterministic ASCII normalization in C++.
- Dynamic codec registry objects map to explicit alias normalization and ICU converter probing.

### Framework object boundary usage

- analyzer-reported target-package framework object accesses: `res.0 read` and `res.length read`
- analyzer-reported dependency framework object accesses: none
- manual review decision: these are false positives for local variables named `res` in BOM/stream helper code, not framework request/response/context objects. No HTTP framework boundary exists for this package.

## Porting decisions

- Reuse `polycpp::buffer::Buffer` as the only public byte container.
- Require ICU for v0 encode/decode instead of copying upstream generated tables.
- Keep the public C++ API batch-oriented and deterministic.
- Implement explicit iconv-lite alias normalization before probing ICU.
- Preserve core BOM semantics for UTF-8, UTF-16, and UTF-32.
- Defer Node stream parity and dynamic codec registry APIs.

Each porting decision is consistent with the ecosystem reuse decisions recorded in `docs/research.md`.

## Analyzer warnings

- `safer-buffer: no entry points found for safer-buffer`
- Agent response: reviewed `safer-buffer@2.1.2` npm metadata manually and decided not to analyze or port its source because the C++ port uses `polycpp::buffer::Buffer` and does not vendor `safer-buffer`.
