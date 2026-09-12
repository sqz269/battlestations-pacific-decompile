// The producers behind the unit-side gunnery pass: the authored target
// preference lists, the per-category gun index on the unit, the per-category
// engagement ranges, the Lua-loaded gunnery tuning on the global config object
// and the "untouchable" flag that removes a unit from every sweep.
//
// docs/UNIT_GUNNERY_PASS.md described these five as contracts. This header is
// their producer side; docs/GUNNERY_TABLES.md carries the evidence and the
// corrections to that document.
//
// Reused rather than redeclared: unit_gunnery_pass.hpp for the consumer-side
// constants (kUnitGunneryCategoryCount, kUnitGunneryClassIdCount,
// kUnitOffCategoryRecords, kUnitOffCategoryRanges, kUnitGunneryListNode*),
// unit_weapons.hpp for kGunOffWeaponClass.
//
// Every descriptive name here is a hypothesis, not a recovered symbol, with two
// exceptions that are string literals in the image: the twelve weapon Function
// spellings (PLANEGUN .. CATAPULT) and the three Lua binding names
// (AddUntouchableUnit, RemoveUntouchableUnit, IsUnitUntouchable).
#ifndef BSP_GUNNERY_TABLES_HPP
#define BSP_GUNNERY_TABLES_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/unit_gunnery_pass.hpp"
#include "bsp/unit_weapons.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses
// ---------------------------------------------------------------------------

// 00956C20, the producer of the per-category gun index and of the per-category
// engagement ranges. __fastcall(Unit*), RET 0, body 00956C20-00956ED0.
inline constexpr std::uint32_t kUnitRebuildWeaponCategoryIndexAddress = 0x00956C20u;

// 00955EB0, the list clear it calls thirteen times. Body 00955EB0-00955EF5.
inline constexpr std::uint32_t kGunCategoryListClearAddress = 0x00955EB0u;

// 007327B0, the only producer of the weapon Function at [gun+3F4h][+80h], which
// is the gunnery category index. Body 007327B0-00732F4F.
inline constexpr std::uint32_t kGunClassReadLuaFieldsAddress = 0x007327B0u;

// 0087D7B0, the global config loader. Body 0087D7B0-0087F96F.
inline constexpr std::uint32_t kGlobalConfigLoadFromLuaGlobalsAddress = 0x0087D7B0u;

// The two gun predicates 00956C20 uses.
inline constexpr std::uint32_t kGunIsOperationalAddress = 0x00729F10u;
inline constexpr std::uint32_t kGunMaxWeaponRangeAddress = 0x00731020u;

// The three Lua bindings behind the untouchable flag at entity+1D4h.
inline constexpr std::uint32_t kLuaAddUntouchableUnitAddress = 0x008AC140u;
inline constexpr std::uint32_t kLuaRemoveUntouchableUnitAddress = 0x008AC2B0u;
inline constexpr std::uint32_t kLuaIsUnitUntouchableAddress = 0x008AC420u;

// ---------------------------------------------------------------------------
// The twelve gunnery categories
// ---------------------------------------------------------------------------
//
// The category index the gunnery pass loops over is not a derived mapping: it
// is the weapon Function of the gun's class descriptor, [gun+3F4h][+80h],
// written by 007327B0 from the `Function` key of the device class row. The
// twelve spellings below are the string literals 007327E4..00732991 compares
// against, in the order of the stores.
enum class GunneryCategory : int {
    kPlaneGun = 0x00,            // "PLANEGUN"
    kAaMachineGun = 0x01,        // "AAMACHINEGUN"
    kLightArtillery = 0x02,      // "LIGHTARTILLERY"
    kMediumArtillery = 0x03,     // "MEDIUMARTILLERY"
    kHeavyArtillery = 0x04,      // "HEAVYARTILLERY"
    kFlak = 0x05,                // "FLAK"
    kLightArtilleryFlak = 0x06,  // "LIGHTARTILLERYFLAK"
    kTorpedo = 0x07,             // "TORPEDO"
    kDepthCharge = 0x08,         // "DEPTHCHARGE"
    kDepthChargeLauncher = 0x09, // "DEPTHCHARGELAUNCHER"
    kBombPlatform = 0x0A,        // "BOMBPLATFORM"
    kCatapult = 0x0B,            // "CATAPULT"
};

