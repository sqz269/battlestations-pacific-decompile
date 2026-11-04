#include "bsp/archive_text_writer.hpp"
#include "bsp/cc13_front5.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/wire_goal4_chain542_switches.hpp"

#include <array>
#include <cstring>
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

// Chain 542: 00BD6990/6AC0/6BD0/7B30/7F50 operate on the writer's actual
// depth, keys, scalar values and output destination. Their native strings
// borrow the same process pool as the rest of the application. The provider
// boundary below preserves the existing supported settings domain (tags 0..3),
// checked FILE failures and explicit host byte sink; it supplies no Lua/VFS
// stand-ins and does not claim the Original CRT/stack/EH ABI.
class ArchiveEmitter final : public cc13_front5::ArchiveEmitHost {
    using NativeStr = cc13_front5::NativeStr;
public:
    explicit ArchiveEmitter(ArchiveTextOutput& output)
        : strings_(game::game_native_string_process()) {
        if (output.compact_encoded_0109cee4 && !output.file_0109cee0)
            throw std::logic_error("encoded archive output requires a FILE");
        if (!output.file_0109cee0 && !output.buffered_0109cecc)
            throw std::logic_error("archive output has no FILE or buffered sink");
        globals_ = {output.file_0109cee0,
            static_cast<std::uint8_t>(output.compact_encoded_0109cee4),
            output.buffered_0109cecc, ""}; // 0109CEE5 is the PE's NUL byte.
    }
    const cc13_front5::ArchiveWriterGlobals& globals() const noexcept { return globals_; }

    void emit(std::string_view bytes) {
        NativeStr owned{0, nullptr};
        assign_bytes(owned, bytes);
        cc13_front5::archive_emit_string_00bd6bd0(owned, globals_, *this);
        // 00BD6BD0 owns and releases the argument, including its encoded copy.
    }

    NativeStr* native_string_assign_0041e870(NativeStr* self, const char* text) override {
        construct_native_string_header_0041e870(self, strings_.raw_context(), require_text(text));
        return self;
    }
    void native_string_resize_0041dd40(NativeStr* self, std::uint32_t length,
        std::uint32_t preserve) override {
        resize_native_string_header_0041dd40(self, strings_.raw_context(), length, preserve != 0);
    }
    void native_string_destroy_0041dd20(NativeStr* self) override {
        destroy_native_string_header_0041dd20(self, strings_.raw_context());
    }
    void* sized_pool_00419cc0() override {
        return native_string_pool_get_or_create_00419cc0(strings_.pool_01090aa8(),
            strings_.manager_01090aa0());
    }
    void sized_pool_return_00bd1510(void* pool, void* block, std::uint32_t size,
        std::uint32_t) override {
        return_native_string_pool_00bd1510(static_cast<NativeStringPoolStorage*>(pool),
            block, size, strings_.returns_disabled_01090aa4());
    }
    std::size_t fwrite_00bfb08b(const void* data, std::size_t size,
        std::size_t count, std::FILE* file) override {
        const auto written = std::fwrite(data, size, count, file);
        if (size != 0 && written != count) throw std::runtime_error("archive fwrite failed");
        return written;
    }
    void save_storage_append_00bd49b0(void* storage, const NativeStr* bytes) override {
        // This is the existing explicitly supplied uncompressed byte sink,
        // not an invented native compressed-block manager at 0109CECC.
        static_cast<ArchiveByteSink*>(storage)->write(view(*bytes));
    }
    std::int32_t object_id_fn_0109cedc(void*) override {
        // ArchiveTextWriter validates its supported scalar tags before output.
        throw std::logic_error("object archive values require an object-id provider");
    }
    NativeStr* native_string_concat_004261a0(const NativeStr* left, NativeStr* out,
        const NativeStr* right) override {
        const std::string joined = std::string(view(*left)) + std::string(view(*right));
        *out = {0, nullptr};
        assign_bytes(*out, joined);
        return out;
    }
    void native_string_append_int_004caca0(NativeStr* self, std::int32_t value) override {
        append(*self, integer_text(value));
    }
    void native_string_append_float_00781740(NativeStr* self, float value) override {
        append(*self, float_text(value));
    }
    void string_replace_substrings_004cad40(NativeStr* self, const NativeStr* find,
        const NativeStr* replacement, std::uint32_t count) override {
        const std::string needle(view(*find)), inserted(view(*replacement));
        if (needle.empty()) throw std::logic_error("archive escape pattern is empty");
        std::string text(view(*self));
        std::size_t position = 0;
        while (count != 0 && (position = text.find(needle, position)) != std::string::npos) {
            text.replace(position, needle.size(), inserted);
            position += inserted.size();
            --count;
        }
        assign_bytes(*self, text);
    }
    NativeStr* native_string_concat_float_006af2b0(const NativeStr* left, NativeStr* out,
        float value) override {
        const std::string joined = std::string(view(*left)) + float_text(value);
        *out = {0, nullptr};
        assign_bytes(*out, joined);
        return out;
    }
    NativeStr* native_string_concat_int_004263b0(const NativeStr* left, NativeStr* out,
        std::int32_t value) override {
        const std::string joined = std::string(view(*left)) + integer_text(value);
        *out = {0, nullptr};
        assign_bytes(*out, joined);
        return out;
    }
    void native_string_append_00425e10(NativeStr* self, const NativeStr* source) override {
        append(*self, view(*source));
    }
    void native_string_assign_header_00425f40(NativeStr* self, const NativeStr* source) override {
        if (self != source) assign_bytes(*self, view(*source));
    }

private:
    static std::string_view view(const NativeStr& value) {
        return {value.data != nullptr ? value.data : "", value.length};
    }
    void assign_bytes(NativeStr& target, std::string_view bytes) {
        if (bytes.size() >= std::numeric_limits<std::uint32_t>::max())
            throw std::length_error("archive fragment exceeds native string length");
        // Snapshot a possibly aliased source before resize can return its block.
        const std::string copy(bytes);
        resize_native_string_header_0041dd40(&target, strings_.raw_context(),
            static_cast<std::uint32_t>(copy.size()), true);
        if (!copy.empty()) std::memcpy(target.data, copy.data(), copy.size());
    }
    void append(NativeStr& target, std::string_view bytes) {
        const std::string joined = std::string(view(target)) + std::string(bytes);
        assign_bytes(target, joined);
    }
    game::GameNativeStringProcess& strings_;
    cc13_front5::ArchiveWriterGlobals globals_;
};

