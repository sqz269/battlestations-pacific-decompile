#pragma once
// Tail of the gameplay tuning settings object (packet cc2_settings_tail).
//
// Addresses: 00836EF0 00836F80 00B685C0 0093C120 0093C210, read-only in Ghidra.
// This header covers the three items docs/GAMEPLAY_SETTINGS.md left open:
//   * the four 58h sub-objects at +240h..+39Fh (WeaponHitAccuracy),
//   * the nine string reads of the loader 0083B5E0,
//   * the two repair multipliers at +3C8h/+3CCh.
// It complements include/bsp/gameplay_settings.hpp; the 76Ch struct, its size
// and its singleton address are declared there and are not repeated.
//
// Every descriptive name below is a hypothesis, not a recovered symbol.
#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// WeaponHitAccuracy, the four 58h sub-objects at +240h..+39Fh
// ---------------------------------------------------------------------------
// The constructor 00424A10 builds four of these in place with 00836EF0
// (00424A55..00424A6D, EBX counting 3..0 and EBP stepping 58h) and the loader
// 0083B5E0 fills each from one sub-table of ShipGlobals["WeaponHitAccuracy"]
// with 00836F80 (0083C81A, 0083C86F, 0083C8C4, 0083C919).
//
// Each Lua key holds a two-element array. Element [1] is the accuracy against a
// target of the small reference size, element [2] against the large one, so the
// 58h record is two scalars followed by two parallel arrays of ten floats. The
// shipped shipglobals.lua comment gives the model: the weapon's maximum range is
// divided into ten parts and each part carries a hit chance in 0..1 that also
// depends on the target's size.
inline constexpr std::size_t kWeaponHitAccuracyProfileSize = 0x58; // 00424A6D step
inline constexpr int kWeaponHitAccuracyBucketCount = 10;          // 00836F80, keys 10..100 percent

// Sub-object order in the settings object. The loader reads the four sub-tables
// in this order and each name comes from the pushed key string.
enum class WeaponHitAccuracyCategory : int {
    Artillery = 0,   // 0083C7C2 "Artillery" (00CE5454) -> +240h
    AntiAir = 1,     // 0083C81F "AA"        (00CFA420) -> +298h
    Torpedo = 2,     // 0083C874 "Torpedo"   (00CE544C) -> +2F0h
    DepthCharge = 3, // 0083C8C9 "DepthCharge" (00CFA700) -> +348h
};
inline constexpr int kWeaponHitAccuracyCategoryCount = 4;

// Byte offsets of the four sub-objects inside the 76Ch settings object, from the
// LEA ECX at each 00836F80 call site.
inline constexpr std::size_t kWeaponHitAccuracyOffsets[kWeaponHitAccuracyCategoryCount] = {
    0x240, // 0083C814
    0x298, // 0083C869
    0x2F0, // 0083C8BE
    0x348, // 0083C913
};

// One 58h sub-object. Field offsets are the FSTP destinations in 00836F80 and
// the defaults are the three constants 00836EF0 stores (00CE3D08 = 100.0f at
// +0h, 00CE386C = 200.0f at +4h, 00CE3800 = 0.5f in all twenty range slots).
struct WeaponHitAccuracyProfile {
    // "TargetReferenceSizes" (00D0A17C), metres. [1] at 00836FD3, [2] at 00837025.
    float small_target_size{100.0F}; // +00h
    float large_target_size{200.0F}; // +04h
    // "Accuracy_<n>0percent_Range"[1], the hit chance against a small target at
    // (max range * n/10). Stores 00837077..00837629, ascending n.
    float small_target_accuracy[kWeaponHitAccuracyBucketCount]{
        0.5F, 0.5F, 0.5F, 0.5F, 0.5F, 0.5F, 0.5F, 0.5F, 0.5F, 0.5F}; // +08h..+2Ch
    // The same keys' element [2], against a large target.
    // Stores 008370C8..0083767A, ascending n.
    float large_target_accuracy[kWeaponHitAccuracyBucketCount]{
        0.5F, 0.5F, 0.5F, 0.5F, 0.5F, 0.5F, 0.5F, 0.5F, 0.5F, 0.5F}; // +30h..+54h
};
static_assert(sizeof(WeaponHitAccuracyProfile) == kWeaponHitAccuracyProfileSize,
              "the sub-object is 58h bytes: 00424A6D steps EBP by 58h");
static_assert(offsetof(WeaponHitAccuracyProfile, small_target_accuracy) == 0x08, "+08h 00837077");
static_assert(offsetof(WeaponHitAccuracyProfile, large_target_accuracy) == 0x30, "+30h 008370C8");

// The eleven Lua keys in the order 00836F80 reads them. Index 0 is
// TargetReferenceSizes; indices 1..10 are the accuracy buckets 10..100 percent.
inline constexpr int kWeaponHitAccuracyKeyCount = 1 + kWeaponHitAccuracyBucketCount;
extern const char* const kWeaponHitAccuracyKeys[kWeaponHitAccuracyKeyCount];

// 00836EF0, __fastcall(this). Twenty-two MOVSS stores, no calls.
void apply_weapon_hit_accuracy_defaults_00836ef0(WeaponHitAccuracyProfile& out) noexcept;

// Integration boundary for 00836F80. Each method is one native call site of the
// repeated eleven-key block; there are no default implementations. The native
// body wraps every read in a NativeString temporary and an SEH scope, which is
// not modelled: only the key path and the destination are.
struct WeaponHitAccuracyTableHost {
    virtual ~WeaponHitAccuracyTableHost() = default;
    // 00B67800 BSP_LuaObject_Field(table, out, key), then 00B67720
    // BSP_LuaObject_Element(field, out, index) with index 1 or 2, then
    // 00B66270 BSP_LuaObject_GetNumber. The three are one logical read here
    // because the loader never keeps the intermediate reference.
    // Returns the current value when the key or the element is absent, which is
    // what the native code leaves in place: the constructor's default survives.
    virtual float read_number(const char* key, int element, float current) = 0;
};

