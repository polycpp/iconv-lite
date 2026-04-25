#include <polycpp/iconv_lite/iconv_lite.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>

namespace polycpp::iconv_lite {
namespace {

enum class EncodingKind {
    icu,
    binary,
    base64,
    hex,
    utf7,
    utf7_imap,
    utf16_auto,
    utf32_auto,
};

struct ResolvedEncoding {
    std::string canonical;
    std::string converter;
    EncodingKind kind = EncodingKind::icu;
    bool bom_aware = false;
};

bool is_digit_string(std::string_view value) noexcept {
    return !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char c) {
        return std::isdigit(c) != 0;
    });
}

bool starts_with(std::string_view value, std::string_view prefix) noexcept {
    return value.size() >= prefix.size() && value.substr(0, prefix.size()) == prefix;
}

bool icu_converter_exists(std::string_view name) noexcept {
    if (name.empty()) return false;
    UErrorCode status = U_ZERO_ERROR;
    UConverter* converter = ucnv_open(std::string(name).c_str(), &status);
    if (converter != nullptr) {
        ucnv_close(converter);
    }
    return U_SUCCESS(status) && converter != nullptr;
}

std::string iso_8859_converter(std::string_view suffix) {
    return "ISO-8859-" + std::string(suffix);
}

std::string windows_converter(std::string_view suffix) {
    return "windows-" + std::string(suffix);
}

std::string cp_converter(std::string_view suffix) {
    return "cp" + std::string(suffix);
}

std::string ibm_converter(std::string_view suffix) {
    return "ibm-" + std::string(suffix);
}

std::string resolve_iso_8859(std::string_view suffix) {
    if (!is_digit_string(suffix)) return {};
    const auto number = std::stoi(std::string(suffix));
    if ((number >= 1 && number <= 11) || (number >= 13 && number <= 16)) {
        return iso_8859_converter(suffix);
    }
    return {};
}

std::string resolve_numeric(std::string_view digits) {
    if (!is_digit_string(digits)) return {};

    const auto number = std::stoi(std::string(digits));
    if (number == 874 || (number >= 1250 && number <= 1258)) {
        return windows_converter(digits);
    }
    if (number >= 28591 && number <= 28606) {
        const auto suffix = std::to_string(number - 28590);
        return resolve_iso_8859(suffix);
    }

    const std::array<std::string, 2> candidates = {
        cp_converter(digits),
        ibm_converter(digits),
    };
    for (const auto& candidate : candidates) {
        if (icu_converter_exists(candidate)) return candidate;
    }
    return {};
}

std::string resolve_windows(std::string_view digits) {
    if (!is_digit_string(digits)) return {};
    const auto candidate = windows_converter(digits);
    return icu_converter_exists(candidate) ? candidate : std::string{};
}

std::string resolve_cp(std::string_view digits) {
    if (!is_digit_string(digits)) return {};
    const auto number = std::stoi(std::string(digits));
    if (number == 874 || (number >= 1250 && number <= 1258)) {
        return windows_converter(digits);
    }
    if (number >= 28591 && number <= 28606) {
        const auto suffix = std::to_string(number - 28590);
        return resolve_iso_8859(suffix);
    }
    const auto candidate = cp_converter(digits);
    return icu_converter_exists(candidate) ? candidate : std::string{};
}