// The `Function` spelling for a category index, or nullptr when out of range.
const char* gunnery_category_function_name(int category) noexcept;

// The inverse: 007327B0's chain, returning -1 when no literal matches. A device
// row whose Function matches nothing leaves the descriptor field untouched.
int gunnery_category_from_function_name(const char* function_name) noexcept;

// ---------------------------------------------------------------------------
// The authored preference lists at 00E092C8
// ---------------------------------------------------------------------------
//
// Twelve rows of 61h dwords on a 184h stride, ending at 00E0A4F8. They are file
// initialized: .data is written to disk from 00E08000 for 10000h bytes, so the
// whole block is initialized storage in the image, and the only reference to
// 00E092C8 anywhere in the image is the MOV ESI immediate at 00727BDB. There is
// no run-time producer and no Lua source; the values below are the image's.
//
// A row lists entity class ids (docs/ENTITY_CLASS_IDS.md, the dword at
// entity+0C4h) best first; a trailing 0 means "absent from this list".
inline constexpr int kGunneryPreferenceRowCapacity = 22;

struct GunneryPreferenceRow {
    int length = 0;
    std::array<std::uint8_t, kGunneryPreferenceRowCapacity> ids{};
};

// The installed table, read from battlestationspacific.exe at 00E092C8. Each
// row's remaining 61h-length entries are zero in the image.
extern const std::array<GunneryPreferenceRow, kUnitGunneryCategoryCount>
    kGunneryPreferenceLists;

// 00727BD0's rule, as a pure function over a preference table.
//
// For each category the row of the rank table is zeroed, then every non-zero id
// in the preference row is given the next one-based rank. The write index is
// `id + category*61h`, which is the source of one quirk worth keeping: category
// 0's single id 61h is one past the class-id space, so its rank lands on the
// first slot of row 1. The next iteration zeroes row 1 before filling it, so the
// stray write never survives. `out` must hold kUnitGunneryCategoryCount *
// kUnitGunneryClassIdCount ints.
void build_rank_table_00727bd0(
    const std::array<GunneryPreferenceRow, kUnitGunneryCategoryCount>& lists,
    int* out) noexcept;

// The rank a category assigns to a class id: one-based position, 0 for a class
// the category never engages. Lower is more preferred.
int gunnery_rank(const int* rank_table, int category, int class_id) noexcept;

// ---------------------------------------------------------------------------
// The per-category gun index on the unit, at unit+394h
// ---------------------------------------------------------------------------
//
// Twelve records of 0Ch bytes. 00956C20 is the producer, and it makes the
// record a doubly linked list header, not the gate/head/unused triple the
// consumer-side reading suggested.
inline constexpr std::size_t kGunneryCategoryRecordOffCount = 0x00;  // unit+394h
inline constexpr std::size_t kGunneryCategoryRecordOffHead = 0x04;   // unit+398h
inline constexpr std::size_t kGunneryCategoryRecordOffTail = 0x08;   // unit+39Ch

// The node 00956C20 allocates with operator new(0Ch).
inline constexpr std::size_t kGunneryListNodeOffPrev = 0x00;
inline constexpr std::size_t kGunneryListNodeOffNext = 0x04;
inline constexpr std::size_t kGunneryListNodeOffGun = 0x08;
inline constexpr std::size_t kGunneryListNodeSize = 0x0C;

// The thirteenth list: every gun that entered any category list, same shape.
inline constexpr std::size_t kUnitOffAllGunsRecord = 0x424;

// The two extra range fields 00956C20 writes next to unit+430h.
inline constexpr std::size_t kUnitOffArtilleryMaxRange = 0x490;  // Functions 2,3,4,6
inline constexpr std::size_t kUnitOffAnyWeaponMaxRange = 0x494;  // every Function

// The per-category accumulated blast term, twelve floats.
inline constexpr std::size_t kUnitOffCategoryBlastSum = 0x460;

