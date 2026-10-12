#pragma once

// Band 00950000-0097FFFF (cc13-gp95, docs/CC13_GP_0095_0097.md). New C++ interfaces, not the
// native register ABI. Names are hypotheses, not recovered symbols.

#include <array>
#include <cstdint>
#include <string_view>

namespace bsp::band_0095_cc13 {

// One Lua mission event block type that 0097E360 BSP_WarningManager_ParseEventBlock accepts.
// `subscription_vtable` is the class vtable the constructor installs; its slots are [0] scalar
// deleting destructor, [1] Load, [2] Save, [3] Matches (vtable block 00D1B670).
struct EventSubscriptionType {
    std::string_view event;              // compared case-insensitively
    std::uint32_t alloc_size;            // operator_new size pushed before the constructor
    std::uint32_t constructor;           // __thiscall(ECX = new block), returns ECX
    std::uint32_t subscription_vtable;
};

// The 23 types in the order 0097E360 tests them: "recon" first with __stricmp on the block's
// data pointer (skipped when it is null), then the others with
// BSP_NativeString_EqualsCStringInsensitive 00425850, which never matches a null string.
inline constexpr std::array<EventSubscriptionType, 23> kEventSubscriptionTypes{{
    {"recon", 0x4C, 0x0097A220, 0x00D1B67C},
    {"kill", 0x3C, 0x0097A450, 0x00D1B68C},
    {"hit", 0x80, 0x0097C2C0, 0x00D1B740},
    {"exitzone", 0x2C, 0x0097A620, 0x00D1B69C},
    {"input", 0x1C, 0x0097A790, 0x00D1B6AC},
    {"surrender", 0x1C, 0x0097A8B0, 0x00D1B6BC},
    {"failure", 0x38, 0x0097C560, 0x00D1B750},
    {"leak", 0x28, 0x0097A9D0, 0x00D1B6CC},
    {"fire", 0x28, 0x0097AAF0, 0x00D1B6DC},
    {"repair", 0x2C, 0x0097AC10, 0x00D1B6EC},
    {"zone", 0x3C, 0x0097C6D0, 0x00D1B760},
    {"command", 0x4C, 0x0097C8A0, 0x00D1B770},
    {"target", 0x2C, 0x0097AD80, 0x00D1B6FC},
    {"ammoType", 0x2C, 0x0097AEF0, 0x00D1B70C},
    {"stock", 0x3C, 0x0097B060, 0x00D1B71C},
    {"gui", 0x2C, 0x0097CAD0, 0x00D1B780},
    {"generate", 0x2C, 0x0097CC40, 0x00D1B790},
    {"player", 0x2C, 0x0097CDB0, 0x00D1B7A0},
    {"musicOver", 0x1C, 0x0097CF20, 0x00D1B7B0},
    {"chat", 0x3C, 0x0097D040, 0x00D1B7C0},
    {"entityKilled", 0x2C, 0x0097D210, 0x00D1B7D0},
    {"shipLanded", 0x1C, 0x0097B230, 0x00D1B72C},
    {"hpEvent", 0x30, 0x0097D380, 0x00D1B7E0},
}};

// 0097E360's selection. Returns nullptr where the native routine would call slot 1 (Load)
// through a null pointer (no type matched, or the block's string is null). `has_data` false
// models the null data pointer of the native string header {length, data}.
const EventSubscriptionType* find_event_subscription_type_0097e360(std::string_view event,
                                                                   bool has_data) noexcept;

}  // namespace bsp::band_0095_cc13