std::string lookup_named_converter(std::string_view canonical) {
    struct Alias {
        std::string_view name;
        std::string_view converter;
    };

    static constexpr Alias aliases[] = {
        {"ascii", "US-ASCII"},
        {"ascii8bit", "US-ASCII"},
        {"usascii", "US-ASCII"},
        {"ansix34", "US-ASCII"},
        {"ansix341968", "US-ASCII"},
        {"ansix341986", "US-ASCII"},
        {"csascii", "US-ASCII"},
        {"cp367", "US-ASCII"},
        {"ibm367", "US-ASCII"},
        {"isoir6", "US-ASCII"},
        {"iso646us", "US-ASCII"},
        {"iso646irv", "US-ASCII"},
        {"us", "US-ASCII"},

        {"latin1", "ISO-8859-1"},
        {"latin2", "ISO-8859-2"},
        {"latin3", "ISO-8859-3"},
        {"latin4", "ISO-8859-4"},
        {"latin5", "ISO-8859-9"},
        {"latin6", "ISO-8859-10"},
        {"latin7", "ISO-8859-13"},
        {"latin8", "ISO-8859-14"},
        {"latin9", "ISO-8859-15"},
        {"latin10", "ISO-8859-16"},
        {"l1", "ISO-8859-1"},
        {"l2", "ISO-8859-2"},
        {"l3", "ISO-8859-3"},
        {"l4", "ISO-8859-4"},
        {"l5", "ISO-8859-9"},
        {"l6", "ISO-8859-10"},
        {"l7", "ISO-8859-13"},
        {"l8", "ISO-8859-14"},
        {"l9", "ISO-8859-15"},
        {"l10", "ISO-8859-16"},
        {"cp819", "ISO-8859-1"},
        {"ibm819", "ISO-8859-1"},
        {"csisolatin1", "ISO-8859-1"},
        {"csisolatin2", "ISO-8859-2"},
        {"csisolatin3", "ISO-8859-3"},
        {"csisolatin4", "ISO-8859-4"},
        {"csisolatincyrillic", "ISO-8859-5"},
        {"csisolatinarabic", "ISO-8859-6"},
        {"csisolatingreek", "ISO-8859-7"},
        {"csisolatinhebrew", "ISO-8859-8"},
        {"csisolatin5", "ISO-8859-9"},
        {"csisolatin6", "ISO-8859-10"},
        {"cyrillic", "ISO-8859-5"},
        {"arabic", "ISO-8859-6"},
        {"arabic8", "ISO-8859-6"},
        {"ecma114", "ISO-8859-6"},
        {"asmo708", "ISO-8859-6"},
        {"greek", "ISO-8859-7"},
        {"greek8", "ISO-8859-7"},
        {"ecma118", "ISO-8859-7"},
        {"elot928", "ISO-8859-7"},
        {"hebrew", "ISO-8859-8"},
        {"hebrew8", "ISO-8859-8"},
        {"turkish", "ISO-8859-9"},
        {"turkish8", "ISO-8859-9"},
        {"thai", "ISO-8859-11"},
        {"thai8", "ISO-8859-11"},
        {"celtic", "ISO-8859-14"},
        {"celtic8", "ISO-8859-14"},
        {"isoceltic", "ISO-8859-14"},
        {"tis620", "TIS-620"},
        {"tis6200", "TIS-620"},
        {"tis62025291", "TIS-620"},
        {"tis62025330", "TIS-620"},

        {"msee", "windows-1250"},
        {"mscyrl", "windows-1251"},
        {"msansi", "windows-1252"},
        {"msgreek", "windows-1253"},
        {"msturk", "windows-1254"},
        {"mshebr", "windows-1255"},
        {"msarab", "windows-1256"},
        {"winbaltrim", "windows-1257"},

        {"koi8r", "KOI8-R"},
        {"cp20866", "KOI8-R"},
        {"ibm878", "KOI8-R"},
        {"cskoi8r", "KOI8-R"},
        {"koi8u", "KOI8-U"},
        {"cp21866", "KOI8-U"},
        {"ibm1168", "KOI8-U"},

        {"shiftjis", "Shift_JIS"},
        {"csshiftjis", "Shift_JIS"},
        {"mskanji", "Shift_JIS"},
        {"sjis", "Shift_JIS"},
        {"windows31j", "Shift_JIS"},
        {"ms31j", "Shift_JIS"},
        {"xsjis", "Shift_JIS"},
        {"windows932", "Shift_JIS"},
        {"ms932", "Shift_JIS"},
        {"eucjp", "EUC-JP"},

        {"gb2312", "cp936"},
        {"gb231280", "cp936"},
        {"gb23121980", "cp936"},
        {"csgb2312", "cp936"},
        {"csiso58gb231280", "cp936"},
        {"euccn", "cp936"},
        {"windows936", "cp936"},
        {"ms936", "cp936"},
        {"cp936", "cp936"},
        {"gbk", "GBK"},
        {"xgbk", "GBK"},
        {"isoir58", "GBK"},
        {"gb18030", "GB18030"},
        {"chinese", "GB18030"},

        {"windows949", "cp949"},
        {"ms949", "cp949"},
        {"cp949", "cp949"},
        {"cseuckr", "cp949"},
        {"csksc56011987", "cp949"},
        {"euckr", "cp949"},
        {"isoir149", "cp949"},
        {"korean", "cp949"},
        {"ksc56011987", "cp949"},
        {"ksc56011989", "cp949"},
        {"ksc5601", "cp949"},

        {"windows950", "cp950"},
        {"ms950", "cp950"},
        {"cp950", "cp950"},
        {"big5", "Big5-HKSCS"},
        {"big5hkscs", "Big5-HKSCS"},
        {"cnbig5", "Big5-HKSCS"},
        {"csbig5", "Big5-HKSCS"},
        {"xxbig5", "Big5-HKSCS"},

        {"utf7", "UTF-7"},
        {"unicode11utf7", "UTF-7"},
        {"utf7imap", "IMAP-mailbox-name"},

        {"mac", "macintosh"},
        {"macintosh", "macintosh"},
        {"csmacintosh", "macintosh"},
        {"macroman", "macintosh"},
        {"macgreek", "macgreek"},
        {"maccyrillic", "maccyrillic"},
        {"maciceland", "maciceland"},
        {"macturkish", "macturkish"},
    };

    for (const auto& alias : aliases) {
        if (canonical == alias.name) return std::string(alias.converter);
    }
    return {};
}

