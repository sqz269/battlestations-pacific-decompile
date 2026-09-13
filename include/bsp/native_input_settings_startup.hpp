#pragma once
#include "bsp/native_lua_bootstrap.hpp"
#include "bsp/native_lua_file_loading.hpp"
#include <cstddef>

namespace bsp {
// Actual allocation footprint, with no implicit initialization or destruction.
// 006AB6B0 produces five tree heads, three checked vectors and the persistent
// actual4C8h Lua owner at78. Opaque cells, byte50 and allocation padding retain
// their original preimages until the remaining table loader writes them.
struct alignas(4) NativeInputSettingsStorage { std::byte bytes[0x540]; };
static_assert(sizeof(NativeInputSettingsStorage) == 0x540);

// Exact normal prefix006AB6B0..006AB7E2. Stops BEFORE the006A7BE0 call and
// must not stand in for a complete constructor or publish this object. On a
// supported allocation exception, unwind the already-built empty members,
// clear the supplied realE198E8 publication and stamp the native base profile.
// On return the caller owns the five heads and persistent owner; full settings
// destruction and table production remain separate, required work.
void* construct_native_input_settings_prefix_006ab6b0(
    void* fresh_settings, void* volatile& publication_00e198e8);

struct NativeInputSettingsScriptServices {
    NativeStringStorage& strings;
    const NativeLuaBootstrapInputs& bootstrap;
    const NativeLuaFileServices& files;
};
enum class NativeInputSettingsScriptPrefixResult { guard_return, tables_pending };

// Stable-address continuation frame for the original stack owner atESP+1C0.
// It remains alive through the pending KeyboardSetup/ControllerInputNames
// parser. No copy, move, registry replacement or independent shadow state.
class NativeInputSettingsScriptFrame {
public:
    NativeInputSettingsScriptFrame() noexcept = default;
    ~NativeInputSettingsScriptFrame() noexcept;
    NativeInputSettingsScriptFrame(const NativeInputSettingsScriptFrame&) = delete;
    NativeInputSettingsScriptFrame& operator=(const NativeInputSettingsScriptFrame&) = delete;
    bool active() const noexcept { return active_; }
    void* temporary_storage() noexcept { return temporary_; }
    NativeLuaStateStorage& temporary_lua() noexcept;
    void construct_temporary() noexcept;
    void close();
private:
    alignas(4) std::byte temporary_[0x4c8];
    bool active_ = false;
};

// Exact006A7BE0..006A7CE1 schedule, plus the original early guard return.
// Set byte4 BEFORE opening persistent settings78 with mask1; execute
// Scripts/datatables/ControlPresets.lua flag0, destroy its path, construct/open
// the SAME retained temporary owner and execute Scripts\datatables\KeyboardSetup.lua
// flag0. Return tables_pending BEFORE any tree clearing/parsing at006A7CE2.
// Requires a fresh inactive frame. Exceptions close only a constructed temporary
// owner; persistent Lua and the started guard survive. No loaded-byte5 write.
NativeInputSettingsScriptPrefixResult load_native_input_settings_scripts_prefix_006a7be0(
    void* settings, NativeInputSettingsScriptServices&, NativeInputSettingsScriptFrame&);

// Explicit source fragment ABIs, not the original ECX/RET entries. Full parser,
// singleton lifetime/publication, original FH3/private stack aliases, native
// hardware-fault behavior and gameplay are not supplied by this module.
} // namespace bsp
