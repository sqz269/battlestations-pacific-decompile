// Focused source fixture; no game execution or original ABI claim.
#include "bsp/frontend_screens_cc13.hpp"
#include "bsp/native_lua_script_overrides.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("c807 native string search regression");
}
class Strings final : public bsp::NativeStringStorage {
public:
    int live{};
    char* allocate(std::uint32_t size) override { ++live; return new char[size]; }
    void release(char* data, std::uint32_t) noexcept override { --live; delete[] data; }
};
struct Manager {
    std::uintptr_t table{0x12345678u}; // Fixture dispatcher identity, never executed.
    std::byte fields[0x44]{};
    bsp::NativeStringVectorStorage suffixes{};
};
static_assert(offsetof(Manager, suffixes) == 0x48);
class Dispatch final : public bsp::NativeLuaVfsDispatch {
public:
    Manager* expected{};
    int queries{};
    std::uint8_t exists(std::uintptr_t table, void* manager, bsp::NativeString& name) override {
        require(manager == expected && table == expected->table);
        require(std::strcmp(name.data(), "scripts/dir.with.dot/mission_english.start.lua") == 0);
        ++queries;
        return 1;
    }
    void* open(std::uintptr_t, void*, const bsp::NativeString&, std::uint32_t) override { throw 1; }
    std::uint8_t is_open(std::uintptr_t, void*) override { throw 1; }
    std::uint64_t length(std::uintptr_t, void*) override { throw 1; }
    void read(std::uintptr_t, void*, void*, std::uint32_t, std::uint32_t*) override { throw 1; }
    void zero_reference(std::uintptr_t, void*) override { throw 1; }
};
}

int main() {
    Strings strings;
    Manager manager;
    Dispatch dispatch;
    dispatch.expected = &manager;
    bsp::NativeString path, suffix, prior;
    path.assign_0041e870(strings, "scripts/dir.with.dot/mission.start.lua");
    suffix.assign_0041e870(strings, "english");
    prior.assign_0041e870(strings, "already-retained");
    bsp::NativeStringVectorStorage output{};
    bsp::register_native_lua_script_suffix_00bdef80(&manager, suffix, strings);
    bsp::append_native_string_vector_004cdc20(output, prior, strings);

    // First dot after the final slash, not a directory dot or the final dot.
    bsp::append_native_lua_script_overrides_00bdef90(&manager, path, output, strings, dispatch);
    require(dispatch.queries == 1 && output.count_04 == 2);
    require(std::strcmp(output.data_00[0].data_04, "already-retained") == 0);
    require(std::strcmp(output.data_00[1].data_04,
        "scripts/dir.with.dot/mission_english.start.lua") == 0);
    require(std::strcmp(path.data(), "scripts/dir.with.dot/mission.start.lua") == 0);

    using bsp::cc13_fe1::native_string_find_first_of_005b69b0;
    require(native_string_find_first_of_005b69b0(&path, ".", -1) == 11);
    require(native_string_find_first_of_005b69b0(&path, ".", path.length()) == -1);
    require(native_string_find_first_of_005b69b0(&path, ".", path.length() + 1) == -1);
    require(native_string_find_first_of_005b69b0(&path, nullptr, 0) == -1);

    bsp::destroy_native_string_vector_004d0fa0(output, strings);
    bsp::destroy_native_string_vector_004d0fa0(manager.suffixes, strings);
    prior.release_to(strings);
    suffix.release_to(strings);
    path.release_to(strings);
    require(strings.live == 0);
    std::puts("c807 native Lua suffix split, retained output and search boundaries: PASS");
}
