#pragma once
#include "bsp/cc13_seg87.hpp"

namespace bsp {
struct GuiLuaHost;
class GuiLua51Host;
struct GuiLuaRef;
}

namespace bsp::game {
class GameNativeStringProcess;

// Concrete CURRENT-CRT display composition for existing actual-storageB68630.
// Borrows one genuine retained process: strings() and raw_context() refer to
// its SAME actual pool/gate/manager cells. Owns no native string/pool/Lua state.
// Keep that process and all Lua owners valid through call and result release.
// No consumer-selected numeric formatter, startup or global publication.
class GameNativeLuaDisplayStringBindings final {
public:
    explicit GameNativeLuaDisplayStringBindings(GameNativeStringProcess& process) noexcept;
    GameNativeLuaDisplayStringBindings(const GameNativeLuaDisplayStringBindings&) = delete;
    GameNativeLuaDisplayStringBindings& operator=(const GameNativeLuaDisplayStringBindings&) = delete;
    GameNativeLuaDisplayStringBindings(GameNativeLuaDisplayStringBindings&&) = delete;
    GameNativeLuaDisplayStringBindings& operator=(GameNativeLuaDisplayStringBindings&&) = delete;

    // fresh is actual aligned8B output storage. A returned string owns its raw
    // buffer; its caller performs exactly one destruction in the same domain.
    NativeString* to_display_string(const NativeLuaObjectStorage& object, void* fresh) const;
    void discard_display_string(GuiLua51Host&, const GuiLuaRef&) const;
private:
    NativeLuaDisplayStringCalls calls_;
};

// Sound's typed fixtures can use semantic string storage; production supplies
// the canonical actual pool. Admit only that same raw pool/Lua reader pair.
void discard_sound_configuration_key_display(GuiLuaHost&, const GuiLuaRef&,
    NativeStringStorage& sound_strings);
} // namespace bsp::game
