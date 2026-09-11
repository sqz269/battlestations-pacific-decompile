#include "bsp/gui_lua_reader.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace bsp {
namespace {

// The library table at 00D62BB8, in order: {name, opener} pairs, terminated by
// a null opener. The base library's name is the empty string at 00CE3A0C.
constexpr std::string_view kLibraryNames[] = {
    "", "package", "table", "io", "os", "string", "math", "debug",
};

// lua_tolstring (00A67810) converts a number in place with LUAI_NUMFFORMAT and
// hands back nothing for any other non-string. Null is what 00BD63B0 tests.
bool lua_tostring_view(const GuiValue& value, std::string& out) {
    if (value.kind() == GuiValue::Kind::String) {
        out = value.string();
        return true;
    }
    if (value.kind() == GuiValue::Kind::Number) {
        char buffer[48];
        std::snprintf(buffer, sizeof(buffer), "%.14g", value.number());
        out = buffer;
        return true;
    }
    return false;
}

// lua_tonumber (00A67770): a number, or a string the lexer can convert; zero
// for everything else, with no error.
double lua_tonumber_value(const GuiValue& value) noexcept {
    if (value.kind() == GuiValue::Kind::Number) {
        return value.number();
    }
    if (value.kind() == GuiValue::Kind::String) {
        const std::string& text = value.string();
        char* end = nullptr;
        const double parsed = std::strtod(text.c_str(), &end);
        if (end != nullptr && end != text.c_str()) {
            return parsed;
        }
    }
    return 0.0;
}

// __ftol, which the integer accessor 00B66290 applies to lua_tonumber's result:
// truncation toward zero.
std::int32_t narrow_to_int(double number) noexcept {
    if (!(number > -2147483649.0 && number < 2147483648.0)) {
        return 0;
    }
    return static_cast<std::int32_t>(number);
}

float narrow_to_float(double number) noexcept {
    return static_cast<float>(number);
}

// One element of an array-like table. 00BD63B0 reads t[1]..t[n] through
// 00B67720, so a missing index is nil and lua_tonumber turns it into zero.
float table_element(const GuiTable& table, std::size_t index_from_one) noexcept {
    if (index_from_one == 0 || index_from_one > table.array.size()) {
        return 0.0f;
    }
    return narrow_to_float(lua_tonumber_value(table.array[index_from_one - 1]));
}

bool store_float_run(const GuiValue& value, void* dest, std::size_t count) noexcept {
    if (dest == nullptr || !value.is_table() || value.table() == nullptr) {
        return false;
    }
    float* out = static_cast<float*>(dest);
    for (std::size_t i = 0; i < count; ++i) {
        out[i] = table_element(*value.table(), i + 1);
    }
    return true;
}

}  // namespace

// ---------------------------------------------------------------------------
// The interpreter the GUI creates
// ---------------------------------------------------------------------------

std::vector<std::string_view> gui_lua_libraries_00b6a020(std::uint32_t mask) {
    std::vector<std::string_view> opened;
    for (std::uint32_t i = 0; i < 8; ++i) {
        if (i == 0 || (mask & (1u << i)) != 0) {
            opened.push_back(kLibraryNames[i]);
        }
    }
    return opened;
}

// ---------------------------------------------------------------------------
// Keys and fields
// ---------------------------------------------------------------------------

GuiLuaVariant gui_lua_key_by_name(const char* name) noexcept {
    GuiLuaVariant key;
    key.tag = static_cast<std::int32_t>(GuiLuaKeyKind::Name);
    key.value.text = name;
    return key;
}

GuiLuaVariant gui_lua_key_by_index(std::int32_t index) noexcept {
    GuiLuaVariant key;
    key.tag = static_cast<std::int32_t>(GuiLuaKeyKind::Index);
    key.value.integer = index;
    return key;
}

GuiLuaVariant gui_lua_key_by_float(float index) noexcept {
    GuiLuaVariant key;
    key.tag = static_cast<std::int32_t>(GuiLuaKeyKind::FloatIndex);
    key.value.number = index;
    return key;
}

