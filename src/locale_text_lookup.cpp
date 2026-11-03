#include "bsp/locale_text_lookup.hpp"
#include "bsp/cc13_seg87.hpp"
#include "bsp/native_wide_string.hpp"
#include "bsp/wide_string_substring_view.hpp"
#include "bsp/wire_goal2_chain116_switches.hpp"
#include "bsp/wire_goal2_chain163_switches.hpp"

#include <cstdint>
#include <cwchar>
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

// The resolver already owns the source UTF-16 string. The view borrows that
// backing; only 00770E50's temporary and result allocate, at its native sites.
// This host has no native pool: use the existing explicit CRT storage domain.
class LocaleWideSubstringAdapter final : public WideStringSubstringHost {
public:
    void wide_string_resize_004c53e0(void* value, std::uint32_t length, std::int32_t preserve) override {
        resize_native_wide_string_header_004c53e0(value, storage_, length, preserve != 0);
    }
    void* wide_string_copy_construct_004c8dd0(void* out, const void* source) override {
        return copy_construct_native_wide_string_header_004c8dd0(out, source, storage_);
    }
    wchar_t* crt_wcsncpy_00bfaeef(wchar_t* out, const wchar_t* source, std::uint32_t count) override {
        return std::wcsncpy(out, source, count);
    }
    void* sized_pool_get_00419cc0() override { return &storage_; }
    void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
        std::int32_t) override {
        static_cast<NativeStringStorage*>(pool)->release(static_cast<char*>(data), bytes);
    }
private:
    NativeStringStorage& storage_ = crt_string_storage();
};

std::u16string locale_substring(const std::u16string& source, std::size_t position,
    std::uint32_t count) {
    WideStringSubstringHeader input{static_cast<std::uint32_t>(source.size()),
        reinterpret_cast<wchar_t*>(const_cast<char16_t*>(source.data()))};
    struct Result {
        WideStringSubstringHeader header{};
        ~Result() { destroy_native_wide_string_header_00436430(&header, crt_string_storage()); }
    } result;
    LocaleWideSubstringAdapter adapter;
    wide_string_substring_view_00770e50(input, result.header,
        static_cast<std::int32_t>(position), count, adapter);
    if (result.header.data == nullptr) return {};
    return std::u16string(reinterpret_cast<const char16_t*>(result.header.data), result.header.length);
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
        std::size_t marker = [&]() -> std::size_t {
        // 00A9EF60 calls the existing wide first-of body; OFF keeps the host marker search.
        if constexpr (kWireGoal2Chain163Bound) {
            // 00A9F5E2..00A9F5FC: the set is the widened "#" at 00CF828C;
            // the haystack is the same output appended above, starting at its old length.
            // Both headers borrow terminated backing for this call only. The body reads it.
            char16_t separators[] = u"#";
            const NativeWideStringHeader text{static_cast<std::uint32_t>(output.size()),
                output.data()};
            const NativeWideStringHeader set{1u, separators};
            const std::int32_t index = find_first_of_native_wide_string_00a9ef60(
                text, set, static_cast<std::int32_t>(old_length));
            return index < 0 ? std::u16string::npos : static_cast<std::size_t>(index);
        } else {
        std::size_t marker = output.find(u'#', old_length);
        return marker;
        }
        }();
        while (marker != std::u16string::npos) {
            const std::size_t close = output.find(u'#', marker + 1);
            if (close == std::u16string::npos) {
                // Native close=-1 wraps close+1 to zero, repeatedly reinserting
                // the complete original output. It has no terminating result.
                throw std::invalid_argument("unmatched locale substitution marker");
            }
            // 00770E50 calls the substring body at the three slices; OFF keeps the host loop.
            if constexpr (kWireGoal2Chain116Bound) {
                const std::string path = narrow_low_bytes_00436630(locale_substring(output, marker + 1,
                    static_cast<std::uint32_t>(close - marker - 1)));
                std::u16string replacement;
                if (path.empty()) {
                    replacement = u"\n";
                } else if (path == ":") {
                    replacement = u"#";
                } else {
                    const std::string value = host_.context_string_00b692c0(path);
                    append_key_list_00a9fad0(replacement, value);
                }
                std::u16string joined = locale_substring(output, 0, static_cast<std::uint32_t>(marker));
                append_checked(joined, replacement);
                append_checked(joined, locale_substring(output, close + 1, 0x7fffffffu));
                output.swap(joined);
                marker = output.find(u'#', marker + replacement.size());
            } else {
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
