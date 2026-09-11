#include "bsp/locale_tables.hpp"
#include "bsp/language_catalog.hpp"

#include <algorithm>
#include <cstring>

namespace bsp {
namespace {

const std::string& empty_string() {
    static const std::string value;  // native 00f88a3c / 00ce3a0c
    return value;
}

// 00a9fc30 folds only 'A'..'Z' with the `(byte)(c + 0xbf) < 0x1a` test and
// accumulates the result as a signed char, so bytes >= 0x80 subtract.
std::uint32_t fold_and_accumulate(std::uint32_t hash, unsigned char byte) noexcept {
    unsigned char folded = byte;
    if (static_cast<unsigned char>(byte + 0xbfu) < 0x1au) {
        folded = static_cast<unsigned char>(byte + 0x20u);
    }
    const std::int32_t as_signed = static_cast<std::int8_t>(folded);
    return hash * 0x83u + static_cast<std::uint32_t>(as_signed);
}

// __stricmp on the native side; both keys are byte strings from the table file.
bool equal_case_insensitive(const std::string& left, const std::string& right) noexcept {
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t i = 0; i < left.size(); ++i) {
        unsigned char a = static_cast<unsigned char>(left[i]);
        unsigned char b = static_cast<unsigned char>(right[i]);
        if (a >= 'A' && a <= 'Z') {
            a = static_cast<unsigned char>(a + 0x20u);
        }
        if (b >= 'A' && b <= 'Z') {
            b = static_cast<unsigned char>(b + 0x20u);
        }
        if (a != b) {
            return false;
        }
    }
    return true;
}

std::string decimal(std::size_t value) {
    // 004260b0 formats with sprintf("%d").
    char buffer[16] = {};
    std::size_t length = 0;
    if (value == 0) {
        buffer[length++] = '0';
    }
    while (value != 0 && length < sizeof(buffer)) {
        buffer[length++] = static_cast<char>('0' + (value % 10));
        value /= 10;
    }
    std::string text(buffer, length);
    std::reverse(text.begin(), text.end());
    return text;
}

}  // namespace

LanguageEntry parse_language_descriptor_008d7bc0(const std::string& text) {
    return parse_native_language_descriptor_008d7bc0(
        std::vector<std::uint8_t>(text.begin(), text.end()));
}

const std::string& language_name_008d4870(const std::vector<LanguageEntry>& table,
    std::size_t language_index) noexcept {
    if (language_index >= table.size()) {
        return empty_string();
    }
    const std::string& name = table[language_index].lanfile;
    return name.empty() ? empty_string() : name;
}

const std::string& language_font_path_008d4890(const std::vector<LanguageEntry>& table,
    std::size_t language_index) noexcept {
    if (language_index >= table.size()) {
        return empty_string();
    }
    const std::string& path = table[language_index].font_path;
    return path.empty() ? empty_string() : path;
}