GuiLuaVariant gui_lua_field(GuiLuaFieldType type, void* dest) noexcept {
    GuiLuaVariant field;
    field.tag = static_cast<std::int32_t>(type);
    field.value.pointer = dest;
    return field;
}

std::size_t gui_lua_field_arity(GuiLuaFieldType type) noexcept {
    switch (type) {
        case GuiLuaFieldType::Vec2:
            return 2;
        case GuiLuaFieldType::Vec3:
            return 3;
        case GuiLuaFieldType::Vec4:
            return 4;
        case GuiLuaFieldType::Matrix4:
            return 16;
        default:
            return 0;
    }
}

// ---------------------------------------------------------------------------
// 00BD63B0: one Lua value into one typed field
// ---------------------------------------------------------------------------

bool gui_lua_store_value_00bd63b0(const GuiValue& value, const GuiLuaVariant& field,
                                  GuiLuaHandleResolver* resolver) noexcept {
    void* const dest = field.value.pointer;
    switch (static_cast<GuiLuaFieldType>(field.tag)) {
        case GuiLuaFieldType::String: {
            // 00BD63D4. The native code resizes the NativeString to the length
            // lua_tolstring reported and memcpys; a null pointer resizes it to
            // zero, which is how a non-string clears the field.
            if (dest == nullptr) {
                return false;
            }
            std::string text;
            if (!lua_tostring_view(value, text)) {
                text.clear();
            }
            *static_cast<std::string*>(dest) = text;
            return true;
        }
        case GuiLuaFieldType::Int: {
            if (dest == nullptr) {
                return false;
            }
            *static_cast<std::int32_t*>(dest) = narrow_to_int(lua_tonumber_value(value));
            return true;
        }
        case GuiLuaFieldType::Float: {
            if (dest == nullptr) {
                return false;
            }
            *static_cast<float*>(dest) = narrow_to_float(lua_tonumber_value(value));
            return true;
        }
        case GuiLuaFieldType::Bool: {
            // 00BD647A, lua_toboolean: false only for nil and false.
            if (dest == nullptr) {
                return false;
            }
            const bool truth = value.kind() != GuiValue::Kind::Nil &&
                               (value.kind() != GuiValue::Kind::Boolean || value.boolean());
            *static_cast<bool*>(dest) = truth;
            return true;
        }
        case GuiLuaFieldType::Handle: {
            // 00BD649E. The integral test comes first, so a number that happens
            // to be integral never reaches the table resolver.
            if (dest == nullptr || resolver == nullptr) {
                return false;
            }
            if (value.kind() == GuiValue::Kind::Number) {
                const double number = value.number();
                if (gui_lua_is_integer_number_00b66a60(number)) {
                    *static_cast<std::int32_t*>(dest) =
                        resolver->resolve_by_number(narrow_to_int(number));
                    return true;
                }
                return false;
            }
            if (value.is_table() && value.table() != nullptr) {
                *static_cast<std::int32_t*>(dest) = resolver->resolve_by_table(value.table());
                return true;
            }
            return false;
        }
        case GuiLuaFieldType::Vec3:
            return store_float_run(value, dest, 3);
        case GuiLuaFieldType::Vec2:
            return store_float_run(value, dest, 2);
        case GuiLuaFieldType::Vec4:
            return store_float_run(value, dest, 4);
        case GuiLuaFieldType::Matrix4: {
            // 00BD664A: rows 1..4, then columns 1..4 within each row, written
            // consecutively. A missing row indexes nil, which the native build
            // cannot survive; here it contributes a row of zeros.
            if (dest == nullptr || !value.is_table() || value.table() == nullptr) {
                return false;
            }
            float* out = static_cast<float*>(dest);
            const GuiTable& rows = *value.table();
            for (std::size_t row = 0; row < 4; ++row) {
                const GuiValue* entry =
                    row < rows.array.size() ? &rows.array[row] : nullptr;
                for (std::size_t column = 0; column < 4; ++column) {
                    float element = 0.0f;
                    if (entry != nullptr && entry->is_table() && entry->table() != nullptr) {
                        element = table_element(*entry->table(), column + 1);
                    }
                    out[row * 4 + column] = element;
                }
            }
            return true;
        }
        case GuiLuaFieldType::ParsedFloat: {
            // 00BD67F2: lua_tolstring then the CRT parse at 00BF8417. The
            // native code hands that parser whatever the accessor returned,
            // including null; here a value with no string form writes nothing.
            if (dest == nullptr) {
                return false;
            }
            std::string text;
            if (!lua_tostring_view(value, text)) {
                return false;
            }
            *static_cast<float*>(dest) = std::strtof(text.c_str(), nullptr);
            return true;
        }
        case GuiLuaFieldType::Unhandled9:
        default:
            // 00BD66D2 falls through the 0Ah test and leaves the field alone.
            return false;
    }
}

