#include <polycpp/iconv_lite/iconv_lite.hpp>

#include <polycpp/iconv_lite/detail/generated_tables.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace polycpp::iconv_lite {
namespace {

constexpr int32_t UNASSIGNED = -1;
constexpr int32_t GB18030_CODE = -2;
constexpr int32_t SEQ_START = -10;
constexpr int32_t NODE_START = -1000;
constexpr int32_t DEF_CHAR = -1;

using ByteNode = std::array<int32_t, 256>;

ByteNode make_unassigned_node() {
    ByteNode node{};
    node.fill(UNASSIGNED);
    return node;
}

template <typename T, size_t N>
constexpr size_t array_size(const T (&)[N]) noexcept {
    return N;
}

struct U16SliceView {
    const uint16_t* data = nullptr;
    size_t size = 0;
};

U16SliceView u16_slice(generated::U16Slice slice) {
    return {generated::U16_DATA + slice.offset, slice.length};
}

void append_utf8(std::string& out, uint32_t cp) {
    if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
        cp = 0xFFFD;
    }
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

void append_utf16(std::vector<uint16_t>& out, uint32_t cp) {
    if (cp > 0x10FFFF) cp = 0xFFFD;
    if (cp <= 0xFFFF) {
        out.push_back(static_cast<uint16_t>(cp));
    } else {
        cp -= 0x10000;
        out.push_back(static_cast<uint16_t>(0xD800 | (cp >> 10)));
        out.push_back(static_cast<uint16_t>(0xDC00 | (cp & 0x3FF)));
    }
}

std::vector<uint16_t> utf8_to_utf16(std::string_view input) {
    std::vector<uint16_t> out;
    out.reserve(input.size());

    size_t i = 0;
    while (i < input.size()) {
        const auto b0 = static_cast<uint8_t>(input[i]);
        uint32_t cp = 0xFFFD;
        size_t len = 1;

        if (b0 <= 0x7F) {
            cp = b0;
        } else if ((b0 & 0xE0) == 0xC0) {
            len = 2;
            if (i + len <= input.size()) {
                const auto b1 = static_cast<uint8_t>(input[i + 1]);
                if ((b1 & 0xC0) == 0x80) {
                    cp = ((b0 & 0x1F) << 6) | (b1 & 0x3F);
                    if (cp < 0x80) cp = 0xFFFD;
                }
            }
        } else if ((b0 & 0xF0) == 0xE0) {
            len = 3;
            if (i + len <= input.size()) {
                const auto b1 = static_cast<uint8_t>(input[i + 1]);
                const auto b2 = static_cast<uint8_t>(input[i + 2]);
                if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80) {
                    cp = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
                    if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) cp = 0xFFFD;
                }
            }
        } else if ((b0 & 0xF8) == 0xF0) {
            len = 4;
            if (i + len <= input.size()) {
                const auto b1 = static_cast<uint8_t>(input[i + 1]);
                const auto b2 = static_cast<uint8_t>(input[i + 2]);
                const auto b3 = static_cast<uint8_t>(input[i + 3]);
                if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80 && (b3 & 0xC0) == 0x80) {
                    cp = ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) |
                         ((b2 & 0x3F) << 6) | (b3 & 0x3F);
                    if (cp < 0x10000 || cp > 0x10FFFF) cp = 0xFFFD;
                }
            }
        }

        append_utf16(out, cp);
        i += len;
    }
    return out;
}

std::string utf16_to_utf8(const std::vector<uint16_t>& input) {
    std::string out;
    out.reserve(input.size() * 2);
    for (size_t i = 0; i < input.size(); ++i) {
        uint32_t cp = input[i];
        if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < input.size()) {
            const uint32_t lo = input[i + 1];
            if (lo >= 0xDC00 && lo <= 0xDFFF) {
                cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                ++i;
            }
        }
        append_utf8(out, cp);
    }
    return out;
}

std::string decode_utf8_bytes(const uint8_t* data, size_t size) {
    return utf16_to_utf8(utf8_to_utf16(std::string_view(reinterpret_cast<const char*>(data), size)));
}

Buffer buffer_from_bytes(const std::vector<uint8_t>& bytes) {
    return Buffer::from(bytes);
}

std::vector<uint8_t> encode_utf16le(std::string_view input) {
    auto units = utf8_to_utf16(input);
    std::vector<uint8_t> bytes;
    bytes.reserve(units.size() * 2);
    for (auto unit : units) {
        bytes.push_back(static_cast<uint8_t>(unit & 0xFF));
        bytes.push_back(static_cast<uint8_t>(unit >> 8));
    }
    return bytes;
}

std::vector<uint8_t> encode_utf16be(std::string_view input) {
    auto units = utf8_to_utf16(input);
    std::vector<uint8_t> bytes;
    bytes.reserve(units.size() * 2);
    for (auto unit : units) {
        bytes.push_back(static_cast<uint8_t>(unit >> 8));
        bytes.push_back(static_cast<uint8_t>(unit & 0xFF));
    }
    return bytes;
}

std::string decode_utf16_bytes(const uint8_t* data, size_t size, bool little_endian) {
    std::vector<uint16_t> units;
    units.reserve(size / 2);
    for (size_t i = 0; i + 1 < size; i += 2) {
        uint16_t unit = little_endian
            ? static_cast<uint16_t>(data[i] | (data[i + 1] << 8))
            : static_cast<uint16_t>((data[i] << 8) | data[i + 1]);
        units.push_back(unit);
    }
    return utf16_to_utf8(units);
}

