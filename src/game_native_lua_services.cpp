#include "bsp/game_native_lua_services.hpp"
#include "bsp/game_native_lua_globals.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include <atomic>
#include <stdexcept>

namespace bsp::game {
struct GameNativeLuaServices::Impl {
    GameNativeLuaGlobalsProcess& process{game_native_lua_globals_process()};
    LuaRuntimeGlobals& globals{process.globals_0108ff20()};
    NativeLuaFundamentalsOwner* volatile& publication{process.fundamentals_0108ff1c()};
    NativeLuaFundamentalsContext fundamentals;
    NativeLuaServiceBindings bindings;
    NativeLuaBootstrapInputs bootstrap;
    std::atomic<bool> interrupted{};
    std::atomic<std::uint32_t> getter_calls{};

    Impl(GameSingletonHost& singletons, const GameNativeVfsRawServices& vfs)
        : fundamentals{singletons.sound_lifetime(), publication, vfs.strings,
              vfs.actual_vfs_publication_0109ceec, vfs.bindings},
          bindings(fundamentals, globals.x360comp, globals.region),
          bootstrap{bindings.bootstrap()} {
        auto& deletion = singletons.native_deletion_bindings();
        if (deletion.actual_lua_fundamentals_publication_0108ff1c &&
            deletion.actual_lua_fundamentals_publication_0108ff1c != &publication)
            throw std::logic_error("application Lua fundamentals publication is already bound elsewhere");
        deletion.actual_lua_fundamentals_publication_0108ff1c = &publication;
        bootstrap.fundamentals_context = this;
        bootstrap.get_fundamentals_00884770 = &get;
    }
    static const NativeLuaFundamentalsView* get(void* opaque) {
        auto& self = *static_cast<Impl*>(opaque);
        self.getter_calls.fetch_add(1, std::memory_order_relaxed);
        try {
            return get_native_lua_fundamentals_00884770(self.fundamentals);
        } catch (...) {
            // Retain all contexts. Native partial publications/registrations
            // are not repaired, cleared, or drained through an invented rollback.
            self.interrupted.store(true, std::memory_order_relaxed);
            throw;
        }
    }
};
GameNativeLuaServices::GameNativeLuaServices(GameSingletonHost& host,
    const GameNativeVfsRawServices& vfs) : impl_(std::make_unique<Impl>(host, vfs)) {}
GameNativeLuaServices::~GameNativeLuaServices() = default;
LuaRuntimeGlobals& GameNativeLuaServices::globals() noexcept { return impl_->globals; }
const NativeLuaBootstrapInputs& GameNativeLuaServices::bootstrap() const noexcept { return impl_->bootstrap; }
NativeLuaServiceBindings& GameNativeLuaServices::binding() noexcept { return impl_->bindings; }
bool GameNativeLuaServices::native_operation_interrupted() const noexcept {
    return impl_->interrupted.load(std::memory_order_relaxed);
}
std::uint32_t GameNativeLuaServices::fundamentals_getter_calls() const noexcept {
    return impl_->getter_calls.load(std::memory_order_relaxed);
}
bool GameNativeLuaServices::fundamentals_published() const noexcept { return impl_->publication != nullptr; }
} // namespace bsp::game
