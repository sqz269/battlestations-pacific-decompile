// Focused regression: modified Lua realloc schedule and the actual mission
// machine's allocator/close path. Build Win32 /MD /link /MANIFEST:EMBED.
#include "bsp/storage_pool.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_mission_lua_machine_storage.hpp"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#include <cstdint>
#include <array>
#include <barrier>
#include <cstdio>
#include <cstdlib>
#include <thread>

namespace {
void require(bool value, const char* message) {
    if (!value) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}
}

int main() {
    auto& pool = bsp::game_lua_storage_pool();
    auto& disabled = bsp::game::game_native_string_process().returns_disabled_01090aa4();
    const auto prior_disabled = disabled;
    disabled = 0;
    auto* a = static_cast<unsigned char*>(bsp::lua_allocate_00a6a1d0(nullptr, nullptr, 0, 31));
    require(a != nullptr, "initial small allocation");
    for (unsigned i = 0; i != 31; ++i) a[i] = static_cast<unsigned char>(i + 1);
    auto* b = static_cast<unsigned char*>(bsp::lua_allocate_00a6a1d0(nullptr, a, 31, 31));
    require(b && b != a, "same-size request must allocate before freeing");
    for (unsigned i = 0; i != 31; ++i) require(b[i] == i + 1, "same-size copy");
    auto* c = static_cast<unsigned char*>(bsp::lua_allocate_00a6a1d0(nullptr, b, 31, 257));
    require(c != nullptr, "cross threshold to CRT");
    for (unsigned i = 0; i != 31; ++i) require(c[i] == i + 1, "growing copy");
    auto* d = static_cast<unsigned char*>(bsp::lua_allocate_00a6a1d0(nullptr, c, 257, 31));
    require(d == b, "shrinking reuses the sized pool's most recent free");
    for (unsigned i = 0; i != 31; ++i) require(d[i] == i + 1, "shrinking copy");
    const auto free_before = pool.live_free_blocks();
    disabled = 1;
    require(bsp::lua_allocate_00a6a1d0(nullptr, d, 31, 0) == nullptr, "zero size result");
    require(pool.live_free_blocks() == free_before, "shared process return gate drops small block");
    disabled = prior_disabled;

    alignas(4) std::uint32_t machine[3]{};
    bsp::construct_native_mission_lua_machine_006b8740(machine);
    auto* state = reinterpret_cast<lua_State*>(machine[1]);
    void* userdata = reinterpret_cast<void*>(1);
    require(lua_getallocf(state, &userdata) == &bsp::lua_allocate_00a6a1d0 && !userdata,
        "actual native mission machine retains recovered callback and null userdata");
    require(luaL_dostring(state,
        "local t={} for i=1,2000 do t[i]=string.rep(tostring(i),20) end "
        "collectgarbage('collect') assert(#t==2000) t=nil collectgarbage('collect')") == 0,
        "real Lua growth, shrink and collection");
    bsp::delete_native_mission_lua_machine_006b87c0(machine, 0);

    // Independent real Lua states share only pool B. Synchronize their start
    // and library-open completion, then overlap allocation/GC/close. A lock
    // around a whole test thread would conceal the original pool race.
    std::barrier phase(4);
    std::array<std::thread, 4> workers;
    for (auto& worker : workers) {
        worker = std::thread([&] {
            phase.arrive_and_wait();
            alignas(4) std::uint32_t independent_machine[3]{};
            bsp::construct_native_mission_lua_machine_006b8740(independent_machine);
            auto* independent_state = reinterpret_cast<lua_State*>(independent_machine[1]);
            phase.arrive_and_wait();
            require(luaL_dostring(independent_state,
                "for round=1,16 do local t={} for i=1,512 do "
                "t[i]=string.rep(tostring(i),20) end collectgarbage('collect') "
                "for i=1,512 do assert(t[i]==string.rep(tostring(i),20)) end "
                "t=nil collectgarbage('collect') end") == 0,
                "concurrent independent Lua states preserve allocated contents");
            bsp::delete_native_mission_lua_machine_006b87c0(independent_machine, 0);
        });
    }
    for (auto& worker : workers) worker.join();
    std::puts("c889 Lua allocator schedule + lifetime + four concurrent Lua states: PASS");
}