cc13_front5::TaggedValue archive_key(const GuiLuaVariant& key, bool root) {
    using Kind = GuiLuaKeyKind;
    if (root && key.tag != static_cast<std::int32_t>(Kind::Name))
        throw std::invalid_argument("root archive key requires a name");
    cc13_front5::TaggedValue result{static_cast<std::uint32_t>(key.tag), 0};
    switch (static_cast<Kind>(key.tag)) {
    case Kind::Name:
        result.payload = reinterpret_cast<std::uint32_t>(require_text(key.value.text));
        break;
    case Kind::Index: result.payload = static_cast<std::uint32_t>(key.value.integer); break;
    case Kind::FloatIndex: std::memcpy(&result.payload, &key.value.number, 4); break;
    default: throw std::invalid_argument("unsupported archive key tag");
    }
    return result;
}

cc13_front5::TaggedValue archive_value(const SettingsValue& value) {
    cc13_front5::TaggedValue result{};
    switch (value.type) {
    case SettingsValueType::String:
        result = {0, reinterpret_cast<std::uint32_t>(require_text(value.text))};
        break;
    case SettingsValueType::Int: result = {1, static_cast<std::uint32_t>(value.integer)}; break;
    case SettingsValueType::Float:
        result.tag = 2;
        std::memcpy(&result.payload, &value.real, 4);
        break;
    case SettingsValueType::Bool: result = {3, value.boolean ? 1u : 0u}; break;
    default: throw std::invalid_argument("unsupported archive scalar tag");
    }
    return result;
}

} // namespace

void ArchiveStringSink::write(std::string_view bytes) {
    if (!bytes.empty()) bytes_.append(bytes.data(), bytes.size());
}

void ArchiveTextOutput::emit(std::string_view bytes) {
    if constexpr (kWireGoal4Chain542Bound) {
        ArchiveEmitter emitter(*this);
        emitter.emit(bytes);
    } else {
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
    if constexpr (kWireGoal4Chain542Bound) {
        ArchiveEmitter emitter(output_);
        cc13_front5::archive_emit_indent_00bd6990(depth_, emitter.globals(), emitter);
    } else {
    //00BD6990 fills with spaces, then overwrites the entire string with09h.
    output_.emit(std::string(depth_, '\t'));
    }
}

void ArchiveTextWriter::terminate_row() {
    if constexpr (kWireGoal4Chain542Bound) {
        ArchiveEmitter emitter(output_);
        cc13_front5::archive_emit_terminator_00bd6ac0(depth_ == 0, emitter.globals(), emitter);
    } else {
    output_.emit(depth_ == 0 ? "\n" : ",\n");
    }
}

void ArchiveTextWriter::begin_section(const char* name) {
    begin_section(gui_lua_key_by_name(name));
}

void ArchiveTextWriter::begin_section(const GuiLuaVariant& key) {
    if (depth_ == std::numeric_limits<std::uint32_t>::max())
        throw std::overflow_error("archive section depth overflow");
    if constexpr (kWireGoal4Chain542Bound) {
        const auto native_key = archive_key(key, depth_ == 0);
        indent();
        ArchiveEmitter emitter(output_);
        cc13_front5::archive_emit_key_00bd7b30(depth_ == 0, native_key, emitter.globals(), emitter);
        output_.emit("{\n");
        ++depth_;
    } else {
    const std::string formatted_key = format_key(key);
    indent();
    output_.emit(formatted_key);
    output_.emit("{\n");
    ++depth_;
    }
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
    if constexpr (kWireGoal4Chain542Bound) {
        const auto native_key = archive_key(key, depth_ == 0);
        const auto native_value = archive_value(value);
        indent();
        ArchiveEmitter emitter(output_);
        cc13_front5::archive_emit_key_00bd7b30(depth_ == 0, native_key, emitter.globals(), emitter);
        cc13_front5::archive_emit_value_00bd7f50(native_value, emitter.globals(), emitter);
        terminate_row();
    } else {
    const std::string formatted_key = format_key(key);
    const std::string formatted_value = scalar_text(value);
    indent();
    output_.emit(formatted_key);
    output_.emit(formatted_value);
    terminate_row();
    }
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