std::vector<uint8_t> encode_utf32(std::string_view input, bool little_endian) {
    auto units = utf8_to_utf16(input);
    std::vector<uint8_t> bytes;
    bytes.reserve(units.size() * 4);
    for (size_t i = 0; i < units.size(); ++i) {
        uint32_t cp = units[i];
        if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < units.size() && units[i + 1] >= 0xDC00 && units[i + 1] <= 0xDFFF) {
            cp = 0x10000 + ((cp - 0xD800) << 10) + (units[i + 1] - 0xDC00);
            ++i;
        }
        if (little_endian) {
            bytes.push_back(static_cast<uint8_t>(cp & 0xFF));
            bytes.push_back(static_cast<uint8_t>((cp >> 8) & 0xFF));
            bytes.push_back(static_cast<uint8_t>((cp >> 16) & 0xFF));
            bytes.push_back(static_cast<uint8_t>((cp >> 24) & 0xFF));
        } else {
            bytes.push_back(static_cast<uint8_t>((cp >> 24) & 0xFF));
            bytes.push_back(static_cast<uint8_t>((cp >> 16) & 0xFF));
            bytes.push_back(static_cast<uint8_t>((cp >> 8) & 0xFF));
            bytes.push_back(static_cast<uint8_t>(cp & 0xFF));
        }
    }
    return bytes;
}

std::string decode_utf32_bytes(const uint8_t* data, size_t size, bool little_endian) {
    std::vector<uint16_t> units;
    units.reserve(size / 2);
    for (size_t i = 0; i + 3 < size; i += 4) {
        uint32_t cp = little_endian
            ? static_cast<uint32_t>(data[i]) | (static_cast<uint32_t>(data[i + 1]) << 8) |
                  (static_cast<uint32_t>(data[i + 2]) << 16) | (static_cast<uint32_t>(data[i + 3]) << 24)
            : (static_cast<uint32_t>(data[i]) << 24) | (static_cast<uint32_t>(data[i + 1]) << 16) |
                  (static_cast<uint32_t>(data[i + 2]) << 8) | static_cast<uint32_t>(data[i + 3]);
        if (cp > 0x10FFFF) cp = 0xFFFD;
        append_utf16(units, cp);
    }
    return utf16_to_utf8(units);
}

std::vector<uint8_t> encode_cesu8(std::string_view input) {
    auto units = utf8_to_utf16(input);
    std::vector<uint8_t> bytes;
    bytes.reserve(units.size() * 3);
    for (uint16_t unit : units) {
        if (unit < 0x80) {
            bytes.push_back(static_cast<uint8_t>(unit));
        } else if (unit < 0x800) {
            bytes.push_back(static_cast<uint8_t>(0xC0 | (unit >> 6)));
            bytes.push_back(static_cast<uint8_t>(0x80 | (unit & 0x3F)));
        } else {
            bytes.push_back(static_cast<uint8_t>(0xE0 | (unit >> 12)));
            bytes.push_back(static_cast<uint8_t>(0x80 | ((unit >> 6) & 0x3F)));
            bytes.push_back(static_cast<uint8_t>(0x80 | (unit & 0x3F)));
        }
    }
    return bytes;
}

std::string decode_cesu8(const uint8_t* data, size_t size) {
    std::vector<uint16_t> units;
    uint32_t acc = 0;
    int cont = 0;
    int acc_bytes = 0;
    for (size_t i = 0; i < size; ++i) {
        const uint8_t b = data[i];
        if ((b & 0xC0) != 0x80) {
            if (cont > 0) {
                units.push_back(0xFFFD);
                cont = 0;
            }
            if (b < 0x80) {
                units.push_back(b);
            } else if (b < 0xE0) {
                acc = b & 0x1F;
                cont = 1;
                acc_bytes = 1;
            } else if (b < 0xF0) {
                acc = b & 0x0F;
                cont = 2;
                acc_bytes = 1;
            } else {
                units.push_back(0xFFFD);
            }
        } else if (cont > 0) {
            acc = (acc << 6) | (b & 0x3F);
            --cont;
            ++acc_bytes;
            if (cont == 0) {
                if ((acc_bytes == 2 && acc < 0x80 && acc > 0) || (acc_bytes == 3 && acc < 0x800)) {
                    units.push_back(0xFFFD);
                } else {
                    units.push_back(static_cast<uint16_t>(acc));
                }
            }
        } else {
            units.push_back(0xFFFD);
        }
    }
    if (cont > 0) units.push_back(0xFFFD);
    return utf16_to_utf8(units);
}

class SbcsCodec {
public:
    explicit SbcsCodec(const generated::SbcsSpec& spec) {
        decode_.fill(0xFFFD);
        auto chars = u16_slice(spec.chars);
        size_t dst = 0;
        if (chars.size == 128) {
            for (; dst < 128; ++dst) decode_[dst] = static_cast<uint32_t>(dst);
        }
        for (size_t i = 0; i < chars.size && dst < 256; ++i, ++dst) {
            decode_[dst] = chars.data[i];
        }
        encode_.reserve(256);
        for (size_t i = 0; i < 256; ++i) {
            encode_[decode_[i]] = static_cast<uint8_t>(i);
        }
    }

    Buffer encode(std::string_view input) const {
        auto units = utf8_to_utf16(input);
        std::vector<uint8_t> bytes;
        bytes.reserve(units.size());
        for (auto unit : units) {
            auto it = encode_.find(unit);
            bytes.push_back(it == encode_.end() ? static_cast<uint8_t>('?') : it->second);
        }
        return buffer_from_bytes(bytes);
    }

    std::string decode(const Buffer& input) const {
        std::vector<uint16_t> units;
        units.reserve(input.length());
        for (size_t i = 0; i < input.length(); ++i) {
            units.push_back(static_cast<uint16_t>(decode_[input[i]]));
        }
        return utf16_to_utf8(units);
    }

private:
    std::array<uint32_t, 256> decode_{};
    std::unordered_map<uint32_t, uint8_t> encode_;
};

struct SeqNode {
    std::optional<int32_t> def;
    std::unordered_map<uint32_t, int32_t> values;
    std::unordered_map<uint32_t, uint32_t> children;
};

class DbcsCodec {
public:
    explicit DbcsCodec(const generated::DbcsSpec& spec) : spec_(spec) {
        decode_tables_.push_back(make_unassigned_node());
        for (size_t i = 0; i < spec.chunks_count; ++i) {
            add_decode_chunk(generated::CHUNKS[spec.chunks_offset + i]);
        }
        if (has_gb18030()) add_gb18030_nodes();
        fill_encode_table(0, 0);
        for (size_t i = 0; i < spec.add_count; ++i) {
            const auto& add = generated::ADD_MAPPINGS[spec.add_offset + i];
            set_encode_char(add.code, static_cast<int32_t>(add.value));
        }
        def_char_sb_ = lookup_encode_char('?').value_or(static_cast<int32_t>('?'));
    }

