#pragma once

#include "bsp/game_native_vfs_runtime.hpp"

namespace bsp {
struct NativeStringRawPoolContext;
namespace game {
class GameNativeVfsApplication;
class GameNativeReadOnlyData;
}

// Borrow the EXISTING application's raw cells/services and the caller's SAME
// retained read-only graph. SAME data identity is a caller precondition, not
// inferred from literal values. Preparation performs metadata/data-span checks
// only, with no native header/manager/provider construction or startup.
class NativeLuaScriptOverridesRawContext final {
public:
    NativeLuaScriptOverridesRawContext(game::GameNativeVfsApplication&,
        const game::GameNativeReadOnlyData&);
    NativeLuaScriptOverridesRawContext(const NativeLuaScriptOverridesRawContext&) = delete;
    NativeLuaScriptOverridesRawContext& operator=(const NativeLuaScriptOverridesRawContext&) = delete;
    NativeLuaScriptOverridesRawContext(NativeLuaScriptOverridesRawContext&&) = delete;
    NativeLuaScriptOverridesRawContext& operator=(NativeLuaScriptOverridesRawContext&&) = delete;

private:
    friend void append_native_lua_script_overrides_00bdef90(void*, const void*,
        void*, const NativeLuaScriptOverridesRawContext&);
    NativeStringRawPoolContext& strings_;
    game::GameNativeVfsRawServices services_;
    const game::GameNativeReadOnlyData& data_;
    const char* const slash_00ce7898_;
    const char* const dot_00ce3a70_;
    const char* const underscore_00ce7890_;
    const char* const empty_0109cef0_;
};

// Ordinary Source overload: Original ECX manager, stacked path8/output0C,
// RET8. Capture manager, suffix range and native cleanup operands at their
// actual epochs; current table/slot8 feeds the concrete exists entry service.
// Keep prior output/duplicates. No NativeString/container casts or fake lookup.
// Caller retains actual headers/backing, app/data/services/cells and serialized
// lifetime through every callback/cleanup; no private-frame alias/Native ABI,
// FH3/SEH, loading/startup, transitive fencing or runtime claim is supplied.
void append_native_lua_script_overrides_00bdef90(void* actual_manager,
    const void* actual_path8, void* actual_output0c,
    const NativeLuaScriptOverridesRawContext&);
} // namespace bsp