ResolvedEncoding resolve_encoding(std::string_view encoding) {
    const auto canonical = canonicalize_encoding(encoding);

    if (canonical == "base64") return {canonical, "base64", EncodingKind::base64, false};
    if (canonical == "hex") return {canonical, "hex", EncodingKind::hex, false};
    if (canonical == "binary") return {canonical, "binary", EncodingKind::binary, false};
    if (canonical == "utf7" || canonical == "unicode11utf7") {
        return {canonical, "UTF-7", EncodingKind::utf7, true};
    }
    if (canonical == "utf7imap") {
        return {canonical, "IMAP-mailbox-name", EncodingKind::utf7_imap, true};
    }

    if (canonical == "utf8" || canonical == "unicode11utf8") {
        return {canonical, "UTF-8", EncodingKind::icu, true};
    }
    if (canonical == "cesu8") {
        return {canonical, "CESU-8", EncodingKind::icu, true};
    }
    if (canonical == "ucs2" || canonical == "utf16le") {
        return {canonical, "UTF-16LE", EncodingKind::icu, true};
    }
    if (canonical == "utf16be") {
        return {canonical, "UTF-16BE", EncodingKind::icu, true};
    }
    if (canonical == "utf16") {
        return {canonical, "UTF-16LE", EncodingKind::utf16_auto, true};
    }
    if (canonical == "utf32le" || canonical == "ucs4le") {
        return {canonical, "UTF-32LE", EncodingKind::icu, true};
    }
    if (canonical == "utf32be" || canonical == "ucs4be") {
        return {canonical, "UTF-32BE", EncodingKind::icu, true};
    }
    if (canonical == "utf32" || canonical == "ucs4") {
        return {canonical, "UTF-32LE", EncodingKind::utf32_auto, true};
    }

    auto converter = lookup_named_converter(canonical);
    if (converter.empty() && starts_with(canonical, "iso8859")) {
        converter = resolve_iso_8859(std::string_view(canonical).substr(7));
    }
    if (converter.empty() && starts_with(canonical, "windows")) {
        converter = resolve_windows(std::string_view(canonical).substr(7));
    }
    if (converter.empty() && starts_with(canonical, "win")) {
        converter = resolve_windows(std::string_view(canonical).substr(3));
    }
    if (converter.empty() && starts_with(canonical, "cp")) {
        converter = resolve_cp(std::string_view(canonical).substr(2));
    }
    if (converter.empty() && starts_with(canonical, "ibm")) {
        const auto digits = std::string_view(canonical).substr(3);
        if (is_digit_string(digits)) converter = ibm_converter(digits);
    }
    if (converter.empty() && is_digit_string(canonical)) {
        converter = resolve_numeric(canonical);
    }

    if (!converter.empty() && icu_converter_exists(converter)) {
        return {canonical, converter, EncodingKind::icu, false};
    }

    throw polycpp::TypeError(
        "Encoding not recognized: '" + std::string(encoding) +
        "' (searched as: '" + canonical + "')")
        .setCode("ERR_ENCODING_NOT_SUPPORTED");
}