    Buffer encode(std::string_view input) const {
        auto units = utf8_to_utf16(input);
        std::vector<uint8_t> out;
        out.reserve(units.size() * (has_gb18030() ? 4 : 3));

        int32_t lead_surrogate = -1;
        std::optional<uint32_t> seq_idx;
        int32_t next_char = -1;
        size_t i = 0;

        while (true) {
            int32_t u_code = 0;
            if (next_char == -1) {
                if (i == units.size()) break;
                u_code = units[i++];
            } else {
                u_code = next_char;
                next_char = -1;
            }

            if (u_code >= 0xD800 && u_code < 0xE000) {
                if (u_code < 0xDC00) {
                    if (lead_surrogate == -1) {
                        lead_surrogate = u_code;
                        continue;
                    }
                    lead_surrogate = u_code;
                    u_code = UNASSIGNED;
                } else if (lead_surrogate != -1) {
                    u_code = 0x10000 + (lead_surrogate - 0xD800) * 0x400 + (u_code - 0xDC00);
                    lead_surrogate = -1;
                } else {
                    u_code = UNASSIGNED;
                }
            } else if (lead_surrogate != -1) {
                next_char = u_code;
                u_code = UNASSIGNED;
                lead_surrogate = -1;
            }

            int32_t dbcs_code = UNASSIGNED;
            if (seq_idx.has_value() && u_code != UNASSIGNED) {
                const auto& node = encode_table_seq_[*seq_idx];
                auto child = node.children.find(static_cast<uint32_t>(u_code));
                if (child != node.children.end()) {
                    seq_idx = child->second;
                    continue;
                }
                auto value = node.values.find(static_cast<uint32_t>(u_code));
                if (value != node.values.end()) {
                    dbcs_code = value->second;
                } else if (node.def.has_value()) {
                    dbcs_code = *node.def;
                    next_char = u_code;
                }
                seq_idx.reset();
            } else if (u_code >= 0) {
                dbcs_code = lookup_encode_char(static_cast<uint32_t>(u_code)).value_or(UNASSIGNED);
                if (dbcs_code <= SEQ_START) {
                    seq_idx = static_cast<uint32_t>(SEQ_START - dbcs_code);
                    continue;
                }
                if (dbcs_code == UNASSIGNED && has_gb18030()) {
                    const auto idx = find_gb_by_unicode(static_cast<uint32_t>(u_code));
                    if (idx >= 0) {
                        uint32_t code = generated::GB_RANGES[spec_.gb_offset + idx].gb_char +
                                        (static_cast<uint32_t>(u_code) - generated::GB_RANGES[spec_.gb_offset + idx].u_char);
                        out.push_back(static_cast<uint8_t>(0x81 + code / 12600));
                        code %= 12600;
                        out.push_back(static_cast<uint8_t>(0x30 + code / 1260));
                        code %= 1260;
                        out.push_back(static_cast<uint8_t>(0x81 + code / 10));
                        out.push_back(static_cast<uint8_t>(0x30 + code % 10));
                        continue;
                    }
                }
            }

            if (dbcs_code == UNASSIGNED) dbcs_code = def_char_sb_;
            write_encoded_code(out, static_cast<uint32_t>(dbcs_code));
        }

        if (seq_idx.has_value()) {
            const auto& node = encode_table_seq_[*seq_idx];
            if (node.def.has_value()) write_encoded_code(out, static_cast<uint32_t>(*node.def));
        }
        if (lead_surrogate != -1) write_encoded_code(out, static_cast<uint32_t>(def_char_sb_));

        return buffer_from_bytes(out);
    }

    std::string decode(const Buffer& input) const {
        std::vector<uint8_t> bytes(input.data(), input.data() + input.length());
        std::vector<uint16_t> units;
        while (true) {
            auto result = decode_once(bytes);
            units.insert(units.end(), result.units.begin(), result.units.end());
            if (result.leftover.empty()) break;
            units.push_back(0xFFFD);
            bytes.assign(result.leftover.begin() + 1, result.leftover.end());
            if (bytes.empty()) break;
        }
        return utf16_to_utf8(units);
    }

private:
    struct DecodeResult {
        std::vector<uint16_t> units;
        std::vector<uint8_t> leftover;
    };

    const generated::DbcsSpec& spec_;
    std::vector<ByteNode> decode_tables_;
    std::vector<std::vector<uint32_t>> decode_table_seq_;
    std::unordered_map<uint32_t, ByteNode> encode_table_;
    std::vector<SeqNode> encode_table_seq_;
    int32_t def_char_sb_ = '?';

    bool has_gb18030() const noexcept { return spec_.gb_count > 0; }

    ByteNode& get_decode_trie_node(uint32_t addr) {
        std::vector<uint8_t> bytes;
        for (; addr > 0; addr >>= 8) bytes.push_back(static_cast<uint8_t>(addr & 0xFF));
        if (bytes.empty()) bytes.push_back(0);

        ByteNode* node = &decode_tables_[0];
        for (size_t idx = bytes.size() - 1; idx > 0; --idx) {
            const uint8_t b = bytes[idx];
            const int32_t val = (*node)[b];
            if (val == UNASSIGNED) {
                (*node)[b] = NODE_START - static_cast<int32_t>(decode_tables_.size());
                decode_tables_.push_back(make_unassigned_node());
                node = &decode_tables_.back();
            } else if (val <= NODE_START) {
                node = &decode_tables_[NODE_START - val];
            } else {
                throw std::runtime_error("iconv-lite table conflict");
            }
        }
        return *node;
    }

