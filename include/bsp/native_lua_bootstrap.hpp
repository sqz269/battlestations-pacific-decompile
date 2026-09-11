#pragma once
#include "bsp/native_lua_objects.hpp"
namespace bsp {
// Readable actual0Ch cache contract. ProducerB68340 writes D62C18 at00,
// allocation pointer04 and low length DWORD08. Cache ownership stays external.
struct NativeLuaFundamentalsView {
    std::uintptr_t vtable_00;
    const char* bytes_04;
    std::uint32_t size_08;
};
static_assert(sizeof(NativeLuaFundamentalsView)==12);
// Explicit live globals/canonical cache getter and application's DoFile.
// No replacement fundamentals or VFS no-op is supplied. get_fundamentals is
// called twice: capture size08 from the first, then bytes04 from the second.
struct NativeLuaBootstrapInputs {
    const volatile std::uint8_t& x360comp_0108ff20;
    const NativeString& region_0108ff24;
    void* fundamentals_context;
    const NativeLuaFundamentalsView* (*get_fundamentals_00884770)(void*);
    int (*do_file_00b69e00)(lua_State*);
};
// Native panic ECX lua_State, EAX0, RET. Converts the current top to a string
// and returns; it neither pops nor logs. The host callback uses Lua's C ABI.
int native_lua_panic_00b669c0(lua_State*);
// FullB6A020: ECX actual4C8h owner, stack library mask, RET4. Always open base,
// then selected libraries in native order. Sets owns/state/initial-top08 only;
// does not reset tracking slots or close an earlier state. PC/X360/region chunks
// use protected calls and ignore statuses; fundamentals uses unprotected call.
// No added exception rollback or automatic close; DoFile has ZERO upvalues.
void open_native_lua_state_00b6a020(NativeLuaStateStorage&,std::uint32_t mask,
    NativeStringStorage&,const NativeLuaBootstrapInputs&);
} // namespace bsp