bool is_integer_within_icu_limit(size_t value) noexcept {
    return value <= static_cast<size_t>(std::numeric_limits<int32_t>::max());
}

[[noreturn]] void throw_conversion_error(std::string_view operation,
                                         std::string_view encoding,
                                         UErrorCode status) {
    throw polycpp::TypeError(
        "Failed to " + std::string(operation) + " using encoding '" +
        std::string(encoding) + "': " + u_errorName(status))
        .setCode("ERR_ENCODING_CONVERSION_FAILED");
}

std::vector<UChar> utf8_to_uchars(std::string_view input, std::string_view encoding) {
    if (!is_integer_within_icu_limit(input.size())) {
        throw polycpp::RangeError("Input is too large for ICU conversion")
            .setCode("ERR_OUT_OF_RANGE");
    }

    UErrorCode status = U_ZERO_ERROR;
    int32_t required = 0;
    u_strFromUTF8WithSub(nullptr,
                         0,
                         &required,
                         input.data(),
                         static_cast<int32_t>(input.size()),
                         0xFFFD,
                         nullptr,
                         &status);
    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
        throw_conversion_error("read UTF-8 input", encoding, status);
    }

    status = U_ZERO_ERROR;
    std::vector<UChar> output(static_cast<size_t>(required) + 1);
    int32_t actual = 0;
    u_strFromUTF8WithSub(output.data(),
                         static_cast<int32_t>(output.size()),
                         &actual,
                         input.data(),
                         static_cast<int32_t>(input.size()),
                         0xFFFD,
                         nullptr,
                         &status);
    if (U_FAILURE(status)) {
        throw_conversion_error("read UTF-8 input", encoding, status);
    }
    output.resize(static_cast<size_t>(actual));
    return output;
}

std::string uchars_to_utf8(const UChar* input, int32_t length, std::string_view encoding) {
    UErrorCode status = U_ZERO_ERROR;
    int32_t required = 0;
    u_strToUTF8WithSub(nullptr,
                       0,
                       &required,
                       input,
                       length,
                       0xFFFD,
                       nullptr,
                       &status);
    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
        throw_conversion_error("write UTF-8 output", encoding, status);
    }

    status = U_ZERO_ERROR;
    std::string output(static_cast<size_t>(required), '\0');
    int32_t actual = 0;
    u_strToUTF8WithSub(output.data(),
                       static_cast<int32_t>(output.size()),
                       &actual,
                       input,
                       length,
                       0xFFFD,
                       nullptr,
                       &status);
    if (U_FAILURE(status)) {
        throw_conversion_error("write UTF-8 output", encoding, status);
    }
    output.resize(static_cast<size_t>(actual));
    return output;
}