// ---------------------------------------------------------------------------
// 00BD61C0: one default into the same field
// ---------------------------------------------------------------------------

bool gui_lua_store_default_00bd61c0(const GuiLuaVariant& field,
                                    const GuiLuaVariant& fallback) noexcept {
    void* const dest = field.value.pointer;
    if (dest == nullptr) {
        return false;
    }
    switch (static_cast<GuiLuaFieldType>(field.tag)) {
        case GuiLuaFieldType::String: {
            const char* text = fallback.value.text;
            *static_cast<std::string*>(dest) = text != nullptr ? std::string(text) : std::string();
            return true;
        }
        case GuiLuaFieldType::Int:
        case GuiLuaFieldType::Handle:
            // 00BD6253: a Handle default is a raw id, taken with no resolver.
            *static_cast<std::int32_t*>(dest) = fallback.value.integer;
            return true;
        case GuiLuaFieldType::Float:
        case GuiLuaFieldType::ParsedFloat:
            // 00BD62F0: the default of a parsed field is already a float.
            *static_cast<float*>(dest) = fallback.value.number;
            return true;
        case GuiLuaFieldType::Bool:
            *static_cast<bool*>(dest) = fallback.value.integer != 0;
            return true;
        case GuiLuaFieldType::Vec2:
        case GuiLuaFieldType::Vec3:
        case GuiLuaFieldType::Vec4:
        case GuiLuaFieldType::Matrix4: {
            const float* source = static_cast<const float*>(fallback.value.pointer);
            if (source == nullptr) {
                return false;
            }
            const std::size_t count =
                gui_lua_field_arity(static_cast<GuiLuaFieldType>(field.tag));
            std::memcpy(dest, source, count * sizeof(float));
            return true;
        }
        case GuiLuaFieldType::Unhandled9:
        default:
            return false;
    }
}

// ---------------------------------------------------------------------------
// The key array
// ---------------------------------------------------------------------------

void gui_lua_clear_key_array_00aaa710(GuiLuaKeyArray& array) noexcept {
    for (std::size_t i = 0; i < kGuiLuaKeyArrayCapacity; ++i) {
        array.keys[i].tag = -1;
        array.keys[i].value.pointer = reinterpret_cast<void*>(~static_cast<std::uintptr_t>(0));
    }
    array.count = 0;
}

// ---------------------------------------------------------------------------
// The type tests
// ---------------------------------------------------------------------------

bool gui_lua_is_nil_00b65fb0(GuiLuaHost& host, const GuiLuaRef& object) {
    return host.type_of(object) == GuiLuaType::Nil;
}

bool gui_lua_is_string_00b660a0(GuiLuaHost& host, const GuiLuaRef& object) {
    return host.type_of(object) == GuiLuaType::String;
}

bool gui_lua_is_number_00b66050(GuiLuaHost& host, const GuiLuaRef& object) {
    return host.type_of(object) == GuiLuaType::Number;
}

bool gui_lua_is_integer_number_00b66a60(double number) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    float narrowed;
    std::int32_t integer;
    unsigned char result;
    __asm {
        fld qword ptr [number]
        fstp dword ptr [narrowed]
        fld dword ptr [narrowed]
        cvttss2si eax, dword ptr [narrowed]
        mov dword ptr [integer], eax
        fild dword ptr [integer]
        fxch st(1)
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 44h
        setnp byte ptr [result]
    }
    return result != 0;
#else
    // Numerical projection only; native FP flags/traps require the Win32 path.
    const float narrowed = static_cast<float>(number);
    if (!(narrowed >= -2147483648.0f && narrowed < 2147483648.0f)) {
        return false;
    }
    const auto integer = static_cast<std::int32_t>(narrowed);
    return static_cast<double>(narrowed) == static_cast<double>(integer);
