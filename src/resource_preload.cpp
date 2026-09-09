#include "bsp/resource_preload.hpp"

namespace bsp {
const std::array<ResourcePreloadRequest, 5>& startup_script_preloads_0073d410() noexcept {
    static constexpr std::array<ResourcePreloadRequest, 5> requests{{
        {"scripts/datatables/inputs.lua", 0x32, 0x0073e239},
        {"scripts/datatables/keyboardsetup.lua", 0x32, 0x0073e2b7},
        {"scripts/datatables/controllerinputnames.lua", 0x32, 0x0073e335},
        {"scripts/datatables/controlpresets.lua", 0x32, 0x0073e3b3},
        {"scripts/datatables/scoring.lua", 0x32, 0x0073e431}
    }};
    return requests;
}

bool preload_startup_scripts_0073d410_fragment(FileStore& store,
    VfsMountContext& mounts, std::size_t& completed, std::string& error) {
    completed = 0;
    error.clear();
    for (const auto& request : startup_script_preloads_0073d410()) {
        if (!cache_resource_00be7ab0_fragment(store, mounts, request.name,
            request.flags, error)) return false;
        ++completed;
    }
    return true;
}
}