    void add_decode_chunk(const generated::Chunk& chunk) {
        auto& table = get_decode_trie_node(chunk.start);
        uint32_t cur_addr = chunk.start & 0xFF;
        for (size_t part_idx = 0; part_idx < chunk.parts_count; ++part_idx) {
            const auto& part = generated::CHUNK_PARTS[chunk.parts_offset + part_idx];
            if (part.is_number) {
                int32_t char_code = table[cur_addr - 1] + 1;
                for (int32_t i = 0; i < part.number; ++i) table[cur_addr++] = char_code++;
                continue;
            }

            auto text = u16_slice(part.text);
            for (size_t i = 0; i < text.size;) {
                uint32_t code = text.data[i++];
                if (code >= 0xD800 && code < 0xDC00) {
                    if (i >= text.size) throw std::runtime_error("invalid surrogate in iconv-lite table");
                    const uint32_t trail = text.data[i++];
                    if (trail < 0xDC00 || trail >= 0xE000) throw std::runtime_error("invalid surrogate in iconv-lite table");
                    table[cur_addr++] = static_cast<int32_t>(0x10000 + (code - 0xD800) * 0x400 + (trail - 0xDC00));
                } else if (code > 0x0FF0 && code <= 0x0FFF) {
                    const size_t len = 0xFFF - code + 2;
                    std::vector<uint32_t> seq;
                    seq.reserve(len);
                    for (size_t j = 0; j < len; ++j) seq.push_back(text.data[i++]);
                    table[cur_addr++] = SEQ_START - static_cast<int32_t>(decode_table_seq_.size());
                    decode_table_seq_.push_back(std::move(seq));
                } else {
                    table[cur_addr++] = static_cast<int32_t>(code);
                }
            }
        }
        if (cur_addr > 0xFF) throw std::runtime_error("invalid iconv-lite chunk length");
    }

    void add_gb18030_nodes() {
        const auto third_idx = static_cast<int32_t>(decode_tables_.size());
        decode_tables_.push_back(make_unassigned_node());
        const auto fourth_idx = static_cast<int32_t>(decode_tables_.size());
        decode_tables_.push_back(make_unassigned_node());

        auto& first = decode_tables_[0];
        for (int i = 0x81; i <= 0xFE; ++i) {
            auto& second = decode_tables_[NODE_START - first[i]];
            for (int j = 0x30; j <= 0x39; ++j) {
                if (second[j] == UNASSIGNED) second[j] = NODE_START - third_idx;
                auto& third = decode_tables_[NODE_START - second[j]];
                for (int k = 0x81; k <= 0xFE; ++k) {
                    if (third[k] == UNASSIGNED) third[k] = NODE_START - fourth_idx;
                    auto& fourth = decode_tables_[NODE_START - third[k]];
                    for (int l = 0x30; l <= 0x39; ++l) {
                        if (fourth[l] == UNASSIGNED) fourth[l] = GB18030_CODE;
                    }
                }
            }
        }
    }

    bool skip_encode(uint32_t mb_code) const {
        for (size_t i = 0; i < spec_.skip_count; ++i) {
            const auto& range = generated::SKIP_RANGES[spec_.skip_offset + i];
            if (mb_code >= range.from && mb_code <= range.to) return true;
        }
        return false;
    }

    ByteNode& get_encode_bucket(uint32_t u_code) {
        const uint32_t high = u_code >> 8;
        auto it = encode_table_.find(high);
        if (it == encode_table_.end()) {
            it = encode_table_.emplace(high, make_unassigned_node()).first;
        }
        return it->second;
    }

    std::optional<int32_t> lookup_encode_char(uint32_t u_code) const {
        auto it = encode_table_.find(u_code >> 8);
        if (it == encode_table_.end()) return std::nullopt;
        return it->second[u_code & 0xFF];
    }

    void set_encode_char(uint32_t u_code, int32_t dbcs_code) {
        auto& bucket = get_encode_bucket(u_code);
        auto& slot = bucket[u_code & 0xFF];
        if (slot <= SEQ_START) {
            encode_table_seq_[SEQ_START - slot].def = dbcs_code;
        } else if (slot == UNASSIGNED) {
            slot = dbcs_code;
        }
    }

    void set_encode_sequence(const std::vector<uint32_t>& seq, int32_t dbcs_code) {
        if (seq.empty()) return;
        auto& bucket = get_encode_bucket(seq[0]);
        auto& slot = bucket[seq[0] & 0xFF];
        uint32_t node_idx = 0;
        if (slot <= SEQ_START) {
            node_idx = static_cast<uint32_t>(SEQ_START - slot);
        } else {
            SeqNode node;
            if (slot != UNASSIGNED) node.def = slot;
            node_idx = static_cast<uint32_t>(encode_table_seq_.size());
            encode_table_seq_.push_back(std::move(node));
            slot = SEQ_START - static_cast<int32_t>(node_idx);
        }

        for (size_t i = 1; i + 1 < seq.size(); ++i) {
            auto& node = encode_table_seq_[node_idx];
            auto child = node.children.find(seq[i]);
            if (child == node.children.end()) {
                const auto new_idx = static_cast<uint32_t>(encode_table_seq_.size());
                encode_table_seq_.push_back(SeqNode{});
                child = node.children.emplace(seq[i], new_idx).first;
            }
            node_idx = child->second;
        }
        encode_table_seq_[node_idx].values[seq.back()] = dbcs_code;
    }

    bool fill_encode_table(size_t node_idx, uint32_t prefix) {
        const auto& node = decode_tables_[node_idx];
        bool has_values = false;
        std::unordered_map<size_t, bool> subnode_empty;
        for (uint32_t i = 0; i < 0x100; ++i) {
            const int32_t u_code = node[i];
            const uint32_t mb_code = prefix + i;
            if (skip_encode(mb_code)) continue;

            if (u_code >= 0) {
                set_encode_char(static_cast<uint32_t>(u_code), static_cast<int32_t>(mb_code));
                has_values = true;
            } else if (u_code <= NODE_START) {
                const size_t sub_idx = static_cast<size_t>(NODE_START - u_code);
                if (!subnode_empty[sub_idx]) {
                    const uint32_t new_prefix = mb_code << 8;
                    if (fill_encode_table(sub_idx, new_prefix)) has_values = true;
                    else subnode_empty[sub_idx] = true;
                }
            } else if (u_code <= SEQ_START) {
                set_encode_sequence(decode_table_seq_[SEQ_START - u_code], static_cast<int32_t>(mb_code));
                has_values = true;
            }
        }
        return has_values;
    }