// Set when any live device has Function 7 TORPEDO.
inline constexpr std::size_t kUnitOffHasTorpedoLauncher = 0x6E0;

// The unit's direct device children: head at unit+48h, next at device+44h.
// docs/UNIT_WEAPON_DEVICES.md step 3 has the same pair.
inline constexpr std::size_t kUnitOffDeviceListHead = 0x48;
inline constexpr std::size_t kDeviceOffNextSibling = 0x44;
inline constexpr std::size_t kDeviceOffDeadFlag = 0x5D;

// The class id vtable[5Ch] must answer for a device to be a gun.
inline constexpr int kDeviceKindGun = 0x20;

// The engagement range each category is seeded with before the max, the float
// at 00CE38B8.
inline constexpr float kGunneryCategoryRangeSeed = 10.0f;

// The scale 00956C20 applies to the blast pair, the double at 00D7A280.
inline constexpr double kGunneryBlastSumScale = 0.5;

// ---------------------------------------------------------------------------
// The Lua-loaded gunnery tuning on the global config object
// ---------------------------------------------------------------------------
//
// 0087D7B0 runs Scripts/datatables/Globals.lua and fetches the global table
// `Globals`. Every field below is an FSTP of BSP_LuaObject_GetNumber
// (00B66270, lua_tonumber), so a key the table does not carry installs 0.0f.
inline constexpr std::size_t kGlobalConfigOffWeaponDirectorThinkTime = 0x88;
inline constexpr std::size_t kGlobalConfigOffSafeToFireCacheTimeOut = 0x8C;
inline constexpr std::size_t kGlobalConfigOffLosTargetHeightAdd = 0x90;
inline constexpr std::size_t kGlobalConfigOffLosViewerHeightAdd = 0x94;
inline constexpr std::size_t kGlobalConfigOffLosTargetHeightMul = 0x98;
inline constexpr std::size_t kGlobalConfigOffLosViewerHeightMul = 0x9C;
inline constexpr std::size_t kGlobalConfigOffLosVisibleTimeOut = 0xA0;
inline constexpr std::size_t kGlobalConfigOffLosInvisibleTimeOut = 0xA4;

// The values the shipped Globals.lua installs.
inline constexpr float kInstalledWeaponDirectorThinkTime = 2.0f;
inline constexpr float kInstalledSafeToFireCacheTimeOut = 2.0f;
inline constexpr float kInstalledLosTargetHeightAdd = 5.0f;
inline constexpr float kInstalledLosViewerHeightAdd = 5.0f;
inline constexpr float kInstalledLosTargetHeightMul = 0.0f;
// Globals.lua has no ViewerHeightMul key: it spells ViewerHeightAdd twice, and
// the first of the pair carries the multiplier's comment. lua_tonumber on the
// missing key gives 0.0f, which is what the shadowed 0.0 would have given too.
inline constexpr float kInstalledLosViewerHeightMul = 0.0f;
inline constexpr float kInstalledLosVisibleTimeOut = 5.0f;
inline constexpr float kInstalledLosInvisibleTimeOut = 4.0f;

// The gunnery pass throttles on +88h: it accumulates dt into this+6Ch and runs
// only when the accumulator reaches the think time, then zeroes it.
struct GunneryThrottle {
    float accumulator = 0.0f;
};

// 00865014..0086506A, with 00864C1D's prime. `attach` seeds the accumulator at
// the threshold so the first tick after attachment runs at once.
void gunnery_throttle_prime_00864c1d(GunneryThrottle& throttle,
                                     float think_time) noexcept;
bool gunnery_throttle_step_00865014(GunneryThrottle& throttle,
                                    float dt,
                                    float think_time) noexcept;

// ---------------------------------------------------------------------------
// The untouchable flag at entity+1D4h
// ---------------------------------------------------------------------------
//
// 00862440 skips a candidate whose proxy carries it. The proxy is
// entity->vtable[140h](): the entity itself for a ship (0047F320 is `mov eax,
// ecx`), [plane+9D4h] for a plane (007B97E0), and [fort+738h] or the fort
// itself for a land fort (006F57A0).
inline constexpr std::size_t kEntityOffUntouchable = 0x1D4;  // one byte
inline constexpr std::size_t kEntityVtableGunneryProxySlot = 0x140;

