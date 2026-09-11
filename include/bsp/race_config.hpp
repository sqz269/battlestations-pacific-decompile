#pragma once

#include "bsp/gui_lua_reader.hpp"
#include "bsp/lua_script_runtime.hpp"
#include "bsp/lua_state_owner.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <array>

namespace bsp {
struct RaceRecordAllocationWords {
    std::uint32_t index_word_04;
    std::array<std::uint32_t, 4> color_words_10;
};
// Canonical record: native20h and Win32 field layout agree. Functions below
// use new C++ interfaces, not the original thiscall ABI or executable vtable.
struct RaceRecord {
    explicit RaceRecord(const RaceRecordAllocationWords&) noexcept;
    std::uint32_t native_vtable_00{};
    std::uint32_t index_04;
    NativeString name_08;
    std::array<float, 4> color_10;
};
// Actual pointer storage corresponding to00F87464/68/6C. The opaque allocator
// word00F87460 is not accessed. Neither pointer replacement nor shrinking
// destroys a record. Replaced allocations remain the caller's responsibility.
using RaceRecordTable = SingletonPointerSlots;
struct RaceConfigContext {
    NativeStringStorage& strings;
    const bool& crt_sse2_conversion;
    const RaceRecordAllocationWords& allocation_words;
    const SingletonLifetimeCallbacks& validation;
};

// ECX=fresh record; EAX=this; RET. Clears only name and writes00D08D20.
RaceRecord& construct_race_record_007ff9d0(RaceRecord&);
// ECX=record; LuaObject* stack; RET4. Native vtable00D08D20 slot+4.
void read_race_record_007ffc00(RaceRecord&, GuiLuaHost&, GuiLuaRef,
    NativeStringStorage&);
// ECX=record; RET. Resets vtable and releases name, preserving its header.
void destroy_race_record_007ff9f0(RaceRecord&, NativeStringStorage&);
// ECX=record; flags DWORD stack; EAX=this; RET4. Bit0 frees the allocation.
RaceRecord* scalar_delete_race_record_007ffb10(RaceRecord*, std::uint32_t,
    NativeStringStorage&);
// ECX=vector owner; count DWORD, fill pointer BY VALUE; RET8. Zero-filled
// growth in the loader, no record retain/release, suffix erasure on shrinking.
void resize_race_record_table_00800090(RaceRecordTable&, std::uint32_t,
    RaceRecord* fill, const SingletonLifetimeCallbacks&);
// Native no explicit input; RET. Uses the supplied canonical global table,
// real Lua owner(mask1), Races.lua and VFS overrides. Converted indices are
// unsigned words; replacement publishes before direct canonical record load.
void load_race_config_00800160(RaceRecordTable&, LuaStateOwnerEnvironment,
    LuaScriptRuntime&, RaceConfigContext&);
// Host convenience only: frees the pointer backing, never the pointed records.
void release_race_record_table_storage(RaceRecordTable&) noexcept;
} // namespace bsp
