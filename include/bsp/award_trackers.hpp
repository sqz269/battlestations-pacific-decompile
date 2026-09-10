#pragma once
#include <cstddef>
#include <cstdint>

// Host projection of the per-frame hint / "award" tracker that GGame::OnMove
// 004e4a40 drives at 004e525e..004e52ba. The native singleton is the 0A0h byte
// object cached in 00E198D0, reached through 004e1ca0 and built by 004e18e0.
// The seven update passes below are called on it once each per frame, in the
// order given here, each after its own fresh 004e1ca0 call.
//
// The subsystem stores hint identifiers, not achievement ids: 00690fd0 resolves
// each identifier through a definition map, then through the localisation table
// and the HintSystem Lua binding (docs/GAME_FRONTEND_STATES.md records the same
// entry point advancing BASICPLANE to BASICPLANE2 when the pause menu opens).
// The names kept here are the raw identifiers the native pushes.
//
// Evidence, native ABI and uncertainty: docs/GAME_AWARD_TRACKERS.md.
namespace bsp {

// ---------------------------------------------------------------------------
// Native layout
// ---------------------------------------------------------------------------

// 004e1ca0 allocates 0A0h bytes before calling 004e18e0, so this is the whole
// object. Offsets below are the ones 004e18e0 writes or a pass reads; the gaps
// are members no routine in this packet touches.
inline constexpr std::size_t kAwardTrackerInstanceSize = 0xA0;

// Byte offsets established from 004e18e0 and the seven passes. Each std::_Tree
// member is three dwords (allocator pad, head node, size) and each std::deque
// member is five (allocator pad, block map, map size, offset, size); the head
// node sizes come from the _Isnil byte the constructor sets.
enum AwardTrackerOffset : std::size_t {
    kAwardTrackerVTable = 0x00,          // 00CE7F08, one slot: 004e1c80
    kAwardTrackerSuppressFlag = 0x04,    // byte, read and cleared by 00690fd0
    kAwardTrackerLastUnitClass = 0x18,   // int, 00692b60 only
    kAwardTrackerDefinitions = 0x30,     // map<NativeString, def>, value 128h
    kAwardTrackerCooldowns = 0x3C,       // map<NativeString, float>
    kAwardTrackerActiveList = 0x48,      // list<NativeString>
    kAwardTrackerSecondList = 0x54,      // list<NativeString>, unread here
    kAwardTrackerActiveName = 0x60,      // NativeString, 8 bytes
    kAwardTrackerQueue = 0x68,           // deque<NativeString>
    kAwardTrackerQueueSize = 0x78,       // dword inside the deque
    kAwardTrackerMapA = 0x7C,            // map, value 14h, unread here
    kAwardTrackerMapB = 0x88,            // map, value 14h, unread here
    kAwardTrackerMapC = 0x94,            // map, value 14h, unread here
};

// 004e18e0 leaves +4h and +18h alone. +18h is the previous unit class id that
// 00692b60 compares against, so the first comparison of a fresh instance reads
// uninitialised heap. Reproduced here as an explicit sentinel instead.
inline constexpr int kAwardTrackerNoUnitClass = -1;

// ---------------------------------------------------------------------------
// Frame gates shared by six of the seven passes
// ---------------------------------------------------------------------------

// 004bca50 on 00E188A8 returns the effective game mode. Every pass except the
// zone pass 00692fd0 returns immediately when it is this value.
inline constexpr int kAwardTrackerSuppressedGameMode = 9;

// Category indices passed to the local unit's vtable slot at +5Ch (00E188D8).
// They are not the class ids of kUnitClassHints; the two spaces are unrelated.
enum AwardTrackerUnitCategory : int {
    kUnitCategoryShip = 0x06,
    kUnitCategorySubmarine = 0x08,
    kUnitCategoryPlane = 0x0F,
    kUnitCategoryReconPlane = 0x0B,
    kUnitCategoryPlaneAlt = 0x18,
    kUnitCategoryGunnery = 0x1B,
};

// ---------------------------------------------------------------------------
// Cooldown pass 0068ec10
// ---------------------------------------------------------------------------

// One entry of the map at +3Ch. The native key is a NativeString compared with
// _stricmp; the pointer here stands for it and must outlive the table.
struct HintCooldown {
    const char* name{nullptr};
    float remaining{0.0f};
};

// 0068ec10, __thiscall, one float argument, RET, no return value. It subtracts
// the raw OnMove delta from every entry, collects the entries that went
// strictly below zero into a temporary list, then erases each collected key
// from the map. An entry sitting at exactly 0.0f survives the frame: the native
// test is COMISS 0.0, value / JBE skip, so only value < 0.0f expires.
// Returns the number of surviving entries, compacted to the front of the array.
std::size_t tick_hint_cooldowns_0068ec10(HintCooldown* entries, std::size_t count,
    float raw_delta) noexcept;

// ---------------------------------------------------------------------------
// Queue drain pass 00692b00
// ---------------------------------------------------------------------------

// Inputs of 00692b00, __fastcall (ECX = tracker), no stack arguments, RET.
struct QueuedHintDrainInputs {
    int game_mode{0};                  // 004bca50
    bool transition_blend_active{false}; // 005b5d50 on *(00E198C4+A4h)
    std::size_t queue_size{0};         // +78h
    bool hint_active{false};           // +60h length != 0
};

// True when the pass pops the front of the deque at +68h and hands it to
// 00690fd0 with the forced flag set (the push of 1 at 00692b37 survives the
// front() call and becomes 00690fd0's third argument), then pop_front.
bool should_drain_queued_hint_00692b00(const QueuedHintDrainInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Unit class pass 00692b60
// ---------------------------------------------------------------------------

// The three category hints tried before the class table, in native order.
// Each is skipped when 007f8e00 already finds it in the profile's unlock set at
// game+650h, and each is recorded with the forced flag clear.
struct CategoryHint {
    int category;      // argument to the local unit vtable slot +5Ch
    const char* name;
};
extern const CategoryHint kBasicCategoryHints[3]; // BASICSUB, BASICPLANE, BASICSHIP
inline constexpr std::size_t kBasicCategoryHintCount = 3;

// Class ids returned by the virtual at slot 0 of the sub-object at player+170h.
struct UnitClassHint {
    int class_id;
    const char* name; // nullptr where the id needs the sub-dispatch below
};
// Native order of the compare chain at 00692e4b..00692f72.
extern const UnitClassHint kUnitClassHints[15];
inline constexpr std::size_t kUnitClassHintCount = 15;

// Class 7 splits on 007edad0 (the aircraft payload kind) and class 0Ch on the
// byte at *(*(player+3D0h)+538h)+144h.
inline constexpr int kUnitClassLevelBomber = 0x07;
inline constexpr int kUnitClassKamikaze = 0x0C;
inline constexpr int kPayloadParatrooper = 5;
inline constexpr int kPayloadOhka = 6;

struct UnitClassHintInputs {
    int class_id{0};
    int payload_kind{0};       // 007edad0, read only for class 7
    bool ohka_variant{false};  // read only for class 0Ch
};

// Returns the hint identifier for a class id, or nullptr when the chain falls
// through (any id outside the table records nothing).
const char* unit_class_hint_00692b60(const UnitClassHintInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Weapon pass 006926f0
// ---------------------------------------------------------------------------

// Surface branch, taken when the unit is category 6 or 1Bh. The selector is the
// dword at *(00E198C4+50h)+44h.
const char* surface_weapon_hint_006926f0(int weapon_mode) noexcept;

// Air branch, taken when the unit is category 0Fh or 18h. The selector is
// 007bca50 on the local unit, or on *(player+3D0h) when the unit is category
// 18h. The fallback needs the byte at unit+C24h.
const char* air_weapon_hint_006926f0(int projectile_kind, bool machinegun_ready) noexcept;

inline constexpr int kProjectileBomb = 0x09;
inline constexpr int kProjectileTorpedo = 0x0A;
inline constexpr int kProjectileRocket = 0x12;
inline constexpr int kProjectileDepthCharge = 0x0B;
inline constexpr int kProjectileParatrooperA = 0x0C;
inline constexpr int kProjectileParatrooperB = 0x0F;

// ---------------------------------------------------------------------------
// Environment pass 00692580
// ---------------------------------------------------------------------------

// The two scalars come from the sub-object at player+A20h: 00939f70 reads the
// float at +38h and 00939f80 the float at +34h. The native converts each with
// _ftol before testing it against zero, so a reading inside (-1.0, 1.0)
// truncates to 0 and does not trip. Both are passed here untruncated.
struct EnvironmentHintInputs {
    float water_amount{0.0f};   // 00939f80, +34h
    float fire_amount{0.0f};    // 00939f70, +38h
    bool submerged{false};      // category 8 and the dword at player+122Ch == 2
    bool engine_signal{false};  // byte at player+9E5h
};

// Priority order: WATER, FIRE, PERISCOPE, ENGINE. Returns nullptr when none
// applies. The caller records the result only when it differs, case
// insensitively, from the name already at +60h.
const char* environment_hint_00692580(const EnvironmentHintInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Zone pass 00692fd0
// ---------------------------------------------------------------------------

// One node of the list at *(*(00E188A8+19CCh)+16Ch). Positions are floats at
// +FCh, +100h and +104h; both radii are stored as int and squared after FILD.
struct CaptureZoneSample {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    std::int32_t landing_radius{0}; // +7C4h
    std::int32_t capture_radius{0}; // +7A0h
    std::int32_t state{0};          // +54h, only state 2 counts
};

inline constexpr int kCaptureZoneActiveState = 2;

struct ZoneProximity {
    bool in_landing_radius{false};
    bool in_capture_radius{false};
};

// Walks the zone list the way 006930f0..0069329a does. A zone counts when its
// state is 2 and the player is inside landing_radius. Inside such a zone the
// pass also tests capture_radius, and a failure there breaks the whole loop
// rather than continuing to the next zone; that early exit is native and is
// reproduced. Distances are squared, so no square root is taken.
ZoneProximity scan_capture_zones_00692fd0(float px, float py, float pz,
    const CaptureZoneSample* zones, std::size_t count) noexcept;

// The two identifiers this pass can record. CAPTURE1STGET wins when the player
// is inside a capture radius; LANDING1STGET needs the landing radius only, but
// also requires the unit to be category 0Bh (recon plane).
extern const char kCaptureFirstGetHint[];  // "CAPTURE1STGET"
extern const char kLandingFirstGetHint[];  // "LANDING1STGET"

// ---------------------------------------------------------------------------
// Strategic map pass 00692960
// ---------------------------------------------------------------------------

extern const char kStrategicMapFirstGetHint[]; // "SM1STGET"

struct StrategicMapHintInputs {
    int game_mode{0};                    // 004bca50
    bool transition_blend_active{false}; // 005b5d50
    bool already_shown{false};           // 0068e600
    bool map_available{false};           // 0066ffa0 on *(00E198C4+BCh)
    bool map_open{false};                // 00652a30, the byte at that object +8h
    bool hint_active{false};             // +60h length != 0
};

// True when the pass records SM1STGET. The native also refuses when the name at
// +60h already equals it, which the hint_active flag covers.
bool should_show_strategic_map_hint_00692960(const StrategicMapHintInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Frame sequence 004e525e..004e52ba
// ---------------------------------------------------------------------------

// One method per native call site inside the award block of GGame::OnMove, in
// native order. resolve_singleton stands for the seven separate 004e1ca0 calls,
// which is why it is listed once but called seven times. Nothing here has a
// default: none of these is a stand-in for unrecovered behaviour.
struct AwardTrackerFrameHost {
    virtual ~AwardTrackerFrameHost() = default;
    virtual void resolve_singleton() = 0;                   // 004e1ca0
    virtual void tick_cooldowns(float raw_delta) = 0;       // 0068ec10
    virtual void drain_queued_hint() = 0;                   // 00692b00
    virtual void update_unit_class_hint() = 0;              // 00692b60
    virtual void update_weapon_hint() = 0;                  // 006926f0
    virtual void update_environment_hint() = 0;             // 00692580
    virtual void update_zone_first_get_hints() = 0;         // 00692fd0
    virtual void update_strategic_map_first_get_hint() = 0; // 00692960
};

// The award block itself. It sits behind the simulation gate at 004e50b0:
// every failing test there jumps to 004e53b4 and skips the block, while both
// branches of the pause test reach 004e525e (004e517a jumps into it, 004e5240
// falls into it). Only 0068ec10 receives a delta, and it receives the raw
// OnMove argument at [esp+48h], not the scaled one the world tick uses.
void run_award_tracker_frame(AwardTrackerFrameHost& host, float raw_delta);
}