class ConverterHandle {
public:
    explicit ConverterHandle(std::string_view converter_name) {
        UErrorCode status = U_ZERO_ERROR;
        converter_ = ucnv_open(std::string(converter_name).c_str(), &status);
        if (U_FAILURE(status) || converter_ == nullptr) {
            throw polycpp::TypeError("Failed to create converter for " + std::string(converter_name))
                .setCode("ERR_ENCODING_NOT_SUPPORTED");
        }
    }

    ConverterHandle(const ConverterHandle&) = delete;
    ConverterHandle& operator=(const ConverterHandle&) = delete;

    ~ConverterHandle() {
        if (converter_ != nullptr) ucnv_close(converter_);
    }

    UConverter* get() const noexcept { return converter_; }

private:
    UConverter* converter_ = nullptr;
};

Buffer encode_with_icu(std::string_view input,
                       std::string_view converter_name,
                       std::string_view requested_encoding) {
    auto utf16 = utf8_to_uchars(input, requested_encoding);
    ConverterHandle converter(converter_name);

    UErrorCode callback_status = U_ZERO_ERROR;
    ucnv_setFromUCallBack(converter.get(),
                          UCNV_FROM_U_CALLBACK_SUBSTITUTE,
                          nullptr,
                          nullptr,
                          nullptr,
                          &callback_status);

    UErrorCode subst_status = U_ZERO_ERROR;
    ucnv_setSubstChars(converter.get(), "?", 1, &subst_status);

    UErrorCode status = U_ZERO_ERROR;
    int32_t required = ucnv_fromUChars(converter.get(),
                                       nullptr,
                                       0,
                                       utf16.data(),
                                       static_cast<int32_t>(utf16.size()),
                                       &status);
    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
        throw_conversion_error("encode", requested_encoding, status);
    }

    status = U_ZERO_ERROR;
    std::vector<uint8_t> bytes(static_cast<size_t>(required));
    const auto actual = ucnv_fromUChars(converter.get(),
                                        reinterpret_cast<char*>(bytes.data()),
                                        static_cast<int32_t>(bytes.size()),
                                        utf16.data(),
                                        static_cast<int32_t>(utf16.size()),
                                        &status);
    if (U_FAILURE(status)) {
        throw_conversion_error("encode", requested_encoding, status);
    }
    bytes.resize(static_cast<size_t>(actual));
    return Buffer::from(bytes);
}

std::string decode_with_icu(const uint8_t* data,
                            size_t length,
                            std::string_view converter_name,
                            std::string_view requested_encoding) {
    if (!is_integer_within_icu_limit(length)) {
        throw polycpp::RangeError("Input is too large for ICU conversion")
            .setCode("ERR_OUT_OF_RANGE");
    }

    ConverterHandle converter(converter_name);
    UErrorCode callback_status = U_ZERO_ERROR;
    ucnv_setToUCallBack(converter.get(),
                        UCNV_TO_U_CALLBACK_SUBSTITUTE,
                        nullptr,
                        nullptr,
                        nullptr,
                        &callback_status);

    UErrorCode status = U_ZERO_ERROR;
    int32_t required = ucnv_toUChars(converter.get(),
                                     nullptr,
                                     0,
                                     reinterpret_cast<const char*>(data),
                                     static_cast<int32_t>(length),
                                     &status);
    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
        throw_conversion_error("decode", requested_encoding, status);
    }

    status = U_ZERO_ERROR;
    std::vector<UChar> utf16(static_cast<size_t>(required) + 1);
    int32_t actual = ucnv_toUChars(converter.get(),
                                   utf16.data(),
                                   static_cast<int32_t>(utf16.size()),
                                   reinterpret_cast<const char*>(data),
                                   static_cast<int32_t>(length),
                                   &status);
    if (U_FAILURE(status)) {
        throw_conversion_error("decode", requested_encoding, status);
    }
    return uchars_to_utf8(utf16.data(), actual, requested_encoding);
}

std::vector<uint8_t> copy_bytes(const Buffer& input) {
    return std::vector<uint8_t>(input.data(), input.data() + input.length());
}

