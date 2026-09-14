#pragma once
// 006E9890 - the projectile class descriptor's finalise hook, which derives the
// engagement range at descriptor+60h.
//
// The gun's range getter 00731020 answers with descriptor+60h. That field is
// NOT the authored Lua `Range`: the base reader 006E8770 puts `Range` at +68h
// and never writes +60h (docs/WEAPON_CLASS_DESCRIPTOR.md). +60h is produced
// only here, by the virtual at slot +10h of the thirteen class-descriptor
// vtables: eleven carry 006E9890 itself, MTorpedo overrides with 00855A90 and
// MFlakBullet with 0070C0B0 (which calls the base and leaves +60h alone).
// The slot is reached from 006EB047, immediately after 006EA910 builds the
// descriptor, so every descriptor a gun can see has been through it.
//
// Everything here is pure: the caller supplies the authored fields and the
// constructor sub-type, and gets back the fields the hook writes. No host, no
// globals. Evidence per claim is in docs/BULLET_ENGAGEMENT_RANGE.md; the native
// call sites are in reports/cc7_bullet_engagement_range_kinds.json.
#include <cstdint>
#include <string>

#include "bsp/projectile_kinds.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses and the offsets this hook owns. projectile_kinds.hpp already
// declares the fields the reader writes; only +60h and +64h are new.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kWeaponClassDeriveEngagementRangeAddr = 0x006E9890;
inline constexpr std::uint32_t kTorpedoClassDeriveEngagementRangeAddr = 0x00855A90;
inline constexpr std::uint32_t kFlakClassDeriveEngagementRangeAddr = 0x0070C0B0;
inline constexpr std::uint32_t kWeaponClassFinalisePairAddr = 0x006EAFE0;

// Written only by the finalise hook; 00731020 and 00729BC0 read it.
inline constexpr std::size_t kWeaponClassOffEngagementRange = 0x60;
// 006E9890's own once-only latch, cleared by the constructor at 006E83CD.
inline constexpr std::size_t kWeaponClassOffFinaliseLatch = 0x64;
// 006E97F0 sets this at 006E9878; 006EB033 tests it to run the pair once.
inline constexpr std::size_t kWeaponClassOffLoadLatch = 0xC8;
// 00855A90 also writes the torpedo's terminal fall speed at 00855AD3; that
// offset is already published as kTorpedoClassOffTerminalFallSpeed in
// bsp/bomb_torpedo_tick.hpp and is deliberately not redeclared here.

// ---------------------------------------------------------------------------
// The constants the hook loads, each with the .rdata address it comes from.
// ---------------------------------------------------------------------------
// 00CFA424, the arm every sub-type outside {1,2,3,4,5,6,7,0Bh,10h} takes.
inline constexpr float kWeaponClassDefaultEngagementRange = 3000.0f;
// 00CFA428, sub-type 0Bh DepthCharge.
inline constexpr float kDepthChargeEngagementRange = 240.0f;
// 00CF9058, the double promotion of 9.81f. R = v0^2 / g inverted at 006E98CB.
inline constexpr double kWeaponClassGravity = 9.8100004196167;
// 00CEFF98, the double promotion of 0.6f, at 00855AA5.
inline constexpr double kTorpedoSwimSpeedFraction = 0.6000000238418579;
// 00CF0B50 and 00CE3808, the two artillery damage tiers at 006E99EF/006E9A14.
inline constexpr float kArtilleryTierLightBelow = 75.0f;
inline constexpr float kArtilleryTierMediumBelow = 150.0f;
// 00D7A218, the value the authored `Range` is COMISS'd against at 006E9946.
inline constexpr float kWeaponClassRangeEpsilon = 0.0f;
// 00D7A248, the constructor's `FlyTime` default at 006E83D3 (FLT_MAX).
inline constexpr float kWeaponClassFlyTimeDefault = 3.4028234663852886e+38f;
// 00CFA420, the literal the sub-type-1 rewrite searches `Name` for.
inline constexpr char kBulletAntiAirNameMarker[] = "AA";

