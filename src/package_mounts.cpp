#include "bsp/package_mounts.hpp"
#include <cstdlib>
#include <cstring>
#include <limits>

namespace bsp {
bool package_mount_priority_0073cb10_fragment(const std::string& name,
    std::int32_t& priority) {
    static_assert(sizeof(long) == 4, "Native Win32 strtol returns signed32");
    if (name.size() > static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)())) return false;
    for (unsigned char c : name) if (c == 0 || c >= 0x80) return false;
    std::uint32_t bits = 1000;
    if (name.size() >= 5 && _strnicmp(name.c_str(), "patch", 5) == 0) {
        // Native atol00bf8417 ->00bf83f1 calls strtol00c03468 with radix10,
        // null endptr. Keep optional whitespace/sign, numeric prefix, saturation
        // on conversion overflow, and ignored suffix; then ADD EDI,3E8 wraps.
        bits += static_cast<std::uint32_t>(std::strtol(name.c_str() + 5, nullptr, 10));
    }
    std::memcpy(&priority, &bits, sizeof(bits));
    return true;
}
}
