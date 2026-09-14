// Focused source fixture only. These callbacks stand in for unavailable game
// fundamentals/DoFile data; production owner APIs require caller-supplied ones.
#include "bsp/native_renderer_lua_owner.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Renderer Lua owner fixture requires MSVC Win32.
#endif

namespace {
using namespace bsp;

struct FixtureStrings final : NativeStringStorage {
    bool fail_next_allocation{};
    char* allocate(std::uint32_t size) override {
        if (fail_next_allocation) {
            fail_next_allocation = false;
            throw std::runtime_error("injected Lua string allocation failure");
        }
        auto* const p = static_cast<char*>(std::malloc(size));
        if (!p) throw std::bad_alloc();
        return p;
    }
    void release(char* block, std::uint32_t) noexcept override { std::free(block); }
};

struct FixtureFundamentals {
    NativeLuaFundamentalsView view{0x00d62c18u, "", 0u};
    int calls{};
};
const NativeLuaFundamentalsView* fundamentals(void* context) {
    auto& fixture = *static_cast<FixtureFundamentals*>(context);
    ++fixture.calls;
    return &fixture.view;
}
int do_file(lua_State*) { return 0; }

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void** slot_begin(void* manager) {
    void** result{};
    std::memcpy(&result, static_cast<std::byte*>(manager) + 4, sizeof result);
    return result;
}
void** slot_end(void* manager) {
    void** result{};
    std::memcpy(&result, static_cast<std::byte*>(manager) + 8, sizeof result);
    return result;
}
bool contains(void* manager, const void* owner) {
    for (void** slot = slot_begin(manager); slot != slot_end(manager); ++slot)
        if (*slot == owner) return true;
    return false;
}
bool all_slots_clear(void* manager) {
    for (void** slot = slot_begin(manager); slot != slot_end(manager); ++slot)
        if (*slot != nullptr) return false;
    return true;
}
std::uint32_t section_depth(void* manager) {
    void* section{};
    std::memcpy(&section, static_cast<std::byte*>(manager) + 0x10, sizeof section);
    check(section != nullptr, "actual manager has no tracked critical section");
    std::uint32_t result{};
    std::memcpy(&result, static_cast<std::byte*>(section) + 0x18, sizeof result);
    return result;
}

template<std::size_t N> void fresh_bytes(std::uint8_t (&raw)[N]) {
    static_assert(N == 0x4cc);
    std::memset(raw, 0xa5, N);
}
} // namespace

int main() {
    try {
        void* volatile manager_publication = nullptr;
        void* volatile owner_publication = nullptr;
        volatile std::uint8_t x360comp = 0;
        NativeString region;
        FixtureStrings strings;
        FixtureFundamentals data;
        const NativeLuaBootstrapInputs bootstrap{
            x360comp, region, &data, &fundamentals, &do_file};
        NativeRendererLuaOwnerContext context{
            manager_publication, owner_publication, strings, bootstrap};

        alignas(NativeRendererLuaOwnerStorage) std::uint8_t normal_raw[0x4cc];
        fresh_bytes(normal_raw);
        auto* const normal = construct_native_renderer_lua_owner_00b1bb90(
            normal_raw, context);
        void* const manager = manager_publication;
        check(manager != nullptr, "actual manager was not published");
        check(owner_publication == normal && contains(manager, normal),
              "normal owner was not published and registered");
        check(normal->vtable_00 == 0x00d5e5a8u && normal->lua_04.state_04,
              "normal owner did not open real Lua");
        check(normal_raw[0x18] == 0xa5u,
              "Lua constructor changed an untouched slot-pointer preimage");
        check(section_depth(manager) == 0, "constructor retained the section");
        destroy_native_renderer_lua_owner_00b1bbf0(*normal, context);
        check(normal->vtable_00 == 0x00ce3818u && !normal->lua_04.state_04,
              "normal destruction did not close Lua and reset base profile");
        check(owner_publication == nullptr && !contains(manager, normal),
              "normal destruction did not unregister current publication");

        alignas(NativeRendererLuaOwnerStorage) std::uint8_t changed_raw[0x4cc];
        fresh_bytes(changed_raw);
        auto* const changed = construct_native_renderer_lua_owner_00b1bb90(
            changed_raw, context);
        void* const sentinel = std::malloc(4);
        check(sentinel != nullptr, "sentinel allocation failed");
        register_native_singleton_object_00bd0c30(manager, nullptr, sentinel);
        owner_publication = sentinel;
        destroy_native_renderer_lua_owner_00b1bbf0(*changed, context);
        check(owner_publication == nullptr && !contains(manager, sentinel),
              "destructor did not reload and unregister current publication");
        check(contains(manager, changed) && changed->vtable_00 == 0x00ce3818u,
              "destructor wrongly removed original owner after publication changed");
        unregister_native_singleton_object_00bcfca0(manager, nullptr, changed);
        std::free(sentinel);

        alignas(NativeRendererLuaOwnerStorage) std::uint8_t failing_raw[0x4cc];
        fresh_bytes(failing_raw);
        strings.fail_next_allocation = true;
        bool injected_failure_seen = false;
        try {
            (void)construct_native_renderer_lua_owner_00b1bb90(
                failing_raw, context);
        } catch (const std::runtime_error& error) {
            injected_failure_seen =
                std::strcmp(error.what(), "injected Lua string allocation failure") == 0;
        }
        auto* const failing = reinterpret_cast<NativeRendererLuaOwnerStorage*>(failing_raw);
        check(injected_failure_seen, "Lua open did not reach injected failure");
        check(failing->vtable_00 == 0x00ce3818u && !failing->lua_04.state_04,
              "Lua unwind did not close state and reset base profile");
        check(owner_publication == nullptr && !contains(manager, failing),
              "Lua unwind did not unregister the base");
        check(section_depth(manager) == 0 && all_slots_clear(manager),
              "fixture left the raw singleton manager locked or registered");
        check(data.calls == 4, "normal Lua opens missed fundamentals lookup");

        const NativeSingletonDeletionBindings bindings{};
        destroy_native_singleton_manager_00bd0400(manager, bindings);
        singleton_lifetime_free(manager);
        manager_publication = nullptr;
        std::cout << "renderer Lua owner fixture passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "renderer Lua owner fixture failed: " << error.what() << '\n';
        return 1;
    }
}
