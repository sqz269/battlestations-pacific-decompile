#include "bsp/locale_text_lookup.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {

template<class String>
void require_native_string(const String& text) {
    if (text.size() > static_cast<std::size_t>(
            (std::numeric_limits<std::int32_t>::max)())) {
        throw std::length_error("locale text exceeds native signed string range");
    }
    if (text.find(typename String::value_type{}) != String::npos) {
        throw std::invalid_argument("locale text contains an embedded terminator");
    }
}

std::u16string widen_bytes_004c5e60(const std::string& text) {
    std::u16string result;
    result.reserve(text.size());
    for (const unsigned char value : text) {
        result.push_back(static_cast<char16_t>(value));
    }
    return result;
}

std::string narrow_low_bytes_00436630(const std::u16string& text) {
    std::string result;
    result.reserve(text.size());
    for (const char16_t value : text) {
        const auto byte = static_cast<unsigned char>(value & 0xffu);
        if (byte == 0) {
            // Native allocates the original wide length, but stops copying on
            // the low-byte zero, leaving the rest of the header's range unread.
            throw std::invalid_argument("locale substitution narrows to an early terminator");
        }
        result.push_back(static_cast<char>(byte));
    }
    return result;
}

void append_checked(std::u16string& output, const std::u16string& suffix) {
    require_native_string(suffix);
    constexpr auto maximum = static_cast<std::size_t>(
        (std::numeric_limits<std::int32_t>::max)());
    if (suffix.size() > maximum - output.size()) {
        throw std::length_error("locale text append exceeds native string range");
    }
    output.append(suffix);
}

} // namespace

char16_t locale_uppercase_00a9eba0(const LocaleTables& tables,
    LocaleTextRuntimeHost& host, char16_t code_unit) {
    const auto& first = tables.sidecar_first();
    const auto& second = tables.sidecar_second();
    for (std::size_t i = 0; i != first.size(); ++i) {
        if (first[i] == static_cast<std::uint16_t>(code_unit)) {
            return static_cast<char16_t>(second.at(i));
        }
    }
    return host.crt_uppercase_00c0391c(code_unit);
}

void LocaleTextResolver::append_key_list_00a9fad0(std::u16string& output,
    const std::string& keys) const {
    require_native_string(keys);
    require_native_string(output);
    if (keys.empty()) {
        return;
    }
    std::size_t begin = 0;
    for (;;) {
        const std::size_t bar = keys.find('|', begin);
        if (bar == std::string::npos) {
            append_key_00a9f4b0(output, keys.substr(begin));
            return;
        }
        append_key_00a9f4b0(output, keys.substr(begin, bar - begin));
        begin = bar + 1;
    }
}

void LocaleTextResolver::append_key_00a9f4b0(std::u16string& output,
    const std::string& key) const {
    require_native_string(key);
    require_native_string(output);
    if (key.empty()) {
        return;
    }
    const std::size_t old_length = output.size();
    const bool uppercase = key.front() == '^';
    std::size_t begin = uppercase ? 1 : 0;
    if (begin == key.size()) {
        return;
    }
    const bool literal = key[begin] == '.';
    if (literal) {
        ++begin;
    }
    if (begin == key.size()) {
        return;
    }
    const std::string stripped = key.substr(begin);
    if (literal) {
        append_checked(output, widen_bytes_004c5e60(stripped));
    } else {
        const auto* text = tables_.find_00a9ec70(stripped);
        append_checked(output, text ? *text : widen_bytes_004c5e60(stripped));
        std::size_t marker = output.find(u'#', old_length);
        while (marker != std::u16string::npos) {
            const std::size_t close = output.find(u'#', marker + 1);
            if (close == std::u16string::npos) {
                // Native close=-1 wraps close+1 to zero, repeatedly reinserting
                // the complete original output. It has no terminating result.
                throw std::invalid_argument("unmatched locale substitution marker");
            }
            const std::string path = narrow_low_bytes_00436630(
                output.substr(marker + 1, close - marker - 1));
            std::u16string replacement;
            if (path.empty()) {
                replacement = u"\n";
            } else if (path == ":") {
                replacement = u"#";
            } else {
                const std::string value = host_.context_string_00b692c0(path);
                append_key_list_00a9fad0(replacement, value);
            }
            std::u16string joined = output.substr(0, marker);
            append_checked(joined, replacement);
            append_checked(joined, output.substr(close + 1));
            output.swap(joined);
            // The recursive call already resolved replacement. Do not scan it
            // again: '#:#' must survive as '#', and '.literal' stays literal.
            marker = output.find(u'#', marker + replacement.size());
        }
    }
    if (uppercase) {
        for (std::size_t i = old_length; i < output.size(); ++i) {
            output[i] = locale_uppercase_00a9eba0(tables_, host_, output[i]);
        }
    }
}

std::u16string LocaleTextResolver::resolve(const std::string& keys) const {
    std::u16string result;
    append_key_list_00a9fad0(result, keys);
    return result;
}

} // namespace bsp
