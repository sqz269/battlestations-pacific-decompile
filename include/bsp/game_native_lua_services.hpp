#pragma once
#include <cstdint>
#include <memory>

namespace bsp {
struct LuaRuntimeGlobals;
struct NativeLuaBootstrapInputs;
class NativeLuaServiceBindings;
}
namespace bsp::game {
class GameSingletonHost;
struct GameNativeVfsRawServices;

// Borrow the existing actual VFS/strings/manager and canonical process Lua
// cells. Retain before calling any getter, and through all Lua closes and the
// shared manager drain. Construction itself performs no native registration.
class GameNativeLuaServices final {
public:
    GameNativeLuaServices(GameSingletonHost&, const GameNativeVfsRawServices&);
    ~GameNativeLuaServices();
    GameNativeLuaServices(const GameNativeLuaServices&) = delete;
    GameNativeLuaServices& operator=(const GameNativeLuaServices&) = delete;
    LuaRuntimeGlobals& globals() noexcept;
    const NativeLuaBootstrapInputs& bootstrap() const noexcept;
    // Activate this binding around every native interpreter entry; the
    // fundamentals-only semantic environment adapter needs no DoFile scope.
    NativeLuaServiceBindings& binding() noexcept;
    bool native_operation_interrupted() const noexcept;
    std::uint32_t fundamentals_getter_calls() const noexcept;
    bool fundamentals_published() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
