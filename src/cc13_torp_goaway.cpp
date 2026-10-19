#include "bsp/cc13_torp_goaway.hpp"

#include "bsp/lua_state_owner.hpp"
#include "bsp/mission_lua_host.hpp"
#include "bsp/native_string.hpp"

#include <cstring>
#include <exception>
#include <new>

namespace bsp::cc13_torp_goaway {
namespace {

// Host allocators for the loader's two storage contracts. The image takes
// both from its own pools; nothing reads the block addresses.
class HeapStrings final : public NativeStringStorage {
public:
    char* allocate(std::uint32_t size) override { return new char[size]; }
    void release(char* block, std::uint32_t) noexcept override { delete[] block; }
};
class HeapDescriptors final : public RobotDescriptorStorage {
public:
    void* allocate(std::uint32_t bytes) override { return ::operator new(bytes); }
    void release(void* block) noexcept override { ::operator delete(block); }
};

// Process-wide, as [00F8A30C] is: 004DC6A0 runs 00901610 once per process.
struct LoadedRows {
    bool attempted{false};
    bool loaded{false};
    std::array<PilotBotParameters, 6> rows{};
};
LoadedRows& loaded_rows() noexcept {
    static LoadedRows rows;
    return rows;
}

}  // namespace

bool MissionScriptFiles::read_file_00b66ca0(const std::string& path, std::uint32_t,
    std::vector<char>& bytes) {
    // 00885110's reader opens in its own mode (kScriptReadMode); the mode the
    // runtime passes is the same 2 at every 00B66CA0 call the loader reaches.
    bytes.clear();
    if (!host_.open_script(path)) {
        ++misses_;
        return false;
    }
    const int size = host_.script_size();
    if (size <= 0) {
        host_.close_script();
        ++misses_;
        return false;
    }
    bytes.resize(static_cast<std::size_t>(size));
    host_.read_script(bytes.data(), size);
    host_.close_script();
    ++reads_;
    return true;
}

std::vector<std::string> MissionScriptFiles::override_paths_00bdef90(const std::string& path) {
    return host_.script_variant_names(path);
}

PilotBotLoadResult load_pilot_bot_config_00901610(LuaScriptFiles& files) {
    PilotBotLoadResult result;
    LoadedRows& cache = loaded_rows();
    if (cache.attempted) {
        result.already_loaded = true;
        result.loaded = cache.loaded;
        return result;
    }
    cache.attempted = true;

    // The owner's bootstrap chunk. 00B6A020 runs the cached fundamentals.lua
    // (00884770) before anything else on every owner it opens.
    std::vector<char> fundamentals;
    if (!files.read_file_00b66ca0("Scripts\\fundamentals.lua", 2, fundamentals)) {
        result.error = "Scripts\\fundamentals.lua did not open";
        return result;
    }

    HeapStrings strings;
    HeapDescriptors descriptors;
    // live0109EEA4, the CRT's SSE2 conversion flag: set on every SSE2 host
    // (src/decal_table_probe.cpp takes the same value).
    const bool crt_sse2_conversion = true;
    RobotConfigContext context{strings, descriptors, crt_sse2_conversion};
    RobotConfigRegistry registry;
    RobotDescriptor* pilot = nullptr;
    RobotDescriptor* tail_gunner = nullptr;
    RobotDescriptor* aa_flak = nullptr;
    RobotDescriptor* aa_gunner = nullptr;
    RobotDescriptor* artillery_subdirector = nullptr;
    RobotDescriptor* artillery_gunner = nullptr;
    RobotDescriptor* torpedo = nullptr;
    RobotDescriptor* depth_charge = nullptr;
    RobotDescriptor* navigator = nullptr;
    RobotConfigAliases aliases{pilot, tail_gunner, aa_flak, aa_gunner, artillery_subdirector,
        artillery_gunner, torpedo, depth_charge, navigator};

    LuaScriptRuntime runtime(files);
    LuaStateOwnerEnvironment environment;
    environment.fundamentals.assign(fundamentals.begin(), fundamentals.end());
    environment.do_file = &LuaScriptRuntime::do_file;
    environment.do_file_context = &runtime;
    // UNCERTAIN, stated: the process globals 0108FF20 (X360COMP byte, REGION
    // string) are not reachable from the mission host. PC builds keep the
    // byte 0, and neither luaMW_init.lua, Robots.lua nor fundamentals.lua in
    // this installation reads X360COMP or REGION.
    environment.x360comp = false;

    try {
        (void)load_robot_config_00901610(registry, aliases, std::move(environment), runtime,
            context);
        if (pilot == nullptr) {
            result.error = "Robots[\"PilotBot\"] was not registered";
        } else {
            const auto* config = reinterpret_cast<const PilotBotConfig*>(pilot);
            cache.rows = config->levels;
            cache.loaded = true;
            result.loaded = true;
        }
    } catch (const std::exception& failure) {
        result.error = failure.what();
    }
    // Host adaptation: the image never frees the registry; only the PilotBot
    // rows are consumed here, so they are copied out and the rest released.
    clear_robot_registry_00900bb0(registry, descriptors);
    if (const auto* counted = dynamic_cast<MissionScriptFiles*>(&files)) {
        result.script_reads = counted->reads();
        result.script_misses = counted->misses();
    }
    return result;
}

const std::array<PilotBotParameters, 6>* loaded_pilot_bot_rows() noexcept {
    const LoadedRows& cache = loaded_rows();
    return cache.loaded ? &cache.rows : nullptr;
}

const PilotBotParameters* pilot_bot_row(int index) noexcept {
    const auto* rows = loaded_pilot_bot_rows();
    if (rows == nullptr || index < 0 || index > 5) return nullptr;
    return &(*rows)[static_cast<std::size_t>(index)];
}

}  // namespace bsp::cc13_torp_goaway