#endif
}

bool gui_lua_is_integer_00b66a60(GuiLuaHost& host, const GuiLuaRef& object) {
    return host.type_of(object) == GuiLuaType::Number &&
        gui_lua_is_integer_number_00b66a60(host.to_number(object));
}

namespace {

// lua_tonumber against a live object: a number, or a string the lexer can
// convert. The host's to_number reports the library's own result.
double ref_number(GuiLuaHost& host, const GuiLuaRef& object) {
    const GuiLuaType type = host.type_of(object);
    if (type == GuiLuaType::Number || type == GuiLuaType::String) {
        return host.to_number(object);
    }
    return 0.0;
}

// t[index] as a float, released afterwards. A missing index is nil and
// lua_tonumber turns it into zero, exactly as the materialised form does.
float ref_element(GuiLuaHost& host, const GuiLuaRef& table, std::int32_t index) {
    const GuiLuaRef element = host.get_by_index(table, index);
    const float number = narrow_to_float(ref_number(host, element));
    host.release(element);
    return number;
}

bool ref_float_run(GuiLuaHost& host, const GuiLuaRef& object, void* dest,
                   std::int32_t count) {
    if (dest == nullptr) {
        return false;
    }
    float* out = static_cast<float*>(dest);
    for (std::int32_t i = 0; i < count; ++i) {
        out[i] = ref_element(host, object, i + 1);
    }
    return true;
}

}  // namespace

bool gui_lua_store_ref_00bd63b0(GuiLuaHost& host, const GuiLuaRef& object,
                                const GuiLuaVariant& field,
                                GuiLuaHandleResolver* resolver) {
    void* const dest = field.value.pointer;
    switch (static_cast<GuiLuaFieldType>(field.tag)) {
        case GuiLuaFieldType::String: {
            if (dest == nullptr) {
                return false;
            }
            const char* text = host.to_string(object);
            *static_cast<std::string*>(dest) = text != nullptr ? std::string(text) : std::string();
            return true;
        }
        case GuiLuaFieldType::Int:
            if (dest == nullptr) {
                return false;
            }
            *static_cast<std::int32_t*>(dest) = narrow_to_int(ref_number(host, object));
            return true;
        case GuiLuaFieldType::Float:
            if (dest == nullptr) {
                return false;
            }
            *static_cast<float*>(dest) = narrow_to_float(ref_number(host, object));
            return true;
        case GuiLuaFieldType::Bool:
            if (dest == nullptr) {
                return false;
            }
            *static_cast<bool*>(dest) = host.to_boolean(object);
            return true;
        case GuiLuaFieldType::Handle:
            if (dest == nullptr || resolver == nullptr) {
                return false;
            }
            if (gui_lua_is_integer_00b66a60(host, object)) {
                *static_cast<std::int32_t*>(dest) =
                    resolver->resolve_by_number(narrow_to_int(host.to_number(object)));
                return true;
            }
            if (host.type_of(object) == GuiLuaType::Table) {
                *static_cast<std::int32_t*>(dest) = resolver->resolve_by_table(nullptr);
                return true;
            }
            return false;
        case GuiLuaFieldType::Vec2:
            return ref_float_run(host, object, dest, 2);
        case GuiLuaFieldType::Vec3:
            return ref_float_run(host, object, dest, 3);
        case GuiLuaFieldType::Vec4:
            return ref_float_run(host, object, dest, 4);
        case GuiLuaFieldType::Matrix4: {
            if (dest == nullptr) {
                return false;
            }
            float* out = static_cast<float*>(dest);
            for (std::int32_t row = 0; row < 4; ++row) {
                const GuiLuaRef line = host.get_by_index(object, row + 1);
                for (std::int32_t column = 0; column < 4; ++column) {
                    out[row * 4 + column] = ref_element(host, line, column + 1);
                }
                host.release(line);
            }
            return true;
        }
        case GuiLuaFieldType::ParsedFloat: {
            if (dest == nullptr) {
                return false;
            }
            const char* text = host.to_string(object);
            if (text == nullptr) {
                return false;
            }
            *static_cast<float*>(dest) = std::strtof(text, nullptr);
            return true;
        }
        case GuiLuaFieldType::Unhandled9:
        default:
            return false;
    }
}