    int32_t find_gb_by_unicode(uint32_t value) const {
        if (!has_gb18030() || generated::GB_RANGES[spec_.gb_offset].u_char > value) return -1;
        int32_t left = 0;
        int32_t right = static_cast<int32_t>(spec_.gb_count);
        while (left < right - 1) {
            const int32_t mid = left + ((right - left + 1) >> 1);
            if (generated::GB_RANGES[spec_.gb_offset + mid].u_char <= value) left = mid;
            else right = mid;
        }
        return left;
    }

    int32_t find_gb_by_pointer(uint32_t value) const {
        if (!has_gb18030() || generated::GB_RANGES[spec_.gb_offset].gb_char > value) return -1;
        int32_t left = 0;
        int32_t right = static_cast<int32_t>(spec_.gb_count);
        while (left < right - 1) {
            const int32_t mid = left + ((right - left + 1) >> 1);
            if (generated::GB_RANGES[spec_.gb_offset + mid].gb_char <= value) left = mid;
            else right = mid;
        }
        return left;
    }

    static void write_encoded_code(std::vector<uint8_t>& out, uint32_t code) {
        if (code < 0x100) {
            out.push_back(static_cast<uint8_t>(code));
        } else if (code < 0x10000) {
            out.push_back(static_cast<uint8_t>(code >> 8));
            out.push_back(static_cast<uint8_t>(code & 0xFF));
        } else if (code < 0x1000000) {
            out.push_back(static_cast<uint8_t>(code >> 16));
            out.push_back(static_cast<uint8_t>((code >> 8) & 0xFF));
            out.push_back(static_cast<uint8_t>(code & 0xFF));
        } else {
            out.push_back(static_cast<uint8_t>(code >> 24));
            out.push_back(static_cast<uint8_t>((code >> 16) & 0xFF));
            out.push_back(static_cast<uint8_t>((code >> 8) & 0xFF));
            out.push_back(static_cast<uint8_t>(code & 0xFF));
        }
    }

    void append_decoded_code(std::vector<uint16_t>& out, uint32_t u_code) const {
        append_utf16(out, u_code);
    }

    DecodeResult decode_once(const std::vector<uint8_t>& input) const {
        DecodeResult result;
        size_t node_idx = 0;
        size_t seq_start = 0;

        for (size_t pos = 0; pos < input.size(); ++pos) {
            const uint8_t cur = input[pos];
            int32_t u_code = decode_tables_[node_idx][cur];

            if (u_code >= 0) {
            } else if (u_code == UNASSIGNED) {
                u_code = 0xFFFD;
                pos = seq_start;
            } else if (u_code == GB18030_CODE) {
                if (pos < 3) {
                    u_code = 0xFFFD;
                    pos = seq_start;
                } else {
                    const uint32_t ptr = (input[pos - 3] - 0x81) * 12600 +
                                         (input[pos - 2] - 0x30) * 1260 +
                                         (input[pos - 1] - 0x81) * 10 +
                                         (cur - 0x30);
                    const auto idx = find_gb_by_pointer(ptr);
                    u_code = idx >= 0
                        ? static_cast<int32_t>(generated::GB_RANGES[spec_.gb_offset + idx].u_char +
                                               ptr - generated::GB_RANGES[spec_.gb_offset + idx].gb_char)
                        : 0xFFFD;
                }
            } else if (u_code <= NODE_START) {
                node_idx = static_cast<size_t>(NODE_START - u_code);
                continue;
            } else if (u_code <= SEQ_START) {
                const auto& seq = decode_table_seq_[SEQ_START - u_code];
                for (size_t i = 0; i + 1 < seq.size(); ++i) append_decoded_code(result.units, seq[i]);
                u_code = static_cast<int32_t>(seq.back());
            } else {
                throw std::runtime_error("invalid iconv-lite decode table value");
            }

            append_decoded_code(result.units, static_cast<uint32_t>(u_code));
            node_idx = 0;
            seq_start = pos + 1;
        }

        if (seq_start < input.size()) {
            result.leftover.assign(input.begin() + static_cast<std::ptrdiff_t>(seq_start), input.end());
        }
        return result;
    }
};

const SbcsCodec& get_sbcs(size_t index) {
    static std::mutex mutex;
    static std::unordered_map<size_t, std::shared_ptr<SbcsCodec>> cache;
    std::lock_guard<std::mutex> lock(mutex);
    auto it = cache.find(index);
    if (it == cache.end()) {
        it = cache.emplace(index, std::make_shared<SbcsCodec>(generated::SBCS_SPECS[index])).first;
    }
    return *it->second;
}

const DbcsCodec& get_dbcs(size_t index) {
    static std::mutex mutex;
    static std::unordered_map<size_t, std::shared_ptr<DbcsCodec>> cache;
    std::lock_guard<std::mutex> lock(mutex);
    auto it = cache.find(index);
    if (it == cache.end()) {
        it = cache.emplace(index, std::make_shared<DbcsCodec>(generated::DBCS_SPECS[index])).first;
    }
    return *it->second;
}

enum class EncodingKind {
    utf8,
    cesu8,
    binary,
    base64,
    hex,
    utf16le,
    utf16be,
    utf16_auto,
    utf32le,
    utf32be,
    utf32_auto,
    utf7,
    utf7_imap,
    sbcs,
    dbcs,
};

struct ResolvedEncoding {
    std::string canonical;
    std::string converter;
    EncodingKind kind = EncodingKind::utf8;
    size_t spec_index = 0;
    bool bom_aware = false;
};