// The sub-types 006E9890 itself produces at 006E9968-006E9A40. No constructor
// writes any of them, which is why docs/WEAPON_CLASS_DESCRIPTOR.md called them
// producerless; this hook is their producer.
inline constexpr int kProjectileSubTypeBulletPlain = 0x02;   // 006E99D8, Name has no "AA"
inline constexpr int kProjectileSubTypeBulletAntiAir = 0x03; // 006E99D8, Name contains "AA"
inline constexpr int kProjectileSubTypeArtilleryLight = 0x05;  // 006E9A06
inline constexpr int kProjectileSubTypeArtilleryMedium = 0x06; // 006E9A2B
inline constexpr int kProjectileSubTypeArtilleryHeavy = 0x07;  // 006E9A39

// ---------------------------------------------------------------------------
// Input: the descriptor as the reader chain left it, plus the constructor's
// sub-type. Defaults are the constructor's (006E8320) and the reader's
// (006E8770 GetFloatOrDefault), so a caller that only fills what the Lua row
// authored reproduces the native state.
// ---------------------------------------------------------------------------
struct WeaponClassFinaliseInput {
    int sub_type{kProjectileSubTypeBullet};  // +8h, the constructor's constant
    float range{0.0f};                       // +68h, "Range"
    float muzzle_speed{0.0f};                // +50h, "V0"
    float fly_time{kWeaponClassFlyTimeDefault};  // +54h, "FlyTime"
    float time_scale{1.0f};                  // +5Ch, constructor only
    // Sub-type 4 only, for the 5/6/7 rewrite: "DamageMin" and
    // "Blast.BlastDamageMin". They do not affect the engagement range.
    float damage_min{0.0f};        // +ACh
    float blast_damage_min{0.0f};  // +B4h
    // Sub-type 1 only, for the 2/3 rewrite. +14h is the character data of the
    // `Name` native string at +10h; an unset name reads as "".
    std::string name;
    // MTorpedo (00855A90) only.
    float water_travel_speed{0.0f};  // +E4h, "WaterTravelSpeed"
    float max_fall{0.0f};            // +E0h, "MaxFall"
    // MFlakBullet (0070C0B0) only.
    float flak_min_range{0.0f};  // +58h, "MinRange", the flak reader 0070C030
};

// Output: every field the hook and its two overrides write, plus the value the
// hook returns in AL.
struct WeaponClassFinaliseResult {
    float engagement_range{0.0f};  // +60h, what 00731020 answers with
    float muzzle_speed{0.0f};      // +50h, rewritten by the artillery arm
    float fly_time{0.0f};          // +54h, rewritten by the gun arm
    float time_scale{1.0f};        // +5Ch, rewritten by the artillery arm
    int sub_type{0};               // +8h after the 006E9968 rewrite
    bool latched{true};            // +64h on return; false only if it was set
    // MTorpedo: +ECh = sqrt(2 * MaxFall * 9.81), and the return is
    // base && +ECh > 0 (00855AD1).
    float terminal_fall_speed{0.0f};
    float swim_speed{0.0f};  // WaterTravelSpeed * 0.6, the factor of the range
    // MFlakBullet: +D8h = MinRange / V0 (0070C0BF).
    float flak_fuse_time{0.0f};
    bool accepted{true};  // AL
};

// 006E9890 with the per-class override selected by sub_type. The selection is
// exact because the constructor sub-type and the vtable are one-to-one before
// the hook runs: 0Ah is the only class whose slot +10h holds 00855A90 and 10h
// the only one holding 0070C0B0.
//
// already_finalised models the +64h latch: a second call writes nothing and
// returns true.
WeaponClassFinaliseResult weapon_class_derive_engagement_range(
    const WeaponClassFinaliseInput& in, bool already_finalised = false) noexcept;

// The single number a host that only needs 00731020's answer wants. Equivalent
// to weapon_class_derive_engagement_range(in).engagement_range.
float weapon_class_engagement_range(const WeaponClassFinaliseInput& in) noexcept;

// Lua `Type` -> the constructor sub-type, through 006EA910's case-insensitive
// name chain (projectile_class_for_lua_type). Returns 0 when nothing matches,
// which is the case where 006EA910 stores a null class and the gun cannot fire.
int weapon_class_sub_type_for_lua_type(const std::string& type) noexcept;

// 006E9968-006E9A40 on its own, for a caller that has a finalised descriptor
// and wants to know which sub-type it ended at. Sub-types other than 1 and 4
// are returned unchanged.
int weapon_class_refined_sub_type(const WeaponClassFinaliseInput& in) noexcept;

}  // namespace bsp
