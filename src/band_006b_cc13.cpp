// Leaf helpers from the 006B0000-0070FFFF gameplay band (cc13-gp6b).
// Reconstructed and build-tested only: not ABI-compatible, not fixture- or game-validated, and not
// wired into any game host. See docs/CC13_GP_006B_0070.md.

#include "bsp/band_006b_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/native_bit_cursor_fields.hpp"

#include <cstring>
#include <utility>

#include "bsp/band_0067_cc13.hpp"

namespace bsp::band_006b_cc13 {
namespace {
template<class T> T airbase_load(const void* p,std::size_t offset) noexcept {
    T value; std::memcpy(&value,static_cast<const std::uint8_t*>(p)+offset,sizeof value); return value;
}
template<class T> T* airbase_field(void* p,std::size_t offset) noexcept {
    return reinterpret_cast<T*>(static_cast<std::uint8_t*>(p)+offset);
}
void airbase_write_common(const void* p,NativeBitCursor* cursor) {
    write_native_session_message_header_0075b480(p,cursor);
    write_native_bool_bit_004290b0(cursor,airbase_load<std::uint8_t>(p,0x1c));
}
void airbase_read_common(void* p,NativeSessionReadStream* stream) {
    read_native_session_message_header_0075b4c0(p,stream);
    read_native_bool_bit_00428d70(&stream->cursor_04,airbase_field<bool>(p,0x1c));
}
}
void airbase_do_launch_slot_write_006bc440(const void* p,NativeBitCursor* c) {
    airbase_write_common(p,c);
    write_native_unsigned_dword_bits_00429070(c,airbase_load<std::uint32_t>(p,0x20),8);
}
void airbase_do_launch_slot_read_006bc470(void* p,NativeSessionReadStream* s) {
    airbase_read_common(p,s);
    read_native_u32_bits_00428d10(&s->cursor_04,airbase_field<std::uint32_t>(p,0x20),8);
}
void airbase_set_slot_write_006bd8c0(const void* p,NativeBitCursor* c) {
    airbase_write_common(p,c);
    write_native_unsigned_dword_bits_00429070(c,airbase_load<std::uint32_t>(p,0x20),6);
    write_native_unsigned_dword_bits_00429070(c,airbase_load<std::uint32_t>(p,0x24),10);
    write_native_unsigned_dword_bits_00429070(c,airbase_load<std::uint32_t>(p,0x28),6);
    write_native_unsigned_dword_bits_00429070(c,airbase_load<std::uint32_t>(p,0x2c),4);
}
void airbase_set_slot_read_006bd920(void* p,NativeSessionReadStream* s) {
    airbase_read_common(p,s);
    read_native_u32_bits_00428d10(&s->cursor_04,airbase_field<std::uint32_t>(p,0x20),6);
    read_native_u32_bits_00428d10(&s->cursor_04,airbase_field<std::uint32_t>(p,0x24),10);
    read_native_u32_bits_00428d10(&s->cursor_04,airbase_field<std::uint32_t>(p,0x28),6);
    read_native_u32_bits_00428d10(&s->cursor_04,airbase_field<std::uint32_t>(p,0x2c),4);
}
void airbase_stock_item_write_006bd9a0(const void* p,NativeBitCursor* c) {
    airbase_write_common(p,c);
    write_native_unsigned_dword_bits_00429070(c,airbase_load<std::uint32_t>(p,0x20),10);
    write_native_unsigned_dword_bits_00429070(c,airbase_load<std::uint32_t>(p,0x24),8);
}
void airbase_stock_item_read_006bda80(void* p,NativeSessionReadStream* s) {
    airbase_read_common(p,s);
    read_native_u32_bits_00428d10(&s->cursor_04,airbase_field<std::uint32_t>(p,0x20),10);
    read_native_u32_bits_00428d10(&s->cursor_04,airbase_field<std::uint32_t>(p,0x24),8);
}
bool airbase_type86_is_type_006bdb00(std::uint32_t type) noexcept {
    return type==0x86 || type==0x49 || type==0x46;
}
void airbase_type86_write_006bdb30(const void* p,NativeBitCursor* c) {
    airbase_write_common(p,c);
    write_native_word_bits_00429120(c,airbase_load<std::uint16_t>(p,0x20),12);
}
void airbase_type86_read_006bdb60(void* p,NativeSessionReadStream* s) {
    airbase_read_common(p,s);
    read_native_word_bits_00428e30(&s->cursor_04,airbase_field<std::uint16_t>(p,0x20),12);
}
std::uint32_t airbase_type86_has_entity_bits_006bdb90(const void* p,const ObjectHandleTables& tables) noexcept {
    const auto id=airbase_load<std::uint16_t>(p,0x20);
    const bool low=static_cast<std::int32_t>(id)<tables.first_end_00f89a10;
    const auto base=static_cast<std::uint32_t>(low ? tables.first_begin_00f89a0c : tables.second_begin_00f89a60);
    const void* const slots=low ? tables.first_entries_00f89a54 : tables.second_entries_00f89aa8;
    const std::uint32_t offset=((static_cast<std::uint32_t>(id)-base)<<4)+0xc;
    const auto payload=airbase_load<std::uint32_t>(slots,offset);
    return (payload&0xffffff00u)|(payload!=0 ? 1u : 0u);
}
void airbase_set_orders_write_006bdce0(const void* p,NativeBitCursor* c) {
    airbase_write_common(p,c);
    write_native_bool_bit_004290b0(c,airbase_load<std::uint8_t>(p,0x20));
    write_native_bool_bit_004290b0(c,airbase_load<std::uint8_t>(p,0x21));
    write_native_unsigned_dword_bits_00429070(c,airbase_load<std::uint32_t>(p,0x24),3);
}
void airbase_set_orders_read_006bdd30(void* p,NativeSessionReadStream* s) {
    airbase_read_common(p,s);
    read_native_bool_bit_00428d70(&s->cursor_04,airbase_field<bool>(p,0x20));
    read_native_bool_bit_00428d70(&s->cursor_04,airbase_field<bool>(p,0x21));
    read_native_u32_bits_00428d10(&s->cursor_04,airbase_field<std::uint32_t>(p,0x24),3);
}
NativeSessionMessageStorage* airbase_message_delete_006bdd80(NativeSessionMessageStorage* p,std::uint32_t flags) {
    return delete_native_session_message_root_00449910(p,flags);
}

const TypeIdRule kSessionMessageTypeRules[] = {
    {0x006BC410u, 0x00CF85F0u, 3, {0x89, 0x49, 0x46}},       // BSP_SessionMessage_AIRBASE_DOLAUNCHSLOT_IsType
    {0x006BD890u, 0x00CF8624u, 3, {0x81, 0x49, 0x46}},       // BSP_SessionMessage_AIRBASE_SETSLOT_IsType
    {0x006BDA50u, 0x00CF8638u, 3, {0x85, 0x49, 0x46}},       // BSP_SessionMessage_AIRBASE_STOCKITEM_IsType
    {0x006BDCB0u, 0x00CF8660u, 3, {0x88, 0x49, 0x46}},       // BSP_SessionMessage_AIRBASE_SETORDERS_IsType
    {0x006D16F0u, 0x00CF8B40u, 4, {0x72, 0x62, 0x49, 0x46}}, // BSP_SessionMessage_AIRBASE_SET_RUNWAYFAILURE_IsType
    {0x006D1770u, 0x00CF8B54u, 4, {0x73, 0x62, 0x49, 0x46}}, // BSP_SessionMessage_AIRBASE_CLEAR_RUNWAYFAILURE_IsType
    {0x006D17F0u, 0x00CF8B68u, 4, {0x74, 0x62, 0x49, 0x46}}, // BSP_SessionMessage_AIRBASE_SET_HANGARFAILURE_IsType
    {0x006D1870u, 0x00CF8B7Cu, 4, {0x75, 0x62, 0x49, 0x46}}, // BSP_SessionMessage_AIRBASE_CLEAR_HANGARFAILURE_IsType
    {0x006D44D0u, 0x00D03720u, 2, {0x47, 0x46}},             // BSP_SessionMessage_CREATE_IsType
    {0x006D4770u, 0x00CF8EBCu, 3, {0xA3, 0x47, 0x46}},       // BSP_SessionMessage_AIRFIELD_CREATE_IsType
    {0x006E0660u, 0x00CF9324u, 3, {0xB6, 0x47, 0x46}},       // BSP_SessionMessage_BOMB_CREATE_IsType
    {0x006E3980u, 0x00CF9628u, 3, {0xAD, 0x49, 0x46}},       // BSP_SessionMessage_GUN_FIRE_IsType
    {0x006E3AA0u, 0x00CF963Cu, 3, {0xB2, 0x49, 0x46}},       // BSP_SessionMessage_BOMBPLATFORM_RELOAD_IsType
    {0x006E3B80u, 0x00CF9650u, 2, {0xB3, 0x46}},             // BSP_SessionMessage_MULTIBOMBPLATFORM_GENERATE_IsType
    {0x006E4870u, 0x00CF9B44u, 3, {0xB1, 0x49, 0x46}},       // BSP_SessionMessage_BOMBPLATFORM_FIRE_IsType
    {0x006EBA00u, 0x00CFAA0Cu, 3, {0x7C, 0x49, 0x46}},       // BSP_SessionMessage_VEHICLE_ADD_LAUNCHED_IsType
    {0x006EBAE0u, 0x00CFAA20u, 3, {0xC6, 0x49, 0x46}},       // BSP_SessionMessage_PLANE_TAKEOFF_IsType
    {0x006EBBE0u, 0x00CFAA34u, 3, {0xB4, 0x49, 0x46}},       // BSP_SessionMessage_CATAPULT_FIRE_IsType
    {0x006F2500u, 0x00CFADBCu, 2, {0xD3, 0x46}},             // BSP_SessionMessage_COMMANDBUILDING_SETOWNER_IsType
    {0x006F25C0u, 0x00CFADD0u, 2, {0xD4, 0x46}},             // BSP_SessionMessage_COMMANDBUILDING_SETLEVEL_IsType
    {0x006F2690u, 0x00CFADE4u, 2, {0xD5, 0x46}},             // BSP_SessionMessage_COMMANDBUILDING_UPDATECAPTURE_IsType
    {0x006F4200u, 0x00CFAE84u, 2, {0xD6, 0x46}},             // BSP_SessionMessage_COMMANDBUILDING_INFERIORLIST_IsType
    {0x006FCB30u, 0x00CFBC28u, 3, {0xB7, 0x47, 0x46}},       // BSP_SessionMessage_DEPTHCHARGE_CREATE_IsType
};
const std::size_t kSessionMessageTypeRuleCount = sizeof(kSessionMessageTypeRules) / sizeof(kSessionMessageTypeRules[0]);

const TypeIdRule kProjectileClassKindRules[] = {
    {0x006EA370u, 0x00CFA4DCu, 3, {0x33, 0x2A, 0x29}},             // BSP_RocketClass_IsKindOf
    {0x006EA3E0u, 0x00CFA508u, 3, {0x2C, 0x2A, 0x29}},             // BSP_DepthChargeClass_IsKindOf
    {0x006EA550u, 0x00CFA56Cu, 3, {0x2B, 0x2A, 0x29}},             // BSP_TorpedoClass_IsKindOf
    {0x006EA620u, 0x00CFA59Cu, 3, {0x34, 0x2A, 0x29}},             // BSP_WaterMineClass_IsKindOf
    {0x006EA6F0u, 0x00CFA5CCu, 3, {0x2E, 0x2A, 0x29}},             // BSP_DummyTargetBulletClass_IsKindOf
    {0x006EA760u, 0x00CFA604u, 3, {0x31, 0x2A, 0x29}},             // BSP_ParatrooperClass_IsKindOf
    {0x006EA830u, 0x00CFA63Cu, 5, {0x17, 0x2F, 0x2D, 0x2A, 0x29}}, // BSP_DummyKamikazePlaneClass_IsKindOf
    {0x006EA8B0u, 0x00CFA678u, 4, {0x08, 0x30, 0x2A, 0x29}},       // BSP_DummySubmarineClass_IsKindOf
};
const std::size_t kProjectileClassKindRuleCount = sizeof(kProjectileClassKindRules) / sizeof(kProjectileClassKindRules[0]);

bool type_id_rule_accepts(const TypeIdRule& rule, std::int32_t type) noexcept {
    for (std::uint8_t i = 0; i < rule.count; ++i) {
        if (type == static_cast<std::int32_t>(rule.ids[i])) {
            return true;
        }
    }
    return false;
}

const TypeIdRule* find_type_id_rule(std::uint32_t address) noexcept {
    for (std::size_t i = 0; i < kSessionMessageTypeRuleCount; ++i) {
        if (kSessionMessageTypeRules[i].address == address) {
            return &kSessionMessageTypeRules[i];
        }
    }
    for (std::size_t i = 0; i < kProjectileClassKindRuleCount; ++i) {
        if (kProjectileClassKindRules[i].address == address) {
            return &kProjectileClassKindRules[i];
        }
    }
    return nullptr;
}

namespace {

constexpr std::int32_t kSlotStateQueued = 2;   // AirOpsSlotState::kLaunching
constexpr std::int32_t kSlotStateParked = 6;   // 006CB28B / 006C8326; not in AirOpsSlotState

std::int32_t state_of(const AirOpsSlot& slot) noexcept {
    return static_cast<std::int32_t>(slot.state);
}

bool squadron_away(const AirOpsSlot& slot) noexcept {
    return slot.launched_squadron != 0u &&
           (slot.state == AirOpsSlotState::kLaunched || slot.state == AirOpsSlotState::kRecalled);
}

} // namespace

int air_ops_count_idle_slots_006bdc30(const AirOpsSlot* slots, int slot_count) noexcept {
    // 006BDC33..006BDC58: walk +2Ch with the 58h stride; count state 1 or 5.
    int idle = 0;
    for (int i = 0; i < slot_count; ++i) {
        const AirOpsSlotState s = slots[i].state;
        if (s == AirOpsSlotState::kCooldown || s == AirOpsSlotState::kReady) {
            ++idle;
        }
    }
    return idle;
}

int air_ops_count_planes_airborne_006bf4c0(const AirOpsSlot* slots,
                                           const std::int32_t* launched_plane_counts,
                                           int slot_count) noexcept {
    int planes = 0;
    for (int i = 0; i < slot_count; ++i) {
        if (squadron_away(slots[i])) {
            planes += launched_plane_counts[i]; // squadron+3CCh
        } else if (state_of(slots[i]) == kSlotStateParked) {
            planes += slots[i].assigned_count;  // slot+8h
        }
    }
    return planes;
}

int air_ops_count_planes_airborne_or_queued_006bf540(const AirOpsSlot* slots,
                                                     const std::int32_t* launched_plane_counts,
                                                     int slot_count) noexcept {
    int planes = 0;
    for (int i = 0; i < slot_count; ++i) {
        if (squadron_away(slots[i])) {
            planes += launched_plane_counts[i];
        } else if (state_of(slots[i]) == kSlotStateParked || state_of(slots[i]) == kSlotStateQueued) {
            planes += slots[i].assigned_count;
        }
    }
    return planes;
}

void air_ops_reset_slots_to_deck_006bd360(AirOpsSlot* slots, int slot_count) noexcept {
    for (int i = 0; i < slot_count; ++i) {
        AirOpsSlot& slot = slots[i];
        slot.state = AirOpsSlotState::kCooldown; // +2Ch = 1
        slot.timer = 0.0F;                       // +30h = 0
        if (slot.launch_requested) {             // +34h
            slot.timer = kAirOpsSlotCooldownSeconds;
            slot.launch_requested = false;
        }
        if (slot.vehicle_class != 0u) {          // +4h
            slot.vehicle_class = 0u;
            slot.class_field_134 = 0;            // +10h
        }
        slot.assigned_count = 0;                 // +8h
    }
}

void air_ops_slot_clear_006c4f30(AirOpsSlot& slot) noexcept {
    slot.launched_squadron = 0u; // after BSP_Observer_UnregisterPair when set
    slot.assigned_count = 0;
    slot.state = AirOpsSlotState::kCooldown;
    slot.timer = kAirOpsSlotCooldownSeconds;
    slot.launch_requested = false;
}

float air_ops_approach_setback_006ba620(float runway_length, bool owner_is_mothership) noexcept {
    // FLD [ESI+B4h]; FMUL double [00CE3DC8] or [00CE65D0]; FSTP float [ESP]; FLD.
    constexpr double kMothership = 0.30000001192092896; // 00CE3DC8
    constexpr double kOther = 0.4000000059604645;       // 00CE65D0
    return static_cast<float>(static_cast<double>(runway_length) * (owner_is_mothership ? kMothership : kOther));
}

float math_min_float_int_006f22b0(std::int32_t bound, float v) noexcept {
    const float b = static_cast<float>(bound); // FILD; FSTP float [ESP]
    return b > v ? v : b;                      // FCOMIP ST0(b),ST1(v); JBE -> bound
}

float math_max_float_int_006f22f0(std::int32_t bound, float v) noexcept {
    const float b = static_cast<float>(bound);
    return v > b ? v : b; // FCOMIP ST0(v),ST1(b); JBE -> bound
}

void vec3_add_scaled_in_place_006e0540(float v[3], const float d[3], float s) noexcept {
    band_0067_cc13::Vec3 vv{v[0], v[1], v[2]};
    const band_0067_cc13::Vec3 dd{d[0], d[1], d[2]};
    band_0067_cc13::vec3_add_scaled_in_place_00709b60(vv, dd, s);
    v[0] = vv.x;
    v[1] = vv.y;
    v[2] = vv.z;
}

float* matrix4x4_transpose_in_place_006fdd00(float m[16]) noexcept {
    // 006FDD02..006FDD5D: (1,4) (2,8) (3,12) (6,9) (7,13) (11,14). In each pair the lower
    // element goes up through FLD/FSTP DWORD, which quiets a signalling NaN (bit 22), and
    // the upper element goes down through MOVSS unchanged (cc13-fixture9).
    auto* w = reinterpret_cast<std::uint32_t*>(m);
    const auto swap_pair = [w](int up, int down) {
        const std::uint32_t u = w[up];
        const std::uint32_t d = w[down];
        w[up] = cc13x87::x87_fld_fstp_bits(d);
        w[down] = u;
    };
    swap_pair(1, 4);
    swap_pair(2, 8);
    swap_pair(3, 12);
    swap_pair(6, 9);
    swap_pair(7, 13);
    swap_pair(11, 14);
    return m;
}

int entity_nth_part_of_type_0c_006d2120(const std::int32_t* part_types, std::size_t part_count,
                                        int n) noexcept {
    int seen = 0;
    for (std::size_t i = 0; i < part_count; ++i) {
        if (part_types[i] == 0x0C) {
            if (seen == n) {
                return static_cast<int>(i);
            }
            ++seen;
        }
    }
    return -1;
}

} // namespace bsp::band_006b_cc13
