#pragma once

// Non-gameplay leaves from 00800000-008FFFFF reconstructed by cc13-sweep8E:
// the scene-text escape and hex byte decoder used by the scene property
// writer/parser, the segment-triangle test, the effect component category
// table and the objective-sync message type ids. Names are hypotheses, not
// recovered symbols. Evidence, ABIs and open questions are in
// docs/CC13_BAND_0080_008F.md. None is a drop-in binary replacement: these are
// C++ interfaces over models of the native data.
//
// x87 model: the image runs with 53-bit precision; every FSTP to a float slot is
// a float cast, and the intermediates in between are modelled as double.

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace bsp::cc13_band0080 {

// 008EF6A0, __fastcall(CL = character) -> int. '0'-'9', 'a'-'f' and 'A'-'F'
// give 0..15; anything else gives 0.
int hex_digit_value_008ef6a0(char c) noexcept;

// 008EF6D0, __fastcall(ECX = const char* token) -> AL. Decodes one `BIN`
// (type 8) token of a scene property file. A null or empty token gives 0. The
// first character is a hex digit (0 when it is not one); when a second
// character exists the result is (first << 4) | digit(second), truncated to a
// byte. Characters after the second are ignored.
std::uint8_t decode_hex_byte_token_008ef6d0(const char* token) noexcept;

// 008EE5F0, __fastcall(ECX = text, EDX = max characters) -> char*. Copies at
// most `max_characters` input characters, writing newline as "\n", double
// quote as "\q" and backslash as "\\". The image writes into the static
// buffer 00F88C50 without a bound and returns it; the model returns a string.
// A null text or a zero count gives the empty string.
std::string escape_scene_text_008ee5f0(const char* text, std::int32_t max_characters);

struct Vec3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// 0085D020, __fastcall(ECX = p0, EDX = p1, [esp+4] a, [esp+8] b, [esp+0Ch] c,
// [esp+10h] out) -> AL, RET 10h. Segment p0..p1 against triangle a, b, c
// (Moller-Trumbore order: h = d x e2, det = e1 . h). Rejects when
// -1e-4 < det < 1e-4 (doubles 00D0D098 / 00D7A268), when u is outside [0, 1]
// (upper bound float 00D7A24C), when v < 0 or u + v > 1, and when t is
// outside [0, 1]. On a hit writes p0 + t * (p1 - p0) and returns true; on a
// miss `out` is untouched. Sole caller BSP_GeomMeshElement_TraceSegmentNode.
bool segment_intersect_triangle_0085d020(const Vec3f& p0, const Vec3f& p1, const Vec3f& a,
                                         const Vec3f& b, const Vec3f& c, Vec3f& out) noexcept;

// Slot +10h of each effect component vtable (docs/GAMEPLAY_EFFECT_COMPONENTS.md
// gives the vtables): a constant per component type.
struct EffectComponentCategory {
    std::string_view component;
    std::uint32_t vtable;
    std::uint32_t slot10_address;
    std::int32_t category;
};

inline constexpr std::array<EffectComponentCategory, 13> kEffectComponentCategories{{
    {"Particle", 0x00D0D5B4, 0x0086BC50, 1},
    {"Sound", 0x00D0DA18, 0x0086FA80, 2},
    {"Tracer", 0x00D0D590, 0x0086B990, 3},
    {"WaterTracer", 0x00D0D5D4, 0x0086BD10, 4},
    {"Flare", 0x00D0D614, 0x0086BDB0, 5},
    {"Shake", 0x00D0D5F4, 0x0086BD60, 6},
    {"ConstRumble", 0x00D0D76C, 0x0086CFC0, 6},
    {"SlopeRumble", 0x00D0D78C, 0x0086D010, 6},
    {"SquareRumble", 0x00D0D7AC, 0x0086D060, 6},
    {"ThunderStorm", 0x00D0DA38, 0x0086FC20, 0},
    {"Waterdrops", 0x00D0D634, 0x0086BDF0, 0},
    {"Light", 0x00D0D654, 0x0086BE30, 0},
    {"Splash", 0x00D0D674, 0x0086BE70, 0},
}};

// Category for a component name, or -1 for a name not in the table.
std::int32_t effect_component_category(std::string_view component) noexcept;

// Objective-sync message types (the MT_ name table at 00E0AB68), constructed
// by 008E1380 and 008DD6A0..008DDB00 and routed on receipt by 008E2390.
enum class ObjectiveSyncMessage : std::uint8_t {
    Create = 0x37,          // MT_OBJ_CREATE, SessionMessage55
    Add = 0x38,             // MT_OBJ_ADD, SessionMessage56
    AddEntity = 0x39,       // MT_OBJ_ADD_ENTITY, SessionMessage57
    RemoveEntity = 0x3A,    // MT_OBJ_REMOVE_ENTITY, SessionMessage58
    AddPosition = 0x3B,     // MT_OBJ_ADD_POSITION, SessionMessage59
    RemovePosition = 0x3C,  // MT_OBJ_REMOVE_POSITION, SessionMessage60
    Completed = 0x3D,       // MT_OBJ_COMPLETED, SessionMessage61
    Failed = 0x3E,          // MT_OBJ_FAILED, SessionMessage62
};

// True for the eight type ids 008E2390 tests (37h..3Eh).
constexpr bool is_objective_sync_message(std::uint8_t type) noexcept
{
    return type >= 0x37 && type <= 0x3E;
}

// ---------------------------------------------------------------------------
// Gameplay leaves from 00800000-0084FFFF, cc13-gp80 (docs/CC13_GP_0080_0084.md).
// Build-tested only: not wired into any host, not fixture- or game-validated.
// ---------------------------------------------------------------------------

// 00805990 BSP_Party_FromName, by-value NativeString {length, buffer} on the
// stack, RET 8, -> EAX. Compares against the party table at 00E0B080 ("OWN",
// "ENEMY", "NEUTRAL", "UNKNOWN") with _stricmp and returns 0..3, or 4 when no
// entry matches. A null buffer matches an empty table entry; the table has
// none, so it gives 4. The image also returns the buffer to the sized
// storage pool (the callee owns the by-value string); the model does not own it.
std::int32_t party_from_name_00805990(const char* buffer) noexcept;

// 0080B190 BSP_TurnPathPlanner_WrapAngle, __fastcall(ECX = float* angle).
// While the angle is below 0 it adds 2pi (double 00CE3828); otherwise, while
// it is above 2pi, it subtracts 2pi. Each step is rounded to float (FSTP to
// the stack slot, then FST back), so the model works in double and casts.
// A NaN angle is left as it is. Part of an unreferenced planner.
void turn_path_wrap_angle_0080b190(float& angle) noexcept;

// 008127E0 BSP_Unit_RefreshFirepowerRatios, __fastcall(ECX = unit). Barrel
// totals live at unit+103Ch..+1048h (written at bind time by 00812310..008124C0);
// the live sums come from 00812550 (artillery), 00812600 (aa mounts),
// 00812680 (torpedo) and 00812730 (depth charge).
struct FirepowerInputs {
    float live[4] = {};    // ST0 results of 00812550, 00812600, 00812680, 00812730
    float total[4] = {};   // unit+103Ch, +1040h, +1044h, +1048h
    bool engine_jammed = false;  // unit+9E5h
    float engine = 0.0f;         // unit+9D8h ("engine", 008131E0)
    float steering = 0.0f;       // unit+9DCh ("steering", 008131E0)
};
struct FirepowerRatios {
    float ratio[4] = {};         // unit+101Ch, +1020h, +1024h, +1028h
    float engine_factor = 0.0f;  // unit+1014h
};
FirepowerRatios refresh_firepower_ratios_008127e0(const FirepowerInputs& in) noexcept;

// 0084EB80 BSP_Stationary_CheckDestroyed, __fastcall(ECX = stationary).
// protection = +150h, max_damage = +17Ch, damage = +1A0h (in/out). With
// protection > 0 the damage is capped at (float)((float)(1 - protection) *
// max_damage) and the object never dies here; otherwise damage >= max_damage
// clamps it, sets +1A4h and calls vtable +70h(1).
enum class StationaryDamageResult { Unchanged, Clamped, Destroyed };
StationaryDamageResult stationary_check_destroyed_0084eb80(float protection, float max_damage,
                                                           float& damage) noexcept;

// 00828EC0 BSP_VehicleClass_IsTorpedoProofSmallCraft, __fastcall(ECX = class).
// The class vtable +18h test is made with 0Eh first and with 0Ch only when
// that fails; a 0Ch class also needs the byte +808h clear. The float +500h
// must then be at least 00D099AC (8.938947f); COMISS/JB makes NaN fail.
// docs/ATTACK_COMMANDS.md: a torpedo order needs this to answer false.
inline constexpr float kTorpedoProofSmallCraftThreshold = 8.938946723937988f;  // float 00D099AC
bool torpedo_proof_small_craft_00828ec0(bool is_kind_0e, bool is_kind_0c, bool byte_808,
                                        float value_500) noexcept;

// Shipyard records (docs/GUNNERY_OPEN_ITEMS.md section 134.1). Only the
// fields the leaves below read are modelled; class_id is [record class]+70h.
struct ShipyardStockRecord {   // +774h vector, stride 20h
    std::int32_t class_id = 0; // [+4h]+70h
    std::int32_t count = 0;    // +8h
};
struct ShipyardQueueSlot {     // +794h vector, stride 4Ch
    std::int32_t state = 0;    // +4h
    bool has_class = false;    // +8h non-null
    std::int32_t class_id = 0; // [+8h]+70h
};

// 008457E0 BSP_Shipyard_ConsumeStock, __thiscall(shipyard, classId, count),
// RET 8. The first record of the class loses `count`, clamped at 0; nothing
// happens when no record matches.
void shipyard_consume_stock_008457e0(ShipyardStockRecord* records, std::size_t n,
                                     std::int32_t class_id, std::int32_t count) noexcept;

// 00844610 BSP_Shipyard_CountAvailableOfClass, __thiscall(shipyard, classId)
// -> int, RET 4: the counts of every matching stock record plus the queue
// slots in state 1 that hold the class.
std::int32_t shipyard_count_available_00844610(const ShipyardStockRecord* records, std::size_t n_records,
                                               const ShipyardQueueSlot* slots, std::size_t n_slots,
                                               std::int32_t class_id) noexcept;

// 008434D0 BSP_Shipyard_CountIdleSlots, __fastcall(ECX = shipyard) -> int:
// queue slots whose state is 0.
std::int32_t shipyard_count_idle_slots_008434d0(const ShipyardQueueSlot* slots, std::size_t n) noexcept;

// 0084CF80 BSP_SupportSite_SquadSizeForClass, __thiscall(site, class) -> int,
// RET 4. Searches the std::list at site+328h, then the one at site+31Ch, for a
// node whose +8h equals class+70h and returns the node's +10h; 3 when neither
// list has it (the SquadSize default 0084D170 writes).
struct SupportSiteListEntry {
    std::int32_t class_id = 0;   // node +8h
    std::int32_t squad_size = 0; // node +10h
};
std::int32_t support_site_squad_size_0084cf80(const SupportSiteListEntry* list_328, std::size_t n_328,
                                              const SupportSiteListEntry* list_31c, std::size_t n_31c,
                                              std::int32_t class_id) noexcept;

} // namespace bsp::cc13_band0080