// ---------------------------------------------------------------------------
// The reader
// ---------------------------------------------------------------------------

GuiLuaReader::GuiLuaReader(GuiLuaHost& host, const GuiLuaRef& root) : host_(&host) {
    // 004425C0: the vector starts empty and the root is pushed into it, so the
    // seed is element zero and every Enter grows the path by one.
    stack_.push_back(root);
}

GuiLuaReader::~GuiLuaReader() {
    // 00441210 through the destructor at 00441A70: every element is released,
    // the block is freed and the three pointers are nulled.
    for (const GuiLuaRef& object : stack_) {
        host_->release(object);
    }
    stack_.clear();
}

void GuiLuaReader::enter_00bd8e20(const GuiLuaVariant& key) {
    if (stack_.empty()) {
        return;
    }
    const GuiLuaRef& table = stack_.back();
    switch (static_cast<GuiLuaKeyKind>(key.tag)) {
        case GuiLuaKeyKind::Name:
            stack_.push_back(host_->get_by_name(table, key.value.text));
            return;
        case GuiLuaKeyKind::Index:
            stack_.push_back(host_->get_by_index(table, key.value.integer));
            return;
        case GuiLuaKeyKind::FloatIndex:
            stack_.push_back(host_->get_by_index(table, narrow_to_int(key.value.number)));
            return;
        default:
            // 00BD5790 leaves the object default constructed and 00BD8E20
            // pushes it anyway, so the depth still grows.
            stack_.push_back(GuiLuaRef{});
            return;
    }
}

void GuiLuaReader::leave_00bd7a20() {
    if (stack_.empty()) {
        return;
    }
    host_->release(stack_.back());
    stack_.pop_back();
}

bool GuiLuaReader::has_key_00bd5eb0(const GuiLuaVariant& key) {
    if (stack_.empty()) {
        return false;
    }
    enter_00bd8e20(key);
    const bool present = !gui_lua_is_nil_00b65fb0(*host_, stack_.back());
    leave_00bd7a20();
    return present;
}

bool GuiLuaReader::read_00bd6830(const GuiLuaVariant& key, const GuiLuaVariant& field,
                                 GuiLuaHandleResolver* resolver) {
    if (stack_.empty()) {
        return false;
    }
    enter_00bd8e20(key);
    const bool stored =
        gui_lua_store_ref_00bd63b0(*host_, stack_.back(), field, resolver);
    leave_00bd7a20();
    return stored;
}

bool GuiLuaReader::read_or_default_00bd68d0(const GuiLuaVariant& key,
                                            const GuiLuaVariant& field,
                                            const GuiLuaVariant& fallback,
                                            GuiLuaHandleResolver* resolver) {
    if (stack_.empty()) {
        return false;
    }
    enter_00bd8e20(key);
    bool stored = false;
    if (gui_lua_is_nil_00b65fb0(*host_, stack_.back())) {
        stored = gui_lua_store_default_00bd61c0(field, fallback);
    } else {
        stored = gui_lua_store_ref_00bd63b0(*host_, stack_.back(), field, resolver);
    }
    leave_00bd7a20();
    return stored;
}

std::size_t GuiLuaReader::enumerate_keys_00bd5f50(GuiLuaKeyArray& array) {
    array.count = 0;
    if (stack_.empty()) {
        return 0;
    }
    const GuiLuaRef& table = stack_.back();
    GuiLuaRef key;
    GuiLuaRef value;
    bool restart = true;
    std::size_t written = 0;
    while (host_->next(table, key, value, restart)) {
        restart = false;
        // The order is the native one: string, then integral number, then any
        // other number. A key of any other type advances the walk but is not
        // recorded and does not move the count.
        if (written < kGuiLuaKeyArrayCapacity) {
            if (gui_lua_is_string_00b660a0(*host_, key)) {
                array.keys[written] = gui_lua_key_by_name(host_->to_string(key));
                ++written;
            } else if (gui_lua_is_integer_00b66a60(*host_, key)) {
                array.keys[written] =
                    gui_lua_key_by_index(narrow_to_int(host_->to_number(key)));
                ++written;
            } else if (gui_lua_is_number_00b66050(*host_, key)) {
                array.keys[written] =
                    gui_lua_key_by_float(narrow_to_float(host_->to_number(key)));
                ++written;
            }
        }
        host_->release(key);
        host_->release(value);
    }
    array.count = static_cast<std::int32_t>(written);
    return written;
}