std::optional<ResolvedEncoding> resolve_manual(std::string_view canonical) {
    if (canonical == "utf8" || canonical == "unicode11utf8") return ResolvedEncoding{std::string(canonical), "utf8", EncodingKind::utf8, 0, true};
    if (canonical == "cesu8") return ResolvedEncoding{std::string(canonical), "cesu8", EncodingKind::cesu8, 0, true};
    if (canonical == "binary") return ResolvedEncoding{std::string(canonical), "binary", EncodingKind::binary, 0, false};
    if (canonical == "base64") return ResolvedEncoding{std::string(canonical), "base64", EncodingKind::base64, 0, false};
    if (canonical == "hex") return ResolvedEncoding{std::string(canonical), "hex", EncodingKind::hex, 0, false};
    if (canonical == "ucs2" || canonical == "utf16le") return ResolvedEncoding{std::string(canonical), "utf16le", EncodingKind::utf16le, 0, true};
    if (canonical == "utf16be") return ResolvedEncoding{std::string(canonical), "utf16be", EncodingKind::utf16be, 0, true};
    if (canonical == "utf16") return ResolvedEncoding{std::string(canonical), "utf16", EncodingKind::utf16_auto, 0, true};
    if (canonical == "utf32le" || canonical == "ucs4le") return ResolvedEncoding{std::string(canonical), "utf32le", EncodingKind::utf32le, 0, true};
    if (canonical == "utf32be" || canonical == "ucs4be") return ResolvedEncoding{std::string(canonical), "utf32be", EncodingKind::utf32be, 0, true};
    if (canonical == "utf32" || canonical == "ucs4") return ResolvedEncoding{std::string(canonical), "utf32", EncodingKind::utf32_auto, 0, true};
    if (canonical == "utf7" || canonical == "unicode11utf7") return ResolvedEncoding{std::string(canonical), "utf7", EncodingKind::utf7, 0, true};
    if (canonical == "utf7imap") return ResolvedEncoding{std::string(canonical), "utf7imap", EncodingKind::utf7_imap, 0, true};
    return std::nullopt;
}

const generated::EncodingEntry* find_generated_entry(std::string_view name) {
    auto first = std::begin(generated::ENCODING_ENTRIES);
    auto last = std::end(generated::ENCODING_ENTRIES);
    auto it = std::lower_bound(first, last, name, [](const generated::EncodingEntry& entry, std::string_view value) {
        return entry.name < value;
    });
    if (it != last && it->name == name) return &*it;
    return nullptr;
}

ResolvedEncoding resolve_encoding(std::string_view encoding) {
    auto current = canonicalize_encoding(encoding);
    for (int depth = 0; depth < 32; ++depth) {
        if (auto manual = resolve_manual(current)) return *manual;
        const auto* entry = find_generated_entry(current);
        if (entry == nullptr) break;
        if (entry->kind == generated::GeneratedKind::alias) {
            current = std::string(entry->target);
            continue;
        }
        if (entry->kind == generated::GeneratedKind::sbcs) {
            return ResolvedEncoding{current, std::string(generated::SBCS_SPECS[entry->index].name), EncodingKind::sbcs, entry->index, false};
        }
        if (entry->kind == generated::GeneratedKind::dbcs) {
            return ResolvedEncoding{current, std::string(generated::DBCS_SPECS[entry->index].name), EncodingKind::dbcs, entry->index, false};
        }
    }
    throw polycpp::TypeError(
        "Encoding not recognized: '" + std::string(encoding) +
        "' (searched as: '" + canonicalize_encoding(encoding) + "')")
        .setCode("ERR_ENCODING_NOT_SUPPORTED");
}

std::string strip_utf8_bom(std::string value) {
    if (value.size() >= 3 &&
        static_cast<uint8_t>(value[0]) == 0xEF &&
        static_cast<uint8_t>(value[1]) == 0xBB &&
        static_cast<uint8_t>(value[2]) == 0xBF) {
        value.erase(0, 3);
    }
    return value;
}

std::string normalize_default_encoding(std::string_view value, std::string_view fallback, std::string_view expected_auto) {
    if (value.empty()) return std::string(fallback);
    const auto canonical = canonicalize_encoding(value);
    if (expected_auto == "utf16") {
        if (canonical == "utf16be") return "utf16be";
        if (canonical == "utf16le" || canonical == "ucs2" || canonical == "utf16") return "utf16le";
    } else if (expected_auto == "utf32") {
        if (canonical == "utf32be" || canonical == "ucs4be") return "utf32be";
        if (canonical == "utf32le" || canonical == "ucs4le" || canonical == "utf32" || canonical == "ucs4") return "utf32le";
    }
    return std::string(fallback);
}

std::string choose_utf16_decode(const Buffer& input, const DecodeOptions& options) {
    if (input.length() >= 2) {
        if (input[0] == 0xFF && input[1] == 0xFE) return "utf16le";
        if (input[0] == 0xFE && input[1] == 0xFF) return "utf16be";
    }
    size_t be = 0;
    size_t le = 0;
    const auto limit = std::min(input.length(), static_cast<size_t>(200));
    for (size_t i = 0; i + 1 < limit; i += 2) {
        if (input[i] == 0 && input[i + 1] != 0) ++be;
        if (input[i] != 0 && input[i + 1] == 0) ++le;
    }
    if (be > le) return "utf16be";
    if (le > be) return "utf16le";
    return normalize_default_encoding(options.default_encoding, "utf16le", "utf16");
}

std::string choose_utf32_decode(const Buffer& input, const DecodeOptions& options) {
    if (input.length() >= 4) {
        if (input[0] == 0xFF && input[1] == 0xFE && input[2] == 0 && input[3] == 0) return "utf32le";
        if (input[0] == 0 && input[1] == 0 && input[2] == 0xFE && input[3] == 0xFF) return "utf32be";
    }
    int64_t invalid_le = 0;
    int64_t invalid_be = 0;
    int64_t bmp_le = 0;
    int64_t bmp_be = 0;
    const auto limit = std::min(input.length(), static_cast<size_t>(400));
    for (size_t i = 0; i + 3 < limit; i += 4) {
        const uint8_t b0 = input[i];
        const uint8_t b1 = input[i + 1];
        const uint8_t b2 = input[i + 2];
        const uint8_t b3 = input[i + 3];
        if (b0 != 0 || b1 > 0x10) ++invalid_be;
        if (b3 != 0 || b2 > 0x10) ++invalid_le;
        if (b0 == 0 && b1 == 0 && (b2 != 0 || b3 != 0)) ++bmp_be;
        if ((b0 != 0 || b1 != 0) && b2 == 0 && b3 == 0) ++bmp_le;
    }
    if (bmp_be - invalid_be > bmp_le - invalid_le) return "utf32be";
    if (bmp_le - invalid_le > bmp_be - invalid_be) return "utf32le";
    return normalize_default_encoding(options.default_encoding, "utf32le", "utf32");
}

