#pragma once
#include <cstdint>

namespace bsp {
struct NativeGameProfileLifetimeContext;
struct NativeGameLuaGlobalsLifetimeCalls {
    virtual ~NativeGameLuaGlobalsLifetimeCalls()=default;
    virtual void lua_globals_invalid_parameter_00bf6713();
    virtual void lua_globals_free_00bf6989(void*);
    virtual void lua_globals_erase_strings_004954f0(void* actual_header,void* output,
        void* first,void* last,NativeGameProfileLifetimeContext&);
};
struct NativeGameLuaGlobalsLifetimeContext {
    // Stable borrowed actual10h headers {opaque,begin,end,capacity_end}.
    // The first owns8h pooled strings; the second owns0Ch records whose
    // first DWORD is a CRT malloc/free pointer and remaining words are opaque.
    void* actual_strings_0108ff30;
    void* actual_records_0108ff40;
    NativeGameLuaGlobalsLifetimeCalls& calls;
};
struct NativeGameLuaGlobalsLifetimeOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    NativeGameLuaGlobalsLifetimeContext* context{};
    NativeGameProfileLifetimeContext* profile{};
    std::uint32_t native_site{};
    std::uint32_t record_index{};
    std::uint32_t iterator_output[2]; // private native output, written by4954F0
    NativeGameLuaGlobalsLifetimeOperation()=default;
    ~NativeGameLuaGlobalsLifetimeOperation();
    NativeGameLuaGlobalsLifetimeOperation(const NativeGameLuaGlobalsLifetimeOperation&)=delete;
    NativeGameLuaGlobalsLifetimeOperation& operator=(const NativeGameLuaGlobalsLifetimeOperation&)=delete;
    // Caller resolves partial ownership first. No free, rollback or retry.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Complete77B B6C360: native cdecl(first,last,destination),plain RET,EAX end.
// B6CF90 also pushes three unused private scratch DWORDs. Forward sequential
// DWORD load/store ordering is observable for overlap; this is not memmove.
// Valid reachable first/last in12-byte steps and accessible memory required.
void* copy_native_game_lua_records_00b6c360(const void* first,const void* last,void* destination) noexcept;
// Complete258B normal B6CF90: no native input,plain RET. A nonempty first
// vector gates BOTH cleanups. Preserve diagnostic/reload and captured-end
// ordering,erase first vector through actual pooled strings,free current
// record payloads with current bounds,then copy captured suffix and publish
// record end. Keep both backing arrays/capacity and stale element bytes.
// Source failure retains progress and rejects replay;not original exception ABI.
void clear_native_game_lua_globals_00b6cf90(NativeGameLuaGlobalsLifetimeContext&,
    NativeGameProfileLifetimeContext&,NativeGameLuaGlobalsLifetimeOperation&);
} // namespace bsp
