// The LuaObject API the datatable loaders share. See docs/LUA_OBJECT_API.md.
// Addresses: 00b65f50, 00b660a0, 00b66420, 00b669a0, 00b66bd0, 00b67080,
//            00b67190, 00b67700, 00b67800, 00b67980, 00b6a390.

#include "bsp/lua_object.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/locale_lua_context.hpp"
#include <cstring>
extern "C" {
#include <lua.h>
}

namespace bsp {

void lua_object_construct_00b65f50(LuaObject& object) noexcept
{
    object.state = nullptr;
    object.kind = LuaObjectKind::Unbound;
    object.reference = -1;
    object.flag_10 = false;
    // +0Ch is deliberately not touched: 00b65f50 does not write it.
}

bool lua_object_is_unbound_00b66420(const LuaObject& object) noexcept
{
    return object.kind == LuaObjectKind::Unbound;
}

bool lua_object_is_string_00b660a0(const LuaObject& object, GuiLuaType type) noexcept
{
    if (object.kind == LuaObjectKind::Unbound) {
        return false;
    }
    if (object.kind != LuaObjectKind::Reference) {
        return false;
    }
    return type == GuiLuaType::String;
}

bool lua_state_owner_should_close_00b669a0(const LuaStateOwnerLifetime& owner) noexcept
{
    return owner.state != nullptr && owner.owns_state;
}

void lua_state_owner_close_00b669a0(LuaStateOwnerLifetime& owner, LuaStateCloseHost& host)
{
    if (lua_state_owner_should_close_00b669a0(owner)) {
        host.lua_close(owner.state);
    }
    // The four bytes at +04h are cleared on both paths; the owns byte is not.
    owner.state = nullptr;
}

// ---------------------------------------------------------------------------

LuaTableScan::LuaTableScan(GuiLuaHost& host, GuiLuaRef table)
    : host_(host), table_(table)
{
    at_end_ = !host_.next(table_, key_, value_, true);
}

LuaTableScan::~LuaTableScan()
{
    // 00740d7a releases the value first, then the key at 00740d8b.
    if (value_.valid()) {
        host_.release(value_);
    }
    if (key_.valid()) {
        host_.release(key_);
    }
}

void LuaTableScan::advance()
{
    if (at_end_) {
        return;
    }
    GuiLuaRef next_key{};
    GuiLuaRef next_value{};
    const bool more = host_.next(table_, next_key, next_value, false);
    if (value_.valid()) {
        host_.release(value_);
    }
    if (key_.valid()) {
        host_.release(key_);
    }
    key_ = more ? next_key : GuiLuaRef{};
    value_ = more ? next_value : GuiLuaRef{};
    at_end_ = !more;
}

// ---------------------------------------------------------------------------

namespace {

class FieldTemporary {
public:
    FieldTemporary(GuiLuaHost& host, const GuiLuaRef& table, const char* key)
        : host_(host), object_(host.get_by_name(table, key))
    {
    }
    ~FieldTemporary()
    {
        if (object_.valid()) {
            host_.release(object_); // 00b67700
        }
    }
    FieldTemporary(const FieldTemporary&) = delete;
    FieldTemporary& operator=(const FieldTemporary&) = delete;

    const GuiLuaRef& get() const noexcept { return object_; }

private:
    GuiLuaHost& host_;
    GuiLuaRef object_;
};

} // namespace

float lua_field_number_00b66270(GuiLuaHost& host, const GuiLuaRef& table, const char* key)
{
    const FieldTemporary field(host, table, key);
    return lua_object_number_00b66270(host, field.get());
}

std::int32_t lua_field_integer_00b66290(GuiLuaHost& host, const GuiLuaRef& table,
    const char* key, const bool& crt_sse2_conversion)
{
    const FieldTemporary field(host, table, key);
    return lua_object_integer_00b66290(host, field.get(), crt_sse2_conversion);
}

std::string lua_field_string_00b662b0(GuiLuaHost& host, const GuiLuaRef& table, const char* key)
{
    const FieldTemporary field(host, table, key);
    const char* text = host.to_string(field.get());
    return text != nullptr ? std::string(text) : std::string();
}

GuiLuaRef lua_global_by_name_00b67980(GuiLuaHost& host, const char* name)
{
    const GuiLuaRef globals = host.globals();
    const GuiLuaRef value = host.get_by_name(globals, name);
    host.release(globals);
    return value;
}

namespace {
int lua_source_type(const NativeLuaObjectStorage& object) {
    if (object.kind_04 == 0) return LUA_TNONE;
    if (object.kind_04 == 2) return lua_type(object.owner_00->state_04, object.index_08);
    return LUA_TTABLE;
}
void lua_source_tabs(void* stream, std::int32_t indent, LuaSourceWriterServices& services) {
    for (std::int32_t remaining = indent; remaining > 0; --remaining)
        services.stream_vslot_5c(stream, "\t", 0);
}
std::int32_t lua_source_next_indent(std::int32_t indent) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(indent) + 1u);
}
std::int32_t lua_source_integer_key(const NativeLuaObjectStorage& key, const bool& crt_mode) {
    // AAC3..AAD0 and BA40..BA4D explicitly FSTP float; FLD float before
    // bound BF7420. Reuse the existing source with that exact narrowed domain.
    return lua_number_integer_00b66290(lua_tonumber(key.owner_00->state_04, key.index_08), crt_mode);
}
}

