#!/usr/bin/env node
'use strict';

const fs = require('fs');
const path = require('path');

if (process.argv.length < 4) {
  console.error('usage: generate_iconv_tables.js <npm-package-dir> <output-header>');
  process.exit(2);
}

const packageDir = path.resolve(process.argv[2]);
const outFile = path.resolve(process.argv[3]);
const encodings = require(path.join(packageDir, 'encodings'));

const u16Data = [];
const chunkParts = [];
const chunks = [];
const skipRanges = [];
const addMappings = [];
const gbRanges = [];
const sbcsSpecs = [];
const dbcsSpecs = [];
const entries = [];
const sbcsIndex = new Map();
const dbcsIndex = new Map();

function addU16String(str) {
  const offset = u16Data.length;
  for (let i = 0; i < str.length; i++) u16Data.push(str.charCodeAt(i));
  return { offset, length: str.length };
}

function cxxString(str) {
  return '"' + String(str).replace(/\\/g, '\\\\').replace(/"/g, '\\"') + '"';
}

function sanitizeId(name) {
  return String(name).replace(/[^A-Za-z0-9_]/g, '_');
}

function addSbcsSpec(name, obj) {
  if (sbcsIndex.has(name)) return sbcsIndex.get(name);
  const chars = addU16String(obj.chars);
  const index = sbcsSpecs.length;
  sbcsSpecs.push({ name, chars });
  sbcsIndex.set(name, index);
  return index;
}

function addDbcsSpec(name, obj) {
  if (dbcsIndex.has(name)) return dbcsIndex.get(name);

  const table = obj.table();
  const chunksOffset = chunks.length;
  for (const chunk of table) {
    const start = parseInt(chunk[0], 16);
    const partsOffset = chunkParts.length;
    for (let i = 1; i < chunk.length; i++) {
      const part = chunk[i];
      if (typeof part === 'number') {
        chunkParts.push({ isNumber: true, number: part, text: { offset: 0, length: 0 } });
      } else if (typeof part === 'string') {
        chunkParts.push({ isNumber: false, number: 0, text: addU16String(part) });
      } else {
        throw new Error(`unexpected chunk part in ${name}: ${typeof part}`);
      }
    }
    chunks.push({ start, partsOffset, partsCount: chunk.length - 1 });
  }

  const skipOffset = skipRanges.length;
  if (obj.encodeSkipVals) {
    for (const val of obj.encodeSkipVals) {
      if (typeof val === 'number') skipRanges.push({ from: val, to: val });
      else skipRanges.push({ from: val.from, to: val.to });
    }
  }

  const addOffset = addMappings.length;
  if (obj.encodeAdd) {
    for (const [key, value] of Object.entries(obj.encodeAdd)) {
      addMappings.push({ code: key.charCodeAt(0), value });
    }
  }

  const gbOffset = gbRanges.length;
  if (typeof obj.gb18030 === 'function') {
    const gb = obj.gb18030();
    for (let i = 0; i < gb.gbChars.length; i++) {
      gbRanges.push({ uChar: gb.uChars[i], gbChar: gb.gbChars[i] });
    }
  }

  const index = dbcsSpecs.length;
  dbcsSpecs.push({
    name,
    chunksOffset,
    chunksCount: table.length,
    skipOffset,
    skipCount: skipRanges.length - skipOffset,
    addOffset,
    addCount: addMappings.length - addOffset,
    gbOffset,
    gbCount: gbRanges.length - gbOffset,
  });
  dbcsIndex.set(name, index);
  return index;
}

for (const [name, value] of Object.entries(encodings)) {
  if (name.startsWith('_')) continue;
  if (typeof value === 'string') {
    entries.push({ name, kind: 'alias', index: 0, target: value });
  } else if (typeof value === 'object' && value) {
    if (value.type === '_sbcs') {
      entries.push({ name, kind: 'sbcs', index: addSbcsSpec(name, value), target: '' });
    } else if (value.type === '_dbcs') {
      entries.push({ name, kind: 'dbcs', index: addDbcsSpec(name, value), target: '' });
    }
  }
}

entries.sort((a, b) => a.name.localeCompare(b.name));

function writeNumberArray(name, values, type = 'uint16_t') {
  let out = `inline constexpr ${type} ${name}[] = {\n`;
  for (let i = 0; i < values.length; i += 16) {
    out += '    ' + values.slice(i, i + 16).join(', ') + ',\n';
  }
  out += '};\n\n';
  return out;
}

let output = `#pragma once\n\n`;
output += `// Generated from iconv-lite 0.7.2 published npm encoding tables.\n`;
output += `// Source package: https://github.com/pillarjs/iconv-lite\n`;
output += `// Do not edit by hand; regenerate with tools/generate_iconv_tables.js.\n\n`;
output += `#include <cstddef>\n#include <cstdint>\n#include <string_view>\n\n`;
output += `namespace polycpp::iconv_lite::generated {\n\n`;
output += `enum class GeneratedKind : uint8_t { alias, sbcs, dbcs };\n\n`;
output += `struct U16Slice { uint32_t offset; uint32_t length; };\n`;
output += `struct ChunkPart { bool is_number; int32_t number; U16Slice text; };\n`;
output += `struct Chunk { uint32_t start; uint32_t parts_offset; uint32_t parts_count; };\n`;
output += `struct SkipRange { uint32_t from; uint32_t to; };\n`;
output += `struct AddMapping { uint32_t code; uint32_t value; };\n`;
output += `struct GbRange { uint32_t u_char; uint32_t gb_char; };\n`;
output += `struct SbcsSpec { std::string_view name; U16Slice chars; };\n`;
output += `struct DbcsSpec { std::string_view name; uint32_t chunks_offset; uint32_t chunks_count; uint32_t skip_offset; uint32_t skip_count; uint32_t add_offset; uint32_t add_count; uint32_t gb_offset; uint32_t gb_count; };\n`;
output += `struct EncodingEntry { std::string_view name; GeneratedKind kind; uint32_t index; std::string_view target; };\n\n`;
output += writeNumberArray('U16_DATA', u16Data, 'uint16_t');

output += `inline constexpr ChunkPart CHUNK_PARTS[] = {\n`;
for (const p of chunkParts) {
  output += `    {${p.isNumber ? 'true' : 'false'}, ${p.number}, {${p.text.offset}, ${p.text.length}}},\n`;
}
output += `};\n\n`;

output += `inline constexpr Chunk CHUNKS[] = {\n`;
for (const c of chunks) {
  output += `    {${c.start}, ${c.partsOffset}, ${c.partsCount}},\n`;
}
output += `};\n\n`;

output += `inline constexpr SkipRange SKIP_RANGES[] = {\n`;
for (const r of skipRanges) output += `    {${r.from}, ${r.to}},\n`;
output += `};\n\n`;

output += `inline constexpr AddMapping ADD_MAPPINGS[] = {\n`;
for (const m of addMappings) output += `    {${m.code}, ${m.value}},\n`;
output += `};\n\n`;

output += `inline constexpr GbRange GB_RANGES[] = {\n`;
for (const r of gbRanges) output += `    {${r.uChar}, ${r.gbChar}},\n`;
output += `};\n\n`;

output += `inline constexpr SbcsSpec SBCS_SPECS[] = {\n`;
for (const s of sbcsSpecs) output += `    {${cxxString(s.name)}, {${s.chars.offset}, ${s.chars.length}}},\n`;
output += `};\n\n`;

output += `inline constexpr DbcsSpec DBCS_SPECS[] = {\n`;
for (const s of dbcsSpecs) {
  output += `    {${cxxString(s.name)}, ${s.chunksOffset}, ${s.chunksCount}, ${s.skipOffset}, ${s.skipCount}, ${s.addOffset}, ${s.addCount}, ${s.gbOffset}, ${s.gbCount}},\n`;
}
output += `};\n\n`;

output += `inline constexpr EncodingEntry ENCODING_ENTRIES[] = {\n`;
for (const e of entries) {
  output += `    {${cxxString(e.name)}, GeneratedKind::${e.kind}, ${e.index}, ${cxxString(e.target)}},\n`;
}
output += `};\n\n`;

output += `}  // namespace polycpp::iconv_lite::generated\n`;

fs.mkdirSync(path.dirname(outFile), { recursive: true });
fs.writeFileSync(outFile, output);
console.error(`generated ${outFile}`);
console.error(`${u16Data.length} UTF-16 units, ${sbcsSpecs.length} SBCS specs, ${dbcsSpecs.length} DBCS specs, ${entries.length} entries`);
