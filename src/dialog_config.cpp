#include "bsp/dialog_config.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/lua_numeric.hpp"
#include "bsp/panel_publication.hpp"

#include <cstring>
#include <utility>

namespace bsp {
namespace {
struct OwnedRef {
    GuiLua51Host& host;
    GuiLuaRef ref;
    ~OwnedRef() { host.release(ref); }
    OwnedRef(const OwnedRef&) = delete;
    OwnedRef& operator=(const OwnedRef&) = delete;
    OwnedRef(GuiLua51Host& h, GuiLuaRef r) : host(h), ref(r) {}
};
struct OwnedString {
    NativeStringStorage& storage;
    NativeString value;
    ~OwnedString() { destroy_native_string_header_0041dd20(&value, storage); }
};
// Native next releases value then key before lua_next. GuiLua51Host keeps an
// independent cursor reference, allowing precisely that caller release order.
struct Iteration {
    GuiLua51Host& host;
    GuiLuaRef table;
    GuiLuaRef key{};
    GuiLuaRef value{};
    ~Iteration() { host.release(value); host.release(key); }
    bool next(bool restart) {
        host.release(value); value = {};
        host.release(key); key = {};
        return host.next(table, key, value, restart);
    }
};
float color_lane(float number) {
    const double divisor = 255.0; //00CE4B48=406FE00000000000
    float result;
    __asm {
        fld number
        fdiv divisor
        fstp result
    }
    return result;
}
void read_globals(DialogConfigView owner, GuiLua51Host& host, DialogConfigContext& context) {
    GuiLuaRef table;
    {
        OwnedRef globals(host, host.globals());
        table = host.get_by_name(globals.ref, "DialogCharacters");
    }
    OwnedRef current_table(host, table);
    Iteration it{host, table};
    for (bool more = it.next(true); more; more = it.next(false)) {
        // Native seeds one reader vector, copies it via0044DA70/0044ADA0,
        // then destroys the first before reading the character key. Reuse
        // standard ownership with independent Lua refs; no native STL port.
        GuiLuaRef reader_root;
        {
            OwnedRef temporary(host, host.copy_ref_00b66fa0(it.value));
            OwnedRef first_root(host, host.copy_ref_00b66fa0(temporary.ref));
            host.release(temporary.ref);
            temporary.ref = {};
            reader_root = host.copy_ref_00b66fa0(first_root.ref);
        }
        OwnedRef reader(host, reader_root);
        OwnedString key{context.strings, {}};
        key.value.assign_0041e870(context.strings, host.to_string(it.key));
        auto& picture = lookup_panel_character_0044f220(
            owner.characters_04, key.value, context.strings);
        {
            // reader virtual+10h takes key(tag0,"picture") and field(tag1,
            // actual map value). Int conversion is unconditional, even nil.
            OwnedRef value(host, host.get_by_name(reader.ref, "picture"));
            picture = lua_object_integer_00b66290(host, value.ref,
                context.crt_sse2_conversion);
        }
        // key teardown precedes reader teardown, then next releases value/key.
    }
    {
        OwnedRef globals(host, host.globals());
        OwnedRef pause(host, host.get_by_name(globals.ref, "DialogDefaultPauseTime"));
        owner.default_pause_30 = lua_object_number_00b66270(host, pause.ref);
    }
    {
        OwnedRef globals(host, host.globals());
        OwnedRef colors(host, host.get_by_name(globals.ref, "DialogColors"));
        host.release(current_table.ref);
        current_table.ref = {};
        current_table.ref = host.copy_ref_00b66fa0(colors.ref);
        it.table = current_table.ref;
    }
    for (bool more = it.next(true); more; more = it.next(false)) {
        const std::int32_t key = lua_object_integer_00b66290(host, it.key,
            context.crt_sse2_conversion);
        auto& destination = panel_palette_value_0044ec00(owner.palette_10, &key,
            context.missing_palette_words, context.validation);
        // Capture destination once; write each lane before releasing its Lua
        // temporary or looking up the next. No clamp or added type validation.
        for (std::int32_t lane = 1; lane <= 4; ++lane) {
            OwnedRef value(host, host.get_by_index(it.value, lane));
            destination[static_cast<std::size_t>(lane - 1)] =
                color_lane(lua_object_number_00b66270(host, value.ref));
        }
    }
    owner.state_34 = 0;
}
} // namespace

void load_dialog_config_0044fa30(DialogConfigView owner, LuaStateOwnerEnvironment environment,
    LuaScriptRuntime& scripts, DialogConfigContext& context) {
    PcStorageLuaOwner lua(std::move(environment));
    lua.open_storage_archive_00b6a020(1);
    {
        OwnedString path{context.strings, {}};
        path.value.assign_0041e870(context.strings, "Scripts/datatables/DialogGlobals.lua");
        (void)scripts.run_file(lua.storage_lua_38(), path.value.data(), false);
    }
    {
        GuiLua51Host host(*lua.storage_lua_38());
        read_globals(owner, host, context);
    }
    lua.close_storage_archive_00b65e80();
}
} // namespace bsp