// ---------------------------------------------------------------------------
// Running the page scripts
// ---------------------------------------------------------------------------

GuiLuaChunkOutcome run_gui_lua_chunk_00b66ca0(GuiLuaScriptHost& host,
                                              const std::string& path,
                                              bool obfuscated) {
    GuiLuaChunkOutcome outcome;
    outcome.path = path;

    std::vector<char> buffer;
    if (!host.read_file(path, buffer) || buffer.empty()) {
        // 00B66CC3, 00B66CD4 and 00B66CE5: a missing file, a stream that did
        // not open and a zero-length file all return without touching Lua.
        return outcome;
    }
    outcome.file_read = true;

    if (obfuscated) {
        // 00B66D40..00B66D61: test marker before replacing the prefix byte.
        // Only bytes AFTER the first 01h rotate; the marker becomes a space.
        bool prefix = true;
        for (char& byte : buffer) {
            const auto value = static_cast<unsigned char>(byte);
            if (prefix) {
                if (value == 1) prefix = false;
                byte = ' ';
            } else {
                byte = static_cast<char>((value << 4) | (value >> 4));
            }
        }
    }

    // 00B66DA2 luaL_loadbuffer with the path as the chunk name, then 00B66DC4
    // lua_call regardless of what it returned.
    outcome.compiled = host.load_buffer(buffer.data(), buffer.size(), path.c_str());
    host.call_unprotected(0, -1);
    outcome.called = true;
    outcome.fatal = !outcome.compiled;
    return outcome;
}

std::vector<GuiLuaChunkOutcome> run_gui_lua_file_00b69d40(GuiLuaScriptHost& host,
                                                          const std::string& path,
                                                          bool obfuscated) {
    std::vector<GuiLuaChunkOutcome> outcomes;
    outcomes.push_back(run_gui_lua_chunk_00b66ca0(host, path, obfuscated));
    for (const std::string& override_path : host.override_paths(path)) {
        outcomes.push_back(run_gui_lua_chunk_00b66ca0(host, override_path, obfuscated));
    }
    return outcomes;
}

GuiScreenScriptRun run_gui_screen_scripts_00ac6600(GuiLuaScriptHost& host,
                                                   const std::string& page_name) {
    GuiScreenScriptRun run;
    const std::string common = gui_page_script_path_00ac5600(kGuiCommonScriptName);
    const std::string page = gui_page_script_path_00ac5600(page_name);
    for (const std::string& path : {common, page}) {
        for (const GuiLuaChunkOutcome& outcome : run_gui_lua_file_00b69d40(host, path)) {
            run.fatal = run.fatal || outcome.fatal;
            run.chunks.push_back(outcome);
        }
    }
    return run;
}

bool load_gui_screen_table_00ac6600(GuiLuaScriptHost& scripts, GuiLuaHost& lua,
                                    const std::string& page_name,
                                    void (*visit)(GuiLuaReader&, void*),
                                    void* context) {
    const GuiScreenScriptRun run = run_gui_screen_scripts_00ac6600(scripts, page_name);
    if (run.fatal) {
        // The native build does not reach here: the raise inside the
        // unprotected call takes the process down first.
        return false;
    }

    // 00AC67DA globals, 00AC67DF the reader over them, 00AC6804 the descend
    // into "GuiScreen", 00AC6812 the widget walk, 00AC681F the ascend.
    GuiLuaReader reader(lua, lua.globals());
    reader.enter_00bd8e20(gui_lua_key_by_name(kGuiScreenTableName.data()));
    if (visit != nullptr) {
        visit(reader, context);
    }
    reader.leave_00bd7a20();
    return true;
}

}  // namespace bsp