void prepend_bytes(std::vector<uint8_t>& bytes, std::initializer_list<uint8_t> prefix) {
    bytes.insert(bytes.begin(), prefix.begin(), prefix.end());
}

Buffer with_bom(std::vector<uint8_t> bytes, const ResolvedEncoding& resolved, const EncodeOptions& options) {
    bool add = options.add_bom.value_or(resolved.kind == EncodingKind::utf16_auto || resolved.kind == EncodingKind::utf32_auto);
    if (!resolved.bom_aware || !add) return buffer_from_bytes(bytes);

    switch (resolved.kind) {
        case EncodingKind::utf8:
        case EncodingKind::cesu8:
            prepend_bytes(bytes, {0xEF, 0xBB, 0xBF});
            break;
        case EncodingKind::utf16le:
        case EncodingKind::utf16_auto:
            prepend_bytes(bytes, {0xFF, 0xFE});
            break;
        case EncodingKind::utf16be:
            prepend_bytes(bytes, {0xFE, 0xFF});
            break;
        case EncodingKind::utf32le:
        case EncodingKind::utf32_auto:
            prepend_bytes(bytes, {0xFF, 0xFE, 0x00, 0x00});
            break;
        case EncodingKind::utf32be:
            prepend_bytes(bytes, {0x00, 0x00, 0xFE, 0xFF});
            break;
        default:
            break;
    }
    return buffer_from_bytes(bytes);
}

bool is_utf7_direct(uint8_t c) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
           c == '\'' || c == '(' || c == ')' || c == ',' || c == '-' || c == '.' || c == '/' ||
           c == ':' || c == '?' || c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

bool is_utf7_imap_direct(uint8_t c) noexcept { return c >= 0x20 && c <= 0x7E && c != '&'; }

bool is_base64_char(uint8_t c, bool imap) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
           c == '+' || c == '/' || (imap && c == ',');
}

std::string trim_base64_padding(std::string value) {
    while (!value.empty() && value.back() == '=') value.pop_back();
    return value;
}

std::string maybe_add_utf8_bom(std::string_view input, const ResolvedEncoding& resolved, const EncodeOptions& options) {
    if (resolved.bom_aware && options.add_bom.value_or(false)) return std::string("\xEF\xBB\xBF", 3) + std::string(input);
    return std::string(input);
}

Buffer encode_utf7_like(std::string_view raw_input, bool imap) {
    std::string input(raw_input);
    std::string output;
    output.reserve(input.size() + 8);
    size_t i = 0;
    while (i < input.size()) {
        const auto byte = static_cast<uint8_t>(input[i]);
        const bool direct = imap ? is_utf7_imap_direct(byte) : is_utf7_direct(byte);
        if (direct) {
            output.push_back(input[i++]);
            continue;
        }
        const size_t start = i;
        while (i < input.size()) {
            const auto segment_byte = static_cast<uint8_t>(input[i]);
            const bool segment_direct = imap ? is_utf7_imap_direct(segment_byte) : is_utf7_direct(segment_byte);
            if (segment_direct) break;
            ++i;
        }
        const auto segment = std::string_view(input).substr(start, i - start);
        if (!imap && segment == "+") output += "+-";
        else if (imap && segment == "&") output += "&-";
        else {
            output.push_back(imap ? '&' : '+');
            auto b64 = Buffer::from(encode_utf16be(segment)).toString("base64");
            b64 = trim_base64_padding(std::move(b64));
            if (imap) std::replace(b64.begin(), b64.end(), '/', ',');
            output += b64;
            output.push_back('-');
        }
    }
    return Buffer::from(output);
}

std::string decode_utf7_base64(std::string b64, bool imap) {
    if (imap) std::replace(b64.begin(), b64.end(), ',', '/');
    const auto bytes = Buffer::from(b64, "base64");
    return decode_utf16_bytes(bytes.data(), bytes.length(), false);
}

std::string ascii_decode_lossy(const uint8_t* data, size_t size) {
    std::string out;
    out.reserve(size);
    for (size_t i = 0; i < size; ++i) append_utf8(out, data[i] <= 0x7F ? data[i] : 0xFFFD);
    return out;
}

std::string decode_utf7_like(const Buffer& input, bool imap) {
    std::string output;
    const uint8_t shift = imap ? '&' : '+';
    const uint8_t unshift = '-';
    size_t i = 0;
    size_t direct_start = 0;
    while (i < input.length()) {
        if (input[i] != shift) {
            ++i;
            continue;
        }
        output += ascii_decode_lossy(input.data() + direct_start, i - direct_start);
        ++i;
        if (i < input.length() && input[i] == unshift) {
            output.push_back(static_cast<char>(shift));
            ++i;
            direct_start = i;
            continue;
        }
        const size_t b64_start = i;
        while (i < input.length() && is_base64_char(input[i], imap)) ++i;
        output += decode_utf7_base64(std::string(reinterpret_cast<const char*>(input.data() + b64_start), i - b64_start), imap);
        if (i < input.length() && input[i] == unshift) ++i;
        direct_start = i;
    }
    output += ascii_decode_lossy(input.data() + direct_start, input.length() - direct_start);
    return output;
}

std::vector<uint8_t> encode_binary(std::string_view input) {
    auto units = utf8_to_utf16(input);
    std::vector<uint8_t> bytes;
    bytes.reserve(units.size());
    for (auto unit : units) bytes.push_back(static_cast<uint8_t>(unit & 0xFF));
    return bytes;
}

