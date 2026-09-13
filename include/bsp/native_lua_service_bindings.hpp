#pragma once
#include "bsp/native_lua_fundamentals.hpp"
#include "bsp/native_lua_file_loading.hpp"

namespace bsp {
// Explicit source routing for the context-free callbacks required by native
// Lua bootstrap and file loading. All referenced owners/services are borrowed.
class NativeLuaServiceBindings final {
public:
    NativeLuaServiceBindings(NativeLuaFundamentalsContext& fundamentals,
        const volatile std::uint8_t& x360comp_0108ff20,
        const NativeString& region_0108ff24) noexcept;
    NativeLuaServiceBindings(const NativeLuaServiceBindings&) = delete;
    NativeLuaServiceBindings& operator=(const NativeLuaServiceBindings&) = delete;
    NativeLuaServiceBindings(NativeLuaServiceBindings&&) = delete;
    NativeLuaServiceBindings& operator=(NativeLuaServiceBindings&&) = delete;

    const NativeLuaFileServices& files() const noexcept { return files_; }
    const NativeLuaBootstrapInputs& bootstrap() const noexcept { return bootstrap_; }

    // Activate around every synchronous Lua execution entry. Nested scopes,
    // including scopes for a different bundle, restore the preceding binding.
    class Activation final {
    public:
        explicit Activation(NativeLuaServiceBindings& binding) noexcept;
        ~Activation() noexcept;
        Activation(const Activation&) = delete;
        Activation& operator=(const Activation&) = delete;
        Activation(Activation&&) = delete;
        Activation& operator=(Activation&&) = delete;
    private:
        NativeLuaServiceBindings* previous_;
    };

private:
    static NativeLuaServiceBindings& active();
    static int do_file(lua_State* state);
    static void append_overrides(void* manager, const NativeString& path,
        NativeStringVectorStorage& output);

    NativeLuaFundamentalsContext& fundamentals_;
    NativeLuaFileServices files_;
    NativeLuaBootstrapInputs bootstrap_;
};
} // namespace bsp
