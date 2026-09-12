#pragma once
#include "bsp/native_string_vector.hpp"
#include "bsp/native_lua_vfs_dispatch.hpp"
namespace bsp {
// 4BCB80: ECX actual string, stack byte-set C string / signed start; RET8.
// Searches backward in the counted bytes; the set uses strlen semantics.
std::uint32_t reverse_find_native_string_bytes_004bcb80(const NativeString&,
    const char* byte_set,std::int32_t start);
// 419CA0: ECX actual string; EAX data or external original E17654 fallback.
const char* native_string_data_or_00419ca0(const NativeString&,const char* empty_00e17654) noexcept;
// BDEF80: add48 to manager then tail-jump4CDC20. Original RET4.
void register_native_lua_script_suffix_00bdef80(void* manager,const NativeString&,NativeStringStorage&);
// Full BDEF90, ECX manager, stack path/output-vector, RET8. Manager+48/+4C
// holds actual counted suffix strings; current virtual+08 is routed through
// the chosen dispatcher (default: callable original-ABI table domain).
// Does not invent manager/provider ownership.
// Captures suffix begin/end once. Output is appended, not cleared. Actual
// path/candidate headers and native temporary lifetime ordering are retained.
// 4254B0 diagnostics are verified bare RET, omitted as no observable work.
void append_native_lua_script_overrides_00bdef90(void* manager,const NativeString& path,
    NativeStringVectorStorage& output,NativeStringStorage&,
    NativeLuaVfsDispatch& =callable_native_lua_vfs_dispatch());
} // namespace bsp