std::string maybe_strip_bom(std::string output, const ResolvedEncoding& resolved, const DecodeOptions& options) {
    return resolved.bom_aware && options.strip_bom ? strip_utf8_bom(std::move(output)) : output;
}

}  // namespace

std::string canonicalize_encoding(std::string_view encoding) {
    std::string lower;
    lower.reserve(encoding.size());
    for (unsigned char c : encoding) lower.push_back(static_cast<char>(std::tolower(c)));
    if (lower.size() >= 5 && lower[lower.size() - 5] == ':' &&
        std::all_of(lower.end() - 4, lower.end(), [](unsigned char c) { return std::isdigit(c) != 0; })) {
        lower.resize(lower.size() - 5);
    }
    std::string canonical;
    canonical.reserve(lower.size());
    for (unsigned char c : lower) {
        if (std::isalnum(c) != 0) canonical.push_back(static_cast<char>(c));
    }
    return canonical;
}

bool encoding_exists(std::string_view encoding) noexcept {
    try {
        (void)resolve_encoding(encoding);
        return true;
    } catch (...) {
        return false;
    }
}

EncodingInfo inspect_encoding(std::string_view encoding) {
    auto resolved = resolve_encoding(encoding);
    return EncodingInfo{std::string(encoding), resolved.canonical, resolved.converter, resolved.bom_aware,
                        resolved.kind != EncodingKind::sbcs && resolved.kind != EncodingKind::dbcs};
}

Buffer encode(std::string_view input, std::string_view encoding, const EncodeOptions& options) {
    const auto resolved = resolve_encoding(encoding);
    switch (resolved.kind) {
        case EncodingKind::utf8:
            return with_bom(std::vector<uint8_t>(input.begin(), input.end()), resolved, options);
        case EncodingKind::cesu8:
            return with_bom(encode_cesu8(input), resolved, options);
        case EncodingKind::binary:
            return buffer_from_bytes(encode_binary(input));
        case EncodingKind::base64:
            return Buffer::from(std::string(input), "base64");
        case EncodingKind::hex:
            return Buffer::from(std::string(input), "hex");
        case EncodingKind::utf16le:
            return with_bom(encode_utf16le(input), resolved, options);
        case EncodingKind::utf16be:
            return with_bom(encode_utf16be(input), resolved, options);
        case EncodingKind::utf16_auto:
            return with_bom(encode_utf16le(input), resolved, options);
        case EncodingKind::utf32le:
            return with_bom(encode_utf32(input, true), resolved, options);
        case EncodingKind::utf32be:
            return with_bom(encode_utf32(input, false), resolved, options);
        case EncodingKind::utf32_auto: {
            const auto selected = normalize_default_encoding(options.default_encoding, "utf32le", "utf32");
            auto bytes = encode_utf32(input, selected != "utf32be");
            if (options.add_bom.value_or(true)) {
                if (selected == "utf32be") prepend_bytes(bytes, {0x00, 0x00, 0xFE, 0xFF});
                else prepend_bytes(bytes, {0xFF, 0xFE, 0x00, 0x00});
            }
            return buffer_from_bytes(bytes);
        }
        case EncodingKind::utf7:
            return encode_utf7_like(maybe_add_utf8_bom(input, resolved, options), false);
        case EncodingKind::utf7_imap:
            return encode_utf7_like(maybe_add_utf8_bom(input, resolved, options), true);
        case EncodingKind::sbcs:
            return get_sbcs(resolved.spec_index).encode(input);
        case EncodingKind::dbcs:
            return get_dbcs(resolved.spec_index).encode(input);
    }
    throw polycpp::TypeError("Unsupported encoding").setCode("ERR_ENCODING_NOT_SUPPORTED");
}

std::string decode(const Buffer& input, std::string_view encoding, const DecodeOptions& options) {
    const auto resolved = resolve_encoding(encoding);
    switch (resolved.kind) {
        case EncodingKind::utf8:
            return maybe_strip_bom(decode_utf8_bytes(input.data(), input.length()), resolved, options);
        case EncodingKind::cesu8:
            return maybe_strip_bom(decode_cesu8(input.data(), input.length()), resolved, options);
        case EncodingKind::binary:
            return input.toString("latin1");
        case EncodingKind::base64:
            return input.toString("base64");
        case EncodingKind::hex:
            return input.toString("hex");
        case EncodingKind::utf16le:
            return maybe_strip_bom(decode_utf16_bytes(input.data(), input.length(), true), resolved, options);
        case EncodingKind::utf16be:
            return maybe_strip_bom(decode_utf16_bytes(input.data(), input.length(), false), resolved, options);
        case EncodingKind::utf16_auto: {
            const auto selected = choose_utf16_decode(input, options);
            return maybe_strip_bom(decode_utf16_bytes(input.data(), input.length(), selected != "utf16be"), resolved, options);
        }
        case EncodingKind::utf32le:
            return maybe_strip_bom(decode_utf32_bytes(input.data(), input.length(), true), resolved, options);
        case EncodingKind::utf32be:
            return maybe_strip_bom(decode_utf32_bytes(input.data(), input.length(), false), resolved, options);
        case EncodingKind::utf32_auto: {
            const auto selected = choose_utf32_decode(input, options);
            return maybe_strip_bom(decode_utf32_bytes(input.data(), input.length(), selected != "utf32be"), resolved, options);
        }
        case EncodingKind::utf7:
            return maybe_strip_bom(decode_utf7_like(input, false), resolved, options);
        case EncodingKind::utf7_imap:
            return maybe_strip_bom(decode_utf7_like(input, true), resolved, options);
        case EncodingKind::sbcs:
            return get_sbcs(resolved.spec_index).decode(input);
        case EncodingKind::dbcs:
            return get_dbcs(resolved.spec_index).decode(input);
    }
    throw polycpp::TypeError("Unsupported encoding").setCode("ERR_ENCODING_NOT_SUPPORTED");
}

}  // namespace polycpp::iconv_lite
