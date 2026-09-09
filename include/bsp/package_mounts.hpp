#pragma once
#include <cstdint>
#include <string>

namespace bsp {
// Name/priority phase0073cc75..0073cd0c of startup0073cb10. Ordinary names
// use1000; case-insensitive leading "patch" adds decimal strtol(suffix).
// Uses complete enumerated name, not its basename, and does not require .mpkg.
// ASCII/no embedded NUL host domain; false preserves output. Enumeration,
// duplicate/mounted-name checks and mounting are separate native phases.
bool package_mount_priority_0073cb10_fragment(const std::string& enumerated_name,
    std::int32_t& priority);
}