void prepend_bytes(std::vector<uint8_t>& bytes, std::initializer_list<uint8_t> prefix) {
    bytes.insert(bytes.begin(), prefix.begin(), prefix.end());
}

Buffer prepend_bom(Buffer bytes, const ResolvedEncoding& resolved, const EncodeOptions& options) {
    bool add_bom = false;
    if (options.add_bom.has_value()) {
        add_bom = resolved.bom_aware && *options.add_bom;
    } else {
        add_bom = resolved.kind == EncodingKind::utf16_auto || resolved.kind == EncodingKind::utf32_auto;
    }
    if (!add_bom) return bytes;

    auto data = copy_bytes(bytes);
    if (resolved.converter == "UTF-8" || resolved.converter == "CESU-8" || resolved.converter == "UTF-7") {
        prepend_bytes(data, {0xEF, 0xBB, 0xBF});
    } else if (resolved.converter == "UTF-16LE") {
        prepend_bytes(data, {0xFF, 0xFE});
    } else if (resolved.converter == "UTF-16BE") {
        prepend_bytes(data, {0xFE, 0xFF});
    } else if (resolved.converter == "UTF-32LE") {
        prepend_bytes(data, {0xFF, 0xFE, 0x00, 0x00});
    } else if (resolved.converter == "UTF-32BE") {
        prepend_bytes(data, {0x00, 0x00, 0xFE, 0xFF});
    }
    return Buffer::from(data);
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

std::string normalize_default_encoding(std::string_view value,
                                       std::string_view fallback,
                                       std::string_view expected_auto) {
    if (value.empty()) return std::string(fallback);
    const auto canonical = canonicalize_encoding(value);
    if (expected_auto == "utf16") {
        if (canonical == "utf16be") return "UTF-16BE";
        if (canonical == "utf16le" || canonical == "ucs2" || canonical == "utf16") return "UTF-16LE";
    } else if (expected_auto == "utf32") {
        if (canonical == "utf32be" || canonical == "ucs4be") return "UTF-32BE";
        if (canonical == "utf32le" || canonical == "ucs4le" || canonical == "utf32" || canonical == "ucs4") return "UTF-32LE";
    }
    return std::string(fallback);
}

std::string choose_utf16_decode_converter(const Buffer& input, const DecodeOptions& options) {
    if (input.length() >= 2) {
        if (input[0] == 0xFF && input[1] == 0xFE) return "UTF-16LE";
        if (input[0] == 0xFE && input[1] == 0xFF) return "UTF-16BE";
    }

    size_t ascii_be = 0;
    size_t ascii_le = 0;
    const auto limit = std::min(input.length(), static_cast<size_t>(200));
    for (size_t i = 0; i + 1 < limit; i += 2) {
        if (input[i] == 0x00 && input[i + 1] != 0x00) ++ascii_be;
        if (input[i] != 0x00 && input[i + 1] == 0x00) ++ascii_le;
    }
    if (ascii_be > ascii_le) return "UTF-16BE";
    if (ascii_le > ascii_be) return "UTF-16LE";
    return normalize_default_encoding(options.default_encoding, "UTF-16LE", "utf16");
}

std::string choose_utf32_decode_converter(const Buffer& input, const DecodeOptions& options) {
    if (input.length() >= 4) {
        if (input[0] == 0xFF && input[1] == 0xFE && input[2] == 0x00 && input[3] == 0x00) return "UTF-32LE";
        if (input[0] == 0x00 && input[1] == 0x00 && input[2] == 0xFE && input[3] == 0xFF) return "UTF-32BE";
    }

    size_t invalid_le = 0;
    size_t invalid_be = 0;
    size_t bmp_le = 0;
    size_t bmp_be = 0;
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

    const auto score_be = static_cast<int64_t>(bmp_be) - static_cast<int64_t>(invalid_be);
    const auto score_le = static_cast<int64_t>(bmp_le) - static_cast<int64_t>(invalid_le);
    if (score_be > score_le) return "UTF-32BE";
    if (score_le > score_be) return "UTF-32LE";
    return normalize_default_encoding(options.default_encoding, "UTF-32LE", "utf32");
}

Buffer encode_utf_auto(std::string_view input,
                       const ResolvedEncoding& resolved,
                       const EncodeOptions& options,
                       std::string_view expected_auto) {
    auto selected = std::string(resolved.converter);
    if (expected_auto == "utf32") {
        selected = normalize_default_encoding(options.default_encoding, "UTF-32LE", expected_auto);
    }
    auto adjusted = resolved;
    adjusted.converter = selected;
    return prepend_bom(encode_with_icu(input, selected, resolved.canonical), adjusted, options);
}

std::string decode_utf_auto(const Buffer& input,
                            const ResolvedEncoding& resolved,
                            const DecodeOptions& options) {
    auto selected = resolved.kind == EncodingKind::utf16_auto
        ? choose_utf16_decode_converter(input, options)
        : choose_utf32_decode_converter(input, options);
    auto output = decode_with_icu(input.data(), input.length(), selected, resolved.canonical);
    if (options.strip_bom) output = strip_utf8_bom(std::move(output));
    return output;
}

std::string maybe_strip_bom(std::string output, const ResolvedEncoding& resolved, const DecodeOptions& options) {
    if (resolved.bom_aware && options.strip_bom) return strip_utf8_bom(std::move(output));
    return output;
}

bool is_utf7_direct(uint8_t c) noexcept {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '\'' || c == '(' || c == ')' || c == ',' || c == '-' ||
           c == '.' || c == '/' || c == ':' || c == '?' || c == ' ' ||
           c == '\n' || c == '\r' || c == '\t';
}

bool is_utf7_imap_direct(uint8_t c) noexcept {
    return c >= 0x20 && c <= 0x7E && c != '&';
}

bool is_base64_char(uint8_t c, bool imap) noexcept {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '+' || c == '/' || (imap && c == ',');
}

std::string ascii_decode_lossy(const uint8_t* data, size_t length) {
    std::string output;
    output.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        if (data[i] <= 0x7F) {
            output.push_back(static_cast<char>(data[i]));
        } else {
            output += "\xEF\xBF\xBD";
        }
    }
    return output;
}