// 00836F80, __thiscall(this=profile, LuaObject* table). Reads the eleven keys in
// order, element [1] then element [2].
void load_weapon_hit_accuracy_profile_00836f80(WeaponHitAccuracyProfile& profile,
                                               WeaponHitAccuracyTableHost& host);

// ---------------------------------------------------------------------------
// The nine string reads of 0083B5E0
// ---------------------------------------------------------------------------
// 00B662B0 BSP_LuaObject_GetString is lua_tolstring(L, index, 0) and returns the
// borrowed char*; the loader copies it into a stack NativeString before use.
// 00B685C0 BSP_LuaObject_ConstructStringOrDefault checks the reference kind
// (type 4 = string) and assigns the fallback when it is anything else.
// A NativeString is {u32 length; char* data}, from the memcpy argument order at
// 008418CE..008418D4 and 0083EB28..0083EB2B.
enum class GameplaySettingsStringSink : int {
    EffectHandle = 0,  // the name feeds 00871BA0 and the returned handle is stored
    ClassKeyedValue,   // the name selects a class index in the 97-entry table at 00E0CD80
    FailureRecord,     // the name lands in the failure descriptor vector build
    NativeStringField, // the string itself is stored
};

struct GameplaySettingsStringKey {
    const char* key;              // the Lua key, or nullptr for a positional element
    std::uint32_t site;           // the 00B662B0 / 00B685C0 call site
    std::uint32_t key_string;     // the pushed key string address
    std::uint32_t destination;    // byte offset of the destination field
    GameplaySettingsStringSink sink;
    const char* destination_note; // what the offset belongs to
};
inline constexpr int kGameplaySettingsStringKeyCount = 9;
extern const GameplaySettingsStringKey kGameplaySettingsStringKeys[kGameplaySettingsStringKeyCount];

// Destinations that are offsets in the 76Ch settings object.
inline constexpr std::size_t kRightOfWayValuesVectorOffset = 0x1DC;   // 0083BCCF LEA EBX,[ESI+1DCh]
inline constexpr std::size_t kCollisionEffectHandleOffset = 0x3A4;    // 0083E08C
inline constexpr std::size_t kSinkEffectHandleOffset = 0x72C;         // 0083EB60
// One RightOfWayValues entry: the class index the name resolved to and the float
// from element [2]. Stride 8 is proven by the SAR ECX,3 in the push helper
// 0083A880 and by the two stores at 0083BE5C and 0083BE63.
struct RightOfWayValueEntry {
    std::uint32_t class_index{0}; // +0h, index into the 97-entry table at 00E0CD80
    float value{0.0F};            // +4h
};
static_assert(sizeof(RightOfWayValueEntry) == 8, "stride 8: SAR ECX,3 at 0083A897");
inline constexpr int kRightOfWayClassNameCount = 0x61; // 0083BE47 CMP EDI,61h

// Destinations that are offsets in a free-camera-shot record. The record is 30h
// bytes: EBP is set to end-30h at 00841411 after the emplace.
inline constexpr std::size_t kFreeCameraShotRecordSize = 0x30;   // 00841411
inline constexpr std::size_t kFreeCameraShotNameOffset = 0x00;   // 008418C8
inline constexpr std::size_t kFreeCameraShotBulletEffectOffset = 0x24;    // 008414E6
inline constexpr std::size_t kFreeCameraShotExplosionEffectOffset = 0x28; // 008415D9
inline constexpr std::size_t kFreeCameraShotSplashEffectOffset = 0x2C;    // 008416CF

// ---------------------------------------------------------------------------
// The repair multipliers at +3C8h and +3CCh
// ---------------------------------------------------------------------------
// Both consumers divide by the multiplier, so a larger value means less damage
// per second, which is what the shipped comments say. The producer of the two
// timers is the session message 9Eh arm at 0082203D: selector 0 (SetFireDamage,
// 0088E320) reaches 00939F90 / 0093A470, which write task+38h, and selector 1
// (SetWaterDamage, 0088E790) reaches 00939FA0 / 0093A4F0, which write task+34h.
// 0093C210 runs the +38h timer, so it is the fire step and reads +3CCh;
// 0093C120 runs the +34h timer, so it is the water step and reads +3C8h. The
// authored key names are therefore correct and the Ghidra function names
// BSP_RepairTask_ApplyFireDamage / BSP_RepairTask_ApplyWaterDamage are swapped.
inline constexpr std::size_t kPumpRepairMultiplierOffset = 0x3C8; // 0083E3CB stores, 0093C131 reads
inline constexpr std::size_t kFireRepairMultiplierOffset = 0x3CC; // 0083E417 stores, 0093C221 reads

// Repair priority values that enable each divisor (CMP [ESI+24h] at 0093C126
// and 0093C216). Anything else uses 1.0f from 00D7A24C.
inline constexpr int kRepairPriorityFire = 3;  // 0093C216
inline constexpr int kRepairPriorityWater = 4; // 0093C126

// The divisor either step applies: the settings multiplier when the task's
// priority matches, otherwise 1.0f, times the difficulty modifier 008E6430(3,
// unit) when the singleton at 00F88C30 is live and the byte at 00E0C978 and the
// field at +ACh are both set (0093C143..0093C18B). The modifier is a contract.
float repair_damage_divisor(int task_priority, int step_priority, float settings_multiplier,
                            float difficulty_modifier) noexcept;

}  // namespace bsp
