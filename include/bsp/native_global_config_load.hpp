#pragma once
#include "bsp/global_config.hpp"
#include "bsp/native_global_config_fields.hpp"
#include "bsp/native_lua_bootstrap.hpp"
#include "bsp/native_lua_file_loading.hpp"
#include <memory>

namespace bsp {
// Borrow the same actual string/Lua/VFS/sound domains. The FOV divisor is a
// live cell read separately for Ship and Plane after their Lua lookups.
struct NativeGlobalConfigLoadContext {
    NativeStringStorage& strings;
    const NativeLuaBootstrapInputs& bootstrap;
    const NativeLuaFileServices& files;
    NativeGlobalConfigSoundContext& sound;
    const volatile float& fov_divisor_00f889b4;
};
class NativeGlobalConfigLoadOperation final {
public:
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    NativeGlobalConfigLoadOperation();
    ~NativeGlobalConfigLoadOperation();
    NativeGlobalConfigLoadOperation(const NativeGlobalConfigLoadOperation&)=delete;
    NativeGlobalConfigLoadOperation& operator=(const NativeGlobalConfigLoadOperation&)=delete;
    Phase phase() const noexcept;
    bool retains_native_state() const noexcept;
    std::uint32_t active_call_site() const noexcept;
    NativeLuaStateStorage* retained_lua_state() noexcept;
    // Explicit diagnostic abandonment: release owned temporary references/name
    // and close the retained Lua state. Configuration writes are not rolled back.
    // This is not the original FH3 cleanup schedule and does not permit replay.
    // Requires an independently valid Lua state; phase/site alone is not
    // a restoration token. Do not use after foreign C++ escape through Lua C
    // frames: object release/close can execute Lua, and this method establishes
    // neither restored state invariants nor finalizer-service safety.
    void discard_retained_state_for_diagnostics();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void load_native_global_config_0087d7b0(GlobalConfigOwner&,
        NativeGlobalConfigLoadContext&,NativeGlobalConfigLoadOperation&);
};
// Full normal0087D7B0 schedule over the SAME actual2E8h owner returned by
//00432650. Native ECX owner, no stack arguments, RET. Opens real Lua mask1,
//runs Scripts/datatables/Globals.lua with overrides, then fills all fields.
//Vectors append; numeric defaults require NUMBER, ordinary reads coerce as Lua.
//Color arrays use native zero-based outer indices. No script fallback or
//protected Lua boundary is added. C++ failure retains the frame and forbids
//replay; original FH3/SEH/private stack and binary ABI remain separate work.
void load_native_global_config_0087d7b0(GlobalConfigOwner&,
    NativeGlobalConfigLoadContext&,NativeGlobalConfigLoadOperation&);
} // namespace bsp