// 00B6A390..00B6BD22. Native ECX actual14h object, six stack arguments,
// RET18h, AL1 (upper EAX incidental). Complete normal value/actual-Lua-object
// algorithm from the full1002-line export and register/stack conversion sites.
// EBP+14h is a SIGNED INTEGER indentation count: Ghidra's float param5/loop
// variables are wrong. Existing actual first/next/lookups/releases retain stack
// identities and shifted tracked indices; registry GuiLuaRef is never used.
// Earlier metadata called this probably LuaPlus-derived; no matching vendor
// source/version was established. Library Lua, sorting and CRT bodies are bound.
// String/pool/STL/FH3 allocation and nonlocal error/fault paths are excluded.
bool lua_object_write_lua_source_text_00b6a390(NativeLuaObjectStorage& object,
    void* stream, std::string_view name, std::uint8_t sort, std::int32_t indent,
    std::uint8_t append_crlf, std::uint8_t flatten, const bool& crt_sse2_conversion,
    LuaSourceWriterServices& services) {
    lua_source_tabs(stream, indent, services);
    if (!flatten) services.stream_vslot_5c(stream, std::string(name) + " = ", 0);
    switch (lua_source_type(object)) {
    case LUA_TBOOLEAN:
        services.stream_vslot_5c(stream,
            lua_toboolean(object.owner_00->state_04, object.index_08) ? "true" : "false", 0);
        break;
    case LUA_TNUMBER: {
        const float value = lua_number_float32_00b66270(lua_tonumber(object.owner_00->state_04, object.index_08));
        std::string text = services.call_0043bdf0(value);
        if (std::strstr(text.c_str(), ".")) {
            while (!text.empty() && text.back() == '0') text.pop_back();
            while (!text.empty() && text.back() == '.') text.pop_back();
        }
        services.stream_vslot_5c(stream, text, 0);
        break;
    }
    case LUA_TSTRING: {
        const char* const text = lua_tolstring(object.owner_00->state_04, object.index_08, nullptr);
        services.stream_vslot_5c(stream, std::string("\"") + text + "\"", 0);
        break;
    }
    case LUA_TTABLE: {
        if (!flatten) {
            services.stream_vslot_5c(stream, "\r\n", 0);
            lua_source_tabs(stream, indent, services);
            services.stream_vslot_5c(stream, "{\r\n", 0);
        }
        NativeLuaObjectStorage key, value;
        construct_native_lua_object_00b65f50(&key);
        construct_native_lua_object_00b65f50(&value);
        if (!sort) {
            native_lua_iterate_first_00b67080(object, key, value);
            while (key.kind_04 != 0) {
                std::string key_text;
                const int type = lua_source_type(key);
                if (type == LUA_TNUMBER) key_text = std::to_string(lua_source_integer_key(key, crt_sse2_conversion));
                else if (type == LUA_TSTRING)
                    key_text = std::string("\"") + lua_tolstring(key.owner_00->state_04, key.index_08, nullptr) + "\"";
                // Unsupported key types retain the native empty [] spelling.
                (void)lua_object_write_lua_source_text_00b6a390(value, stream, "[" + key_text + "]",
                    0, lua_source_next_indent(indent), 0, 0, crt_sse2_conversion, services);
                services.stream_vslot_5c(stream, ",\r\n", 0);
                native_lua_iterate_next_00b67190(object, key, value);
            }
        } else {
            std::vector<std::int32_t> numbers;
            std::vector<std::string> strings;
            native_lua_iterate_first_00b67080(object, key, value);
            while (key.kind_04 != 0) {
                const int type = lua_source_type(key);
                if (type == LUA_TNUMBER) numbers.push_back(lua_source_integer_key(key, crt_sse2_conversion));
                else if (type == LUA_TSTRING)
                    strings.emplace_back(lua_tolstring(key.owner_00->state_04, key.index_08, nullptr));
                native_lua_iterate_next_00b67190(object, key, value);
            }
            services.sort_integers_00623170(numbers);
            services.sort_strings_00b69f50(strings);
            for (const auto index : numbers) {
                const std::string child_name = (flatten ? std::string(name) : std::string())
                    + "[" + std::to_string(index) + "]";
                NativeLuaObjectStorage child;
                native_lua_argument_at_00b677e0(object, &child, index);
                (void)lua_object_write_lua_source_text_00b6a390(child, stream, child_name,
                    sort, flatten ? indent : lua_source_next_indent(indent), 0, 0, crt_sse2_conversion, services);
                destroy_native_lua_object_00b67700(child);
                if (!flatten) services.stream_vslot_5c(stream, ",\r\n", 0);
            }
            for (const auto& string_key : strings) {
                const std::string child_name = (flatten ? std::string(name) : std::string())
                    + "[\"" + string_key + "\"]";
                NativeLuaObjectStorage child;
                // Inline the established B68100 wrapper: it forwards its
                // native string's C data (empty for null) to B67800 unchanged.
                native_lua_get_by_name_00b67800(object, &child, string_key.c_str());
                (void)lua_object_write_lua_source_text_00b6a390(child, stream, child_name,
                    sort, flatten ? indent : lua_source_next_indent(indent), 0, 0, crt_sse2_conversion, services);
                destroy_native_lua_object_00b67700(child);
                if (!flatten) services.stream_vslot_5c(stream, ",\r\n", 0);
            }
        }
        destroy_native_lua_object_00b67700(value);
        destroy_native_lua_object_00b67700(key);
        if (!flatten) {
            lua_source_tabs(stream, indent, services);
            services.stream_vslot_5c(stream, "}", 0);
        }
        break;
    }
    default:
        // B68550 rechecks the type, then delegates to existing B68460.
        services.stream_vslot_5c(stream, locale_lua_type_name_00b68460(lua_source_type(object)), 0);
        break;
    }
    if (append_crlf) services.stream_vslot_5c(stream, "\r\n", 0);
    return true;
}

} // namespace bsp