std::string utf16be_to_trimmed_base64(std::string_view input, bool imap) {
    auto utf16be = encode_with_icu(input, "UTF-16BE", imap ? "utf7imap" : "utf7");
    auto encoded = utf16be.toString("base64");
    while (!encoded.empty() && encoded.back() == '=') encoded.pop_back();
    if (imap) {
        std::replace(encoded.begin(), encoded.end(), '/', ',');
    }
    return encoded;
}

std::string maybe_with_bom(std::string_view input, const ResolvedEncoding& resolved, const EncodeOptions& options) {
    if (resolved.bom_aware && options.add_bom.value_or(false)) {
        return std::string("\xEF\xBB\xBF", 3) + std::string(input);
    }
    return std::string(input);
}

Buffer encode_utf7_like(std::string_view input, bool imap) {
    std::string output;
    output.reserve(input.size() + 8);

    size_t i = 0;
    while (i < input.size()) {
        const auto byte = static_cast<uint8_t>(input[i]);
        const bool direct = imap ? is_utf7_imap_direct(byte) : is_utf7_direct(byte);
        if (direct) {
            output.push_back(input[i]);
            ++i;
            continue;
        }

        const size_t start = i;
        while (i < input.size()) {
            const auto segment_byte = static_cast<uint8_t>(input[i]);
            const bool segment_direct = imap ? is_utf7_imap_direct(segment_byte) : is_utf7_direct(segment_byte);
            if (segment_direct) break;
            ++i;
        }

        const auto segment = input.substr(start, i - start);
        if (!imap && segment == "+") {
            output += "+-";
        } else if (imap && segment == "&") {
            output += "&-";
        } else {
            output.push_back(imap ? '&' : '+');
            output += utf16be_to_trimmed_base64(segment, imap);
            output.push_back('-');
        }
    }

    return Buffer::from(output);
}

