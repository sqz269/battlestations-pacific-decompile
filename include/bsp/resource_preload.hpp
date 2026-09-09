#pragma once
#include "bsp/mounted_streams.hpp"
#include <array>
#include <cstddef>

namespace bsp {
struct ResourcePreloadRequest {
    const char* name;
    std::uint32_t flags;
    std::uint32_t call_address; // Original instruction; provenance, not dispatch.
};

// Five calls in0073d410, after mounts/factories exist. These literals have no
// search fallback at the call sites. This is not a full application initializer.
const std::array<ResourcePreloadRequest, 5>& startup_script_preloads_0073d410() noexcept;

// Opens exact names in native order, before duplicate checks. This guarded host
// projection stops on the first unavailable/incomplete source. Earlier inserted
// entries remain; completed counts successful requests, including duplicates.
// The native caller ignores the population routine's incidental return value.
// Evidence/ABI/limits: docs/STARTUP_SCRIPT_PRELOAD.md.
bool preload_startup_scripts_0073d410_fragment(FileStore&, VfsMountContext&,
    std::size_t& completed, std::string& error);
}