bool parse_lockit_table_00aa0020(const std::vector<std::uint8_t>& bytes,
    std::vector<LocaleRecord>& output, std::string& error) {
    error.clear();
    std::size_t position = 0;
    const std::size_t size = bytes.size();

    const auto read_token = [&bytes, &position, size](std::string& token) {
        const std::size_t start = position;
        while (position < size && bytes[position] != ' ') {
            ++position;
        }
        if (position >= size) {
            return false;
        }
        token.assign(reinterpret_cast<const char*>(bytes.data()) + start, position - start);
        ++position;  // the separating space, overwritten with NUL natively
        return true;
    };

    std::string category;
    std::string name;
    while (position < size) {
        const std::size_t record_start = position;
        // Native checks EOF immediately after each first-token byte read,
        // before checking whether that byte was a space.
        if (!read_token(category) || position == size) {
            return true;
        }
        if (!read_token(name)) {
            error = "truncated record header at offset " + decimal(record_start);
            return false;
        }
        // The composed key goes into a 152-byte stack buffer natively; the
        // separator between the two tokens is the "." at 00ce3a70.
        if (category.size() + name.size() + 2 > kLockitKeyBufferBytes) {
            error = "record key at offset " + decimal(record_start) +
                " exceeds the native key buffer";
            return false;
        }

        const std::size_t text_start = position;
        std::size_t text_end = size;
        std::uint16_t previous = 0;
        bool terminated = false;
        while (position + 1 < size) {
            const std::uint16_t unit = static_cast<std::uint16_t>(
                bytes[position] | (static_cast<std::uint16_t>(bytes[position + 1]) << 8));
            if (previous == 0x0a00u && unit == 0x0d00u) {
                text_end = position - 2;
                position += 2;
                terminated = true;
                break;
            }
            previous = unit;
            position += 2;
        }
        if (!terminated) {
            error = "unterminated record text at offset " + decimal(record_start);
            return false;
        }
        if (text_end - text_start > kLockitValueBufferBytes) {
            error = "record text at offset " + decimal(record_start) +
                " exceeds the native text buffer";
            return false;
        }

        const auto category_nul = category.find('\0');
        if (category_nul != std::string::npos) category.resize(category_nul);
        const auto name_nul = name.find('\0');
        if (name_nul != std::string::npos) name.resize(name_nul);
        LocaleRecord record;
        record.key.reserve(category.size() + 1 + name.size());
        record.key = category;
        record.key += '.';
        record.key += name;
        record.text.reserve((text_end - text_start) / 2);
        for (std::size_t at = text_start; at + 1 < text_end; at += 2) {
            if (bytes[at] == 0 && bytes[at + 1] == 0) break;
            record.text.push_back(static_cast<char16_t>(
                bytes[at] | (static_cast<std::uint16_t>(bytes[at + 1]) << 8)));
        }
        output.push_back(std::move(record));
    }
    return true;
}

bool parse_lockit_index_00aa0020(const std::vector<std::uint8_t>& bytes,
    std::vector<std::uint16_t>& first, std::vector<std::uint16_t>& second,
    std::string& error) {
    error.clear();
    if (bytes.empty()) return true;
    if (bytes.size() < 2) {
        error = "sidecar has a partial two-byte prefix";
        return false;
    }
    for (std::size_t at = 2; at + 3 < bytes.size(); at += 4) {
        first.push_back(static_cast<std::uint16_t>(
            bytes[at] | (static_cast<std::uint16_t>(bytes[at + 1]) << 8)));
        second.push_back(static_cast<std::uint16_t>(
            bytes[at + 2] | (static_cast<std::uint16_t>(bytes[at + 3]) << 8)));
    }
    if ((bytes.size() - 2) % 4 != 0) {
        error = "sidecar ends with a partial 16-bit pair";
        return false;
    }
    return true;
}

std::string lockit_table_path_00aa09d0(const std::string& language) {
    std::string path = "lockit/";
    path += language;
    path += ".lan";
    return path;
}

std::uint32_t LocaleTables::bucket_00a9fc30(const std::string& key) noexcept {
    std::uint32_t hash = 0;
    for (const char character : key) {
        hash = fold_and_accumulate(hash, static_cast<unsigned char>(character));
    }
    return hash & 0xfffu;
}

void LocaleTables::register_table_00aa0d30(std::string table_name) {
    tables_.push_back(std::move(table_name));
}

void LocaleTables::clear_0073c240() noexcept {
    storage_.clear();
    for (std::size_t index = 0; index < kBucketCount; ++index) {
        buckets_[index] = nullptr;
    }
    count_ = 0;
}

void LocaleTables::insert_00a9fc30(std::string key, std::u16string text) {
    const std::uint32_t index = bucket_00a9fc30(key);
    for (Node* node = buckets_[index]; node != nullptr; node = node->next) {
        if (equal_case_insensitive(node->key, key)) {
            node->text = std::move(text);  // repeated key overwrites
            return;
        }
    }
    auto node = std::make_unique<Node>();
    node->key = std::move(key);
    node->text = std::move(text);
    node->next = buckets_[index];
    buckets_[index] = node.get();
    storage_.push_back(std::move(node));
    ++count_;
}

