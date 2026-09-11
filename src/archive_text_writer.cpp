#include "bsp/archive_text_writer.hpp"

#include <array>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {

const char* require_text(const char* text) {
    if (!text) throw std::invalid_argument("archive text pointer is null");
    return text;
}

std::string integer_text(std::int32_t value) {
    //004CACA0 appends004260B0: sprintf("%d", value).
    std::array<char, 52> buffer{};
    const int count = std::snprintf(buffer.data(), buffer.size(), "%d", value);
    if (count < 0 || static_cast<std::size_t>(count) >= buffer.size())
        throw std::runtime_error("archive integer formatting failed");
    return {buffer.data(), static_cast<std::size_t>(count)};
}

std::string float_text(float value) {
    //00781740 appends0043BDF0: FLD float/FSTP double, sprintf("%f").
    //52 bytes also suffice for every finite binary32 value in fixed notation.
    std::array<char, 52> buffer{};
    const int count = std::snprintf(buffer.data(), buffer.size(), "%f",
        static_cast<double>(value));
    if (count < 0 || static_cast<std::size_t>(count) >= buffer.size())
        throw std::runtime_error("archive float formatting failed");
    return {buffer.data(), static_cast<std::size_t>(count)};
}

std::string scalar_text(const SettingsValue& value) {
    switch (value.type) {
    case SettingsValueType::String: {
        //00BD7F50 replaces backslashes first, quotes second, then wraps quotes.
        //A bytewise escape pass produces those same bytes, including raw LF/TAB.
        std::string result(1, '"');
        for (const char* p = require_text(value.text); *p; ++p) {
            if (*p == '\\' || *p == '"') result += '\\';
            result += *p;
        }
        result += '"';
        return result;
    }
    case SettingsValueType::Int: return integer_text(value.integer);
    case SettingsValueType::Float: return float_text(value.real);
    case SettingsValueType::Bool: return value.boolean ? "true" : "false";
    }
    throw std::invalid_argument("unsupported archive scalar tag");
}

void file_fragment(std::FILE* file, std::string_view bytes) {
    if (!bytes.empty() && std::fwrite(bytes.data(), bytes.size(), 1, file) != 1)
        throw std::runtime_error("archive fwrite failed");
}

} // namespace

void ArchiveStringSink::write(std::string_view bytes) {
    if (!bytes.empty()) bytes_.append(bytes.data(), bytes.size());
}

void ArchiveTextOutput::emit(std::string_view bytes) {
    if (compact_encoded_0109cee4) {
        if (!file_0109cee0)
            throw std::logic_error("encoded archive output requires a FILE");
        //00BD6100 strips these bytes everywhere, even inside quoted text.
        //SAR CL,4 sign-extends; ordinary nibble rotation is wrong for>=80h.
        for (unsigned char byte : bytes) {
            if (byte == ' ' || byte == '\r' || byte == '\n' || byte == '\t') continue;
            const unsigned int shifted = (byte >> 4) | ((byte & 0x80u) ? 0xf0u : 0u);
            const char encoded = static_cast<char>(shifted | ((byte << 4) & 0xffu));
            file_fragment(file_0109cee0, std::string_view(&encoded, 1));
        }
    } else if (file_0109cee0) {
        file_fragment(file_0109cee0, bytes);
    } else if (buffered_0109cecc) {
        buffered_0109cecc->write(bytes);
    } else {
        throw std::logic_error("archive output has no FILE or buffered sink");
    }
}

ArchiveTextWriter::ArchiveTextWriter(ArchiveTextOutput& output,
    ArchiveSettingsServices* services) noexcept
    : output_(output), settings_services_(services) {}

std::string ArchiveTextWriter::format_key(const GuiLuaVariant& key) const {
    //00BD7B30 takes root flag in CL and an8h variant on the stack.
    if (depth_ == 0) {
        //Native ignores tag here and treats payload as a C-string pointer.
        if (key.tag != static_cast<std::int32_t>(GuiLuaKeyKind::Name))
            throw std::invalid_argument("root archive key requires a name");
        return std::string(require_text(key.value.text)) + " = ";
    }
    std::string result = "[";
    switch (static_cast<GuiLuaKeyKind>(key.tag)) {
    case GuiLuaKeyKind::Name:
        //Unlike values, native key text receives no escaping at all.
        result += '"';
        result += require_text(key.value.text);
        result += '"';
        break;
    case GuiLuaKeyKind::Index: result += integer_text(key.value.integer); break;
    case GuiLuaKeyKind::FloatIndex: result += float_text(key.value.number); break;
    default: throw std::invalid_argument("unsupported archive key tag");
    }
    result += "] = ";
    return result;
}

void ArchiveTextWriter::indent() {
    //00BD6990 fills with spaces, then overwrites the entire string with09h.
    output_.emit(std::string(depth_, '\t'));
}

void ArchiveTextWriter::terminate_row() {
    output_.emit(depth_ == 0 ? "\n" : ",\n");
}

void ArchiveTextWriter::begin_section(const char* name) {
    begin_section(gui_lua_key_by_name(name));
}

void ArchiveTextWriter::begin_section(const GuiLuaVariant& key) {
    if (depth_ == std::numeric_limits<std::uint32_t>::max())
        throw std::overflow_error("archive section depth overflow");
    const std::string formatted_key = format_key(key);
    indent();
    output_.emit(formatted_key);
    output_.emit("{\n");
    ++depth_;
}

void ArchiveTextWriter::end_section() {
    if (!depth_) throw std::logic_error("archive section depth underflow");
    --depth_;
    indent();
    output_.emit("}");
    terminate_row();
}

void ArchiveTextWriter::write_field(const char* key, const SettingsValue& value) {
    write_field(gui_lua_key_by_name(key), value);
}

void ArchiveTextWriter::write_field(const GuiLuaVariant& key, const SettingsValue& value) {
    const std::string formatted_key = format_key(key);
    const std::string formatted_value = scalar_text(value);
    indent();
    output_.emit(formatted_key);
    output_.emit(formatted_value);
    terminate_row();
}

void ArchiveTextWriter::write_options_text_008d6170() {
    if (!settings_services_)
        throw std::logic_error("archive settings services were not supplied");
    settings_services_->write_options_text_008d6170();
}

void ArchiveTextWriter::write_keyboard_setup() {
    if (!settings_services_)
        throw std::logic_error("archive settings services were not supplied");
    settings_services_->write_keyboard_setup(*this);
}

} // namespace bsp