std::string decode_utf7_base64(std::string b64, bool imap) {
    if (b64.empty()) return {};
    if (imap) {
        std::replace(b64.begin(), b64.end(), ',', '/');
    }
    auto bytes = Buffer::from(b64, "base64");
    return decode_with_icu(bytes.data(), bytes.length(), "UTF-16BE", imap ? "utf7imap" : "utf7");
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
        while (i < input.length() && is_base64_char(input[i], imap)) {
            ++i;
        }

        output += decode_utf7_base64(
            std::string(reinterpret_cast<const char*>(input.data() + b64_start), i - b64_start),
            imap);

        if (i < input.length() && input[i] == unshift) {
            ++i;
        }
        direct_start = i;
    }

    output += ascii_decode_lossy(input.data() + direct_start, input.length() - direct_start);
    return output;
}

}  // namespace

std::string canonicalize_encoding(std::string_view encoding) {
    std::string lower;
    lower.reserve(encoding.size());
    for (unsigned char c : encoding) {
        lower.push_back(static_cast<char>(std::tolower(c)));
    }

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
    return EncodingInfo{
        std::string(encoding),
        resolved.canonical,
        resolved.converter,
        resolved.bom_aware,
        resolved.kind == EncodingKind::binary || resolved.kind == EncodingKind::base64 ||
            resolved.kind == EncodingKind::hex || resolved.kind == EncodingKind::utf7 ||
            resolved.kind == EncodingKind::utf7_imap,
    };
}

Buffer encode(std::string_view input, std::string_view encoding, const EncodeOptions& options) {
    const auto resolved = resolve_encoding(encoding);

    if (resolved.kind == EncodingKind::base64) {
        return Buffer::from(std::string(input), "base64");
    }
    if (resolved.kind == EncodingKind::hex) {
        return Buffer::from(std::string(input), "hex");
    }
    if (resolved.kind == EncodingKind::binary) {
        return Buffer::from(std::string(input), "latin1");
    }
    if (resolved.kind == EncodingKind::utf7) {
        return encode_utf7_like(maybe_with_bom(input, resolved, options), false);
    }
    if (resolved.kind == EncodingKind::utf7_imap) {
        return encode_utf7_like(maybe_with_bom(input, resolved, options), true);
    }
    if (resolved.kind == EncodingKind::utf16_auto) {
        return encode_utf_auto(input, resolved, options, "utf16");
    }
    if (resolved.kind == EncodingKind::utf32_auto) {
        return encode_utf_auto(input, resolved, options, "utf32");
    }

    return prepend_bom(encode_with_icu(input, resolved.converter, resolved.canonical), resolved, options);
}

std::string decode(const Buffer& input, std::string_view encoding, const DecodeOptions& options) {
    const auto resolved = resolve_encoding(encoding);

    if (resolved.kind == EncodingKind::base64) {
        return input.toString("base64");
    }
    if (resolved.kind == EncodingKind::hex) {
        return input.toString("hex");
    }
    if (resolved.kind == EncodingKind::binary) {
        return input.toString("latin1");
    }
    if (resolved.kind == EncodingKind::utf7) {
        return maybe_strip_bom(decode_utf7_like(input, false), resolved, options);
    }
    if (resolved.kind == EncodingKind::utf7_imap) {
        return maybe_strip_bom(decode_utf7_like(input, true), resolved, options);
    }
    if (resolved.kind == EncodingKind::utf16_auto || resolved.kind == EncodingKind::utf32_auto) {
        return decode_utf_auto(input, resolved, options);
    }

    return maybe_strip_bom(
        decode_with_icu(input.data(), input.length(), resolved.converter, resolved.canonical),
        resolved,
        options);
}

}  // namespace polycpp::iconv_lite