// 00862440's rule: a candidate is suppressed when the proxy exists and the flag
// is set.
bool entity_suppresses_gunnery_00862440(bool proxy_present,
                                        bool untouchable) noexcept;

// ---------------------------------------------------------------------------
// The rebuild. One virtual per native call site 00956C20 makes.
// ---------------------------------------------------------------------------

// What the rebuild reads off one device.
struct GunneryRebuildDevice {
    void* device = nullptr;
    bool dead = false;          // [device+5Dh]
    bool is_gun = false;        // vtable[5Ch](20h)
    bool operational = false;   // 00729F10
    int weapon_function = 0;    // [device+3F4h][+80h]
    float max_range = 0.0f;     // 00731020
    // The kind-6 extra the rebuild only consults for category 6:
    // [[[gun+3F4h]+74h]+7Ch]+60h.
    float flak_alternate_range = 0.0f;
    bool has_blast = false;     // [desc+78h] > 0
    float blast_inner = 0.0f;   // [[desc+74h (+48h when kind 6 in cat 6)]+34h]+0ACh
    float blast_outer = 0.0f;   // ...+0B0h
};

struct GunneryRebuildResult {
    // unit+394h + cat*0Ch, in insertion order.
    std::array<std::array<void*, 32>, kUnitGunneryCategoryCount> category_guns{};
    std::array<int, kUnitGunneryCategoryCount> category_counts{};
    // unit+424h, every gun that entered a category list.
    std::array<void*, 256> all_guns{};
    int all_gun_count = 0;
    // unit+430h + cat*4 and unit+460h + cat*4.
    std::array<float, kUnitGunneryCategoryCount> category_ranges{};
    std::array<float, kUnitGunneryCategoryCount> category_blast_sums{};
    float artillery_max_range = 0.0f;  // unit+490h
    float any_weapon_max_range = 0.0f; // unit+494h
    bool has_torpedo_launcher = false; // unit+6E0h
};

struct UnitWeaponCategoryIndexHost {
    virtual ~UnitWeaponCategoryIndexHost() = default;

    // 00956C31: unit+6E0h = 0.
    virtual void clear_torpedo_flag() = 0;

    // 00956C38 and 00956C4A: 00955EB0 on unit+424h, then on each of the twelve
    // records at unit+394h + i*0Ch. `record` is -1 for the all-guns list.
    virtual void clear_list_00955eb0(int record) = 0;

    // 00956C57..00956D3E: the device walk, head unit+48h chained through +44h.
    virtual int device_count() = 0;
    virtual GunneryRebuildDevice device_at(int index) = 0;

    // 00956CB6-00956CF4 and 00956CF9-00956D30: the node allocations and the
    // two tail appends.
    virtual void append_to_category(int category, void* gun) = 0;
    virtual void append_to_all_guns(void* gun) = 0;

    // 00956C8F: unit+6E0h = 1 when a gun has Function 7.
    virtual void set_torpedo_flag() = 0;

    // 00956D63, 00956D6C, 00956DF3, 00956E0D, 00956E9B and the two seeds at
    // 00956D49/00956D51: the range and blast stores.
    virtual void store_category_range(int category, float range) = 0;
    virtual void store_category_blast_sum(int category, float sum) = 0;
    virtual void store_artillery_max_range(float range) = 0;
    virtual void store_any_weapon_max_range(float range) = 0;
};

// The sequence 00956C20 runs, in its order.
GunneryRebuildResult rebuild_weapon_category_index_00956c20(
    UnitWeaponCategoryIndexHost& host);

// The range rule on its own: the seed, the per-gun max and the category-6
// alternate. 00956D63..00956E0E.
float category_engagement_range_00956d63(
    const GunneryRebuildDevice* guns, int count, int category) noexcept;

// 00956DC9: the Functions that feed unit+490h.
bool weapon_function_is_artillery_00956dc9(int weapon_function) noexcept;

}  // namespace bsp

#endif  // BSP_GUNNERY_TABLES_HPP
