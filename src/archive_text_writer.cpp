#include "bsp/archive_text_writer.hpp"
#include "bsp/cc13_front5.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/main_menu_selection_listener.hpp"
#include "bsp/mp_seg43_cc13.hpp"
#include "bsp/native_particle_texture_names_raw.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_append.hpp"
#include "bsp/native_string_float_format.hpp"
#include "bsp/oct18_tail_c.hpp"
#include "bsp/pool_pool15_cc13.hpp"

#include <array>
#include <cstddef>
#include <cstring>
#include <exception>
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
class ArchiveEmitter final : public cc13_front5::ArchiveEmitHost,
    public pool15_cc13::NativeStringAppendIntHost,
    public cc13_seg43::NativeStringFloatFormatHost {
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

    template<class Emit> void run(Emit emit) {
        emit();
        // Output errors are held inside their providers until the native body
        // has finished all trailing releases. This includes both the original
        // string and emit_text's separate by-value copy in compact mode.
        // The host error then escapes at the same public emitter boundary;
        // no cleanup repeats a native release or reads an unwound local header.
        if (output_failure_) std::rethrow_exception(output_failure_);
    }

    void emit(std::string_view bytes) {
        NativeStr owned{0, nullptr};
        assign_bytes(owned, bytes);
        run([&] { cc13_front5::archive_emit_string_00bd6bd0(owned, globals_, *this); });
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
    void native_string_resize_0041dd40(void* self, std::uint32_t length,
        std::uint32_t preserve) override {
        resize_native_string_header_0041dd40(self, strings_.raw_context(), length, preserve != 0);
    }
    void native_string_from_int_004260b0(void* header, std::int32_t value) override {
        NativeString fresh;
        bsp::native_string_from_int_004260b0(fresh, value, strings_.strings());
        // Transfer the resulting 8-byte header to the native caller's fresh
        // storage. NativeString has no implicit buffer-releasing destructor.
        static_assert(sizeof fresh == sizeof(NativeStr));
        std::memcpy(header, &fresh, sizeof fresh);
    }
    void native_string_from_float_0043bdf0(void* header, float value) override {
        construct_native_string_float_header_0043bdf0(header, strings_.raw_context(), value);
    }
    void memcpy_00bf7680(void* target, const void* source, std::uint32_t size) override {
        if (size != 0) std::memcpy(target, source, size);
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
    void sized_pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size,
        std::uint32_t flag) override {
        sized_pool_return_00bd1510(pool, block, size, flag);
    }
    std::size_t fwrite_00bfb08b(const void* data, std::size_t size,
        std::size_t count, std::FILE* file) override {
        if (output_failure_) return 0;
        try {
            const auto written = std::fwrite(data, size, count, file);
            if (size != 0 && written != count) throw std::runtime_error("archive fwrite failed");
            return written;
        } catch (...) {
            output_failure_ = std::current_exception();
            return 0;
        }
    }
    void save_storage_append_00bd49b0(void* storage, const NativeStr* bytes) override {
        // This is the existing explicitly supplied uncompressed byte sink,
        // not an invented native compressed-block manager at 0109CECC.
        if (output_failure_) return;
        try {
            static_cast<ArchiveByteSink*>(storage)->write(view(*bytes));
        } catch (...) {
            output_failure_ = std::current_exception();
        }
    }
    std::int32_t object_id_fn_0109cedc(void*) override {
        // ArchiveTextWriter validates its supported scalar tags before output.
        throw std::logic_error("object archive values require an object-id provider");
    }
    NativeStr* native_string_concat_004261a0(const NativeStr* left, NativeStr* out,
        const NativeStr* right) override {
        return static_cast<NativeStr*>(concatenate_native_string_headers_004261a0(
            left, out, right, strings_.raw_context()));
    }
    void native_string_append_int_004caca0(NativeStr* self, std::int32_t value) override {
        pool15_cc13::native_string_append_int_004caca0(self, value, *this);
    }
    void native_string_append_float_00781740(NativeStr* self, float value) override {
        cc13_seg43::native_string_append_float_00781740(self, value, *this, strings_.strings());
    }
    void string_replace_substrings_004cad40(NativeStr* self, const NativeStr* find,
        const NativeStr* replacement, std::uint32_t count) override {
        replace_native_particle_string_substrings_004cad40(self, find, replacement,
            count, strings_.raw_context());
    }
    NativeStr* native_string_concat_float_006af2b0(const NativeStr*, NativeStr*, float) override {
        // Only extended value tags 5..8 use this operation; the public writer
        // admits scalar tags 0..3. Do not duplicate an unused native body.
        throw std::logic_error("extended archive values require an extended-value provider");
    }
    NativeStr* native_string_concat_int_004263b0(const NativeStr*, NativeStr*, std::int32_t) override {
        // Quoted-integer tag 10 is likewise outside the admitted domain.
        throw std::logic_error("extended archive values require an extended-value provider");
    }
    void native_string_append_00425e10(NativeStr* self, const NativeStr* source) override {
        append_native_string_00425e10(self, source, strings_.strings());
    }
    void native_string_assign_header_00425f40(NativeStr* self, const NativeStr* source) override {
        assign_native_string_header_00425f40(self, source, strings_.raw_context());
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
    game::GameNativeStringProcess& strings_;
    cc13_front5::ArchiveWriterGlobals globals_;
    std::exception_ptr output_failure_;
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

// 00BD8CD0's explicit bindings have native-style context-free callbacks. Bind
// the real destination for this invocation, restoring it on nested writes and
// exceptions. Each dependency captures the current output globals afresh, as
// the native emitters do. No writer depth or key/value payload is synthesized.
class ArchiveFieldScope final {
public:
    explicit ArchiveFieldScope(ArchiveTextOutput& output) noexcept
        : previous_(current_) { current_ = &output; }
    ~ArchiveFieldScope() { current_ = previous_; }
    ArchiveFieldScope(const ArchiveFieldScope&) = delete;
    ArchiveFieldScope& operator=(const ArchiveFieldScope&) = delete;

    static const oct18_tail_c::ArchiveFieldBindings& bindings() noexcept {
        static const oct18_tail_c::ArchiveFieldBindings calls{
            [](std::uint32_t depth) {
                ArchiveEmitter emitter(*current_);
                emitter.run([&] {
                    cc13_front5::archive_emit_indent_00bd6990(depth, emitter.globals(), emitter);
                });
            },
            [](std::uint8_t root, oct18_tail_c::GuiTaggedWord key) {
                // Keep the public writer's supported-key boundary even if a
                // reentrant sink changes the depth before BD8CEC reloads it.
                if (root && key.tag != static_cast<std::uint32_t>(GuiLuaKeyKind::Name))
                    throw std::invalid_argument("root archive key requires a name");
                ArchiveEmitter emitter(*current_);
                emitter.run([&] {
                    cc13_front5::archive_emit_key_00bd7b30(root, {key.tag, key.value},
                        emitter.globals(), emitter);
                });
            },
            [](oct18_tail_c::GuiTaggedWord value) {
                ArchiveEmitter emitter(*current_);
                emitter.run([&] {
                    cc13_front5::archive_emit_value_00bd7f50({value.tag, value.value},
                        emitter.globals(), emitter);
                });
            },
            [](std::uint8_t root) {
                ArchiveEmitter emitter(*current_);
                emitter.run([&] {
                    cc13_front5::archive_emit_terminator_00bd6ac0(root, emitter.globals(), emitter);
                });
            }
        };
        return calls;
    }

private:
    static thread_local ArchiveTextOutput* current_;
    ArchiveTextOutput* previous_;
};
thread_local ArchiveTextOutput* ArchiveFieldScope::current_{};

} // namespace

void ArchiveStringSink::write(std::string_view bytes) {
    if (!bytes.empty()) bytes_.append(bytes.data(), bytes.size());
}

void ArchiveTextOutput::emit(std::string_view bytes) {
    ArchiveEmitter emitter(*this);
    emitter.emit(bytes);
}

ArchiveTextWriter::ArchiveTextWriter(ArchiveTextOutput& output,
    ArchiveSettingsServices* services) noexcept
    : output_(output), settings_services_(services) {
    static_assert(offsetof(NativeState, depth_04) == 4 && sizeof(NativeState) == 8);
}

std::string ArchiveTextWriter::format_key(const GuiLuaVariant& key) const {
    //00BD7B30 takes root flag in CL and an8h variant on the stack.
    if (native_.depth_04 == 0) {
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
    ArchiveEmitter emitter(output_);
    emitter.run([&] { cc13_front5::archive_emit_indent_00bd6990(native_.depth_04, emitter.globals(), emitter); });
}

void ArchiveTextWriter::terminate_row() {
    ArchiveEmitter emitter(output_);
    emitter.run([&] { cc13_front5::archive_emit_terminator_00bd6ac0(native_.depth_04 == 0, emitter.globals(), emitter); });
}

void ArchiveTextWriter::begin_section(const char* name) {
    begin_section(gui_lua_key_by_name(name));
}

void ArchiveTextWriter::begin_section(const GuiLuaVariant& key) {
    if (native_.depth_04 == std::numeric_limits<std::uint32_t>::max())
        throw std::overflow_error("archive section depth overflow");
    const auto native_key = archive_key(key, native_.depth_04 == 0);
    indent();
    ArchiveEmitter emitter(output_);
    emitter.run([&] { cc13_front5::archive_emit_key_00bd7b30(native_.depth_04 == 0, native_key, emitter.globals(), emitter); });
    output_.emit("{\n");
    ++native_.depth_04;
}

void ArchiveTextWriter::end_section() {
    if (!native_.depth_04) throw std::logic_error("archive section depth underflow");
    --native_.depth_04;
    indent();
    output_.emit("}");
    terminate_row();
}

void ArchiveTextWriter::write_field(const char* key, const SettingsValue& value) {
    write_field(gui_lua_key_by_name(key), value);
}

void ArchiveTextWriter::write_field(const GuiLuaVariant& key, const SettingsValue& value) {
    const auto native_key = archive_key(key, native_.depth_04 == 0);
    const auto native_value = archive_value(value);
    ArchiveFieldScope scope(output_);
    // 00BD8CD3/8CDF/8D00 read the retained writer+4 around the four emitter
    // calls (bsp.py show 00bd8cd0 --asm). Replace the inline orchestration,
    // preserving the real scalar payloads, current sink and depth ownership.
    oct18_tail_c::archive_write_field_00bd8cd0(&native_,
        {native_key.tag, native_key.payload}, {native_value.tag, native_value.payload},
        scope.bindings());
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
