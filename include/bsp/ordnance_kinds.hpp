#pragma once

#include <cstdint>

namespace bsp {

// What a projectile class descriptor answers to `descriptor->vtable[8](kind)`,
// the predicate the `007ED7E0` family uses to ask a weapon what it carries.
// docs/ORDNANCE_KIND_IDENTITY.md proves three things this header depends on:
//
//  * `vtable[8]` is a `bool(int)` PREDICATE, not a getter (`007B91FF TEST AL,AL`),
//    and it lives on the projectile class descriptor `gun[+3F8h][+34h]` that
//    `006EA910` builds - not on an entity. An entity's own vtable slot 8 is the
//    shared stub `00923030` whose entire body is `C2 04 00`; entities answer
//    class questions at `+5Ch` instead.
//  * The `kind` argument is in the **entity class-id space** of
//    `docs/ENTITY_CLASS_IDS.md`. Proved three ways: `006E4060` passes the same
//    literal `2Ah` into both `child->vtable[5Ch]` (an entity) and
//    `descriptor->vtable[8]`; the `DummyKamikazePlane` descriptor answers `17h`
//    `MPlaneKamikaze` and `DummySubmarine` answers `08h` `MSubmarine`, ids far
//    outside any plausible ordnance-local enum; and all thirteen descriptor
//    classes line up one-to-one with the authored `Type` strings.
//  * `007B9500`'s kind really is `31h` `MParatrooper` (`007B9505 PUSH 31h`).
//    `docs/ATTACK_COMMANDS.md` labelled that helper `levelbomb`, which is the
//    part that was wrong: the arm it gates tests for paratroopers FIRST and
//    routes them to a paradrop against `MCommandBuilding`, falling through to
//    general bombs otherwise. Dropping paratroopers is not a level bombing run,
//    which is exactly why the game asks.
//
// The sets below are COPIED from that document's table, not derived. The
// descriptor chains are hand-written and diverge from the entity parent graph -
// `MBomb`'s entity parent is `02` while its descriptor answers `2Ah, 29h`, and
// `DummyTarget`/`DummySubmarine` have entity parent `2Dh` but omit it. Deriving
// the chains from the parent column gets two of thirteen wrong.

// One descriptor's answer set, as a bitmask over class ids 08h..34h.
struct OrdnanceKindSet {
    std::uint64_t mask{0};

    constexpr bool contains(int class_id) const {
        return class_id >= 0x08 && class_id <= 0x3f &&
               (mask & (std::uint64_t(1) << (class_id - 0x08))) != 0;
    }
};
// Header-only: a small closed table and four predicates, so it needs no
// translation unit of its own.
namespace ordnance_detail {

constexpr std::uint64_t bit(int class_id) {
    return std::uint64_t(1) << (class_id - 0x08);
}

struct BulletTypeRow {
    const char* lua_type;
    std::uint64_t answers;
    const char* test_body;  // the shared vtable[8] body, for provenance
};

// Copied verbatim from docs/ORDNANCE_KIND_IDENTITY.md's table. Do not derive
// these chains from ENTITY_CLASS_IDS.md's parent column: they are hand-written
// and two of them diverge from it.
constexpr BulletTypeRow kBulletTypes[] = {
    {"Bullet",             bit(0x29),                                              "006E8400"},
    {"Artillery",          bit(0x29),                                              "006E8400"},
    {"Kamikaze",           bit(0x29),                                              "006E8400"},
    {"Flak",               bit(0x29),                                              "006E8400"},
    {"Bomb",               bit(0x2a) | bit(0x29),                                  "006EA2A0"},
    {"Torpedo",            bit(0x2b) | bit(0x2a) | bit(0x29),                      "006EA550"},
    {"DepthCharge",        bit(0x2c) | bit(0x2a) | bit(0x29),                      "006EA3E0"},
    {"DummyTarget",        bit(0x2e) | bit(0x2a) | bit(0x29),                      "006EA6F0"},
    {"DummyKamikazePlane", bit(0x17) | bit(0x2f) | bit(0x2d) | bit(0x2a) | bit(0x29), "006EA830"},
    {"DummySubmarine",     bit(0x08) | bit(0x30) | bit(0x2a) | bit(0x29),          "006EA8B0"},
    {"Paratrooper",        bit(0x31) | bit(0x2a) | bit(0x29),                      "006EA760"},
    {"Rocket",             bit(0x33) | bit(0x2a) | bit(0x29),                      "006EA370"},
    {"WaterMine",          bit(0x34) | bit(0x2a) | bit(0x29),                      "006EA620"},
};

// 00425850 BSP_NativeString_EqualsCStringInsensitive.
inline bool equals_insensitive(const char* a, const char* b) {
    if (a == nullptr || b == nullptr) return false;
    for (; *a != '\0' && *b != '\0'; ++a, ++b) {
        char ca = *a;
        char cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<char>(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<char>(cb - 'A' + 'a');
        if (ca != cb) return false;
    }
    return *a == '\0' && *b == '\0';
}

// 007B9320's exclusions: a slot whose ordnance has its own command class is not
// a general bomb. 34h MWaterMine is deliberately absent - see the header.
constexpr int kGeneralBombExcluded[] = {0x2c, 0x31, 0x2b, 0x33, 0x2d};

}  // namespace ordnance_detail

inline OrdnanceKindSet ordnance_kinds_for_bullet_type(const char* lua_type) {
    OrdnanceKindSet out;
    if (lua_type == nullptr) return out;
    for (const ordnance_detail::BulletTypeRow& row : ordnance_detail::kBulletTypes) {
        if (ordnance_detail::equals_insensitive(lua_type, row.lua_type)) {
            out.mask = row.answers;
            return out;
        }
    }
    // 006EA910's fall-through: an unrecognised Type matches no test, so the
    // descriptor answers nothing. An empty set, not a default.
    return out;
}

inline bool ordnance_has_paratrooper_31h(const OrdnanceKindSet& set) {
    return set.contains(0x31);
}

inline bool ordnance_has_torpedo_2bh(const OrdnanceKindSet& set) {
    return set.contains(0x2b);
}

inline bool ordnance_has_drop_kamikaze_2fh(const OrdnanceKindSet& set) {
    return set.contains(0x2f);
}

inline bool ordnance_has_general_bomb_2ah(const OrdnanceKindSet& set) {
    if (!set.contains(0x2a)) return false;
    for (const int excluded : ordnance_detail::kGeneralBombExcluded) {
        if (set.contains(excluded)) return false;
    }
    return true;
}


}  // namespace bsp
