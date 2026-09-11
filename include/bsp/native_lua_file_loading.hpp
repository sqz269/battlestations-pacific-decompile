#pragma once
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_string_vector.hpp"
namespace bsp {
// Explicit application services. Manager and returned streams have actual
// storage with CALLABLE original-ABI tables. This is not a numeric-vtable
// resolver, VFS owner or stream implementation. No default service is supplied.
struct NativeLuaFileServices {
    void* const volatile& manager_0109ceec;
    void (*append_overrides_00bdef90)(void* manager,const NativeString&,NativeStringVectorStorage&);
    int (*do_file_00b69e00)(lua_State*);
};
// ECX owner; stack NativeString path / low-byte obfuscation flag; RET8.
// Calls current manager+04 and stream+18/+30/+24 with original stack ABI.
// Early returns retain a returned stream. Read count/load status ignored;
// decrement references+04, dispatch current+00 at zero, then unprotected Lua
// call and free. Live length reads and signed32/unsigned64 loop comparison.
void load_native_lua_chunk_00b66ca0(NativeLuaStateStorage&,const NativeString&,
    std::uint32_t obfuscated,const NativeLuaFileServices&);
// Full B69D40 disk extent. Initial chunk then current manager's override
// append; captured vector begin/end order. Full vector cleanup on C++ unwind.
void run_native_lua_file_00b69d40(NativeLuaStateStorage&,const NativeString&,
    std::uint32_t obfuscated,NativeStringStorage&,const NativeLuaFileServices&);
// Original callback receives ECX Lua state and returns EAX0/RET. Host bridge
// provides services explicitly and installs its own ZERO-upvalue callback.
// Borrowed actual owner, call frame/index0, unchecked string conversion/path,
// flag0 and the full override path. Not an original SEH/binary replacement.
int do_native_lua_file_00b69e00(lua_State*,NativeStringStorage&,const NativeLuaFileServices&);
} // namespace bsp