const std::u16string* LocaleTables::find_00a9ec70(const std::string& key) const noexcept {
    const std::uint32_t index = bucket_00a9fc30(key);
    for (const Node* node = buckets_[index]; node != nullptr; node = node->next) {
        if (equal_case_insensitive(node->key, key)) {
            return &node->text;
        }
    }
    return nullptr;
}

std::vector<std::string> LocaleTables::split_lookup_key_00a9fad0(const std::string& key) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (start <= key.size()) {
        const std::size_t bar = key.find('|', start);
        if (bar == std::string::npos) {
            if (start < key.size()) {
                parts.push_back(key.substr(start));
            }
            break;
        }
        parts.push_back(key.substr(start, bar - start));
        start = bar + 1;
    }
    return parts;
}

bool LocaleTables::load_one_file(LocaleTableSource& source, const std::string& name,
    bool skip_sidecar, std::string& error) {
    auto files = source.override_paths_00bdef90(name);
    // 00aa0077 appends suffix variants; 00aa0088 appends the original last.
    files.push_back(name);
    for (const auto& file : files) {
        std::vector<std::uint8_t> bytes;
        if (!source.read_file(file, 2, bytes)) {
            error = file + ": locale table stream did not open";
            return false;
        }
        loaded_files_.push_back(file);
        std::vector<LocaleRecord> records;
        const bool parsed = parse_lockit_table_00aa0020(bytes, records, error);
        // Native inserts at every completed record, before later malformed
        // input can fail. Do not make the whole file a transaction.
        for (LocaleRecord& record : records) {
            insert_00a9fc30(std::move(record.key), std::move(record.text));
        }
        if (!parsed) {
            error = file + ": " + error;
            return false;
        }
    }
    if (skip_sidecar) return true;

    // One sidecar for the original unnumbered name, after all variants/base.
    // 00aa0379..0410 clears both vectors before the unconditional open.
    sidecar_first_.clear();
    sidecar_second_.clear();
    std::vector<std::uint8_t> sidecar;
    if (!source.read_file(name + "x", 2, sidecar)) {
        error = name + "x: locale sidecar stream did not open";
        return false;
    }
    if (!parse_lockit_index_00aa0020(sidecar, sidecar_first_, sidecar_second_, error)) {
        error = name + "x: " + error;
        return false;
    }
    return true;
}

bool LocaleTables::load_current_language(LocaleTableSource& source, std::string& error) {
    error.clear();
    loaded_files_.clear();
    const std::string base = lockit_table_path_00aa09d0(language_);
    if (!load_one_file(source, base, false, error)) {
        return false;
    }
    for (std::size_t index = 0; index < kMaxNumberedSuffix; ++index) {
        const std::string numbered = base + decimal(index);
        if (!source.resolves_existing_name_00bdf4c0(numbered)) {
            break;  // the native loop breaks on the first name that does not resolve
        }
        if (!load_one_file(source, numbered, true, error)) {
            return false;
        }
    }
    return true;
}

bool LocaleTables::set_language_00aa09d0(LocaleTableSource& source,
    LocaleGuiRefreshHost& gui, const std::string& language, bool& changed,
    std::string& error) {
    error.clear();
    changed = false;
    if (equal_case_insensitive(language_, language)) {
        return true;
    }
    changed = true;
    language_ = language;
    clear_0073c240();
    if (tables_.empty()) {
        return true;  // the native gate on the registered-name vector
    }
    if (!load_current_language(source, error)) return false;
    gui.refresh_locale_00aa4650();
    return true;
}

bool LocaleTables::reload_00aa06d0(LocaleTableSource& source, bool force,
    std::string& error) {
    error.clear();
    if (count_ != 0 && !force) {
        return true;
    }
    return load_current_language(source, error);
}

}  // namespace bsp
