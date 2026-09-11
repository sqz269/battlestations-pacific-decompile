// The LuaObject API the datatable loaders share. See docs/LUA_OBJECT_API.md.
// Addresses: 00b65f50, 00b660a0, 00b66420, 00b669a0, 00b66bd0, 00b67080,
//            00b67190, 00b67700, 00b67800, 00b67980.

#include "bsp/lua_object.hpp"

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

} // namespace bsp
