// Focused Source fixture for chain389, not a game or native differential run.
// Check the VFS's distinct-wrapper/same-cells admission and the raw deleting
// path over actual prefix-constructed settings headers. No table/Lua loading.
#include "bsp/game_input_settings_runtime.hpp"
#include "bsp/game_native_input_settings_process.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdio>
#include <cstring>
#include <new>
#include <stdexcept>

namespace {
std::uint32_t word(const void* p, std::size_t offset) {
    std::uint32_t result;
    std::memcpy(&result, static_cast<const char*>(p) + offset, sizeof result);
    return result;
}
void put(void* p, std::size_t offset, const void* value) {
    std::memcpy(static_cast<char*>(p) + offset, &value, sizeof value);
}
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    using namespace bsp;
    using namespace bsp::game;
    try {
        auto& strings = game_native_string_process();
        auto& process = game_native_input_settings_process();
        ActualNativeStringPoolStorage vfs_wrapper(strings.pool_01090aa8(),
            strings.returns_disabled_01090aa4(), strings.manager_01090aa0());
        // Valid but unused services: this fixture never calls the table loader.
        volatile std::uint8_t platform{};
        NativeString region;
        NativeLuaBootstrapInputs bootstrap{platform, region, nullptr, nullptr, nullptr};
        void* volatile file_manager{};
        NativeLuaFileServices files{file_manager, nullptr, nullptr};
        const bool sse2 = false;
        const volatile float one = 1.0f, replacement = 0.0f;
        GameInputSettingsRuntime runtime({process.publication_00e198e8(),
            strings.manager_01090aa0(), vfs_wrapper, bootstrap, files, sse2,
            one, replacement, {}});
        auto& context = runtime.context();
        process.bind_context(&context);
        require(context.actual_raw_strings == &strings.raw_context(), "same-cell wrapper rejected");

        NativeInputSettingsStorage owner;
        construct_native_input_settings_prefix_006ab6b0(&owner, process.publication_00e198e8());
        process.publication_00e198e8() = &owner;
        // One valid, default-empty native string, with an allocated vector.
        // Its null payload requires no string-pool getter or Lua interpreter.
        auto* name = ::new (singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 8, sizeof(NativeString)})) NativeString;
        put(&owner, 0x18, name);
        put(&owner, 0x1c, name + 1);
        put(&owner, 0x20, name + 1);
        require(scalar_delete_native_input_settings_006ab800(&owner, 2, context) == &owner,
            "flags2 changed the returned receiver");
        require(!process.publication_00e198e8(), "publication not cleared");
        require(word(&owner, 0) == 0x00ce3818u, "base profile not restored");
        for (const auto offset : {8u, 0x24u, 0x54u, 0x60u, 0x6cu}) {
            require(word(&owner, offset + 4) == 0 && word(&owner, offset + 8) == 0,
                "tree header not retired");
        }
        for (const auto offset : {0x14u, 0x30u, 0x40u}) {
            require(word(&owner, offset + 4) == 0 && word(&owner, offset + 8) == 0 &&
                word(&owner, offset + 12) == 0, "vector header not retired");
        }
        process.bind_context(nullptr);

        NativeStringPoolStorage* volatile other_pool{};
        volatile std::uint32_t other_gate{};
        ActualNativeStringPoolStorage foreign(other_pool, other_gate, strings.manager_01090aa0());
        GameInputSettingsRuntime wrong({process.publication_00e198e8(),
            strings.manager_01090aa0(), foreign, bootstrap, files, sse2, one, replacement, {}});
        bool rejected = false;
        try { process.bind_context(&wrong.context()); }
        catch (const std::logic_error&) { rejected = true; }
        require(rejected, "foreign pool cells admitted");
        require(!strings.pool_01090aa8() && !strings.manager_01090aa0(),
            "fixture unexpectedly created a pool or manager");
        std::puts("chain389 input cleanup: same-cell wrapper accepted, raw flags2 cleanup passed, foreign cells rejected");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "chain389 input cleanup: %s\n", error.what());
        return 1;
    }
}
