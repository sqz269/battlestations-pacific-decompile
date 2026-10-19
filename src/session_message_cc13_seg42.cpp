// Segment 42 session-message classes (cc13-seg42). Every body below was read
// against its listing; per-function ABI notes and uncertainty are in
// include/bsp/session_message_cc13_seg42.hpp and docs/CC13_SEG42.md.
#include "bsp/session_message_cc13_seg42.hpp"

#include <array>
#include <cstring>
#include <vector>

#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_owned_string.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp::seg42 {
namespace {
using U = std::uint32_t;
using B = std::uint8_t;
using S = NativeSessionMessageStorage;
using C = NativeBitCursor;
using R = NativeSessionReadStream;

U bits(float value) { U out; std::memcpy(&out, &value, 4); return out; }
float f32(U value) { float out; std::memcpy(&out, &value, 4); return out; }
U* fbits(float* value) { return reinterpret_cast<U*>(value); }
const U* fbits(const float* value) { return reinterpret_cast<const U*>(value); }
bool* boolp(B* value) { return reinterpret_cast<bool*>(value); }

const U* vt(const Context& c, U image_address) { return c.vtables.vtable(image_address); }

// The inlined 0075B430: delivery 3, +8/+0Ch zero, base vtable 00D02C68, type,
// then the selected slot record from game+18CCh[game+18ECh] when 0..7.
void base(S* m, U type, const Context& c) {
    NativeSessionMessageContext session{c.current_game_00e188a8};
    construct_native_session_message_0075b430(m, type, session);
}
void finish(S* m, U delivery, const U* profile) { m->delivery_04 = delivery; m->profile_00 = profile; }
void zero_header(HeaderedMessage* h) { h->id_18 = 0; h->relay_1a = 0; }

// ConstructForRead prologue: +8, +0Ch, +14h, type byte, then the 12-bit id
// word and relay byte. No session read.
void for_read(S* m) { m->field_08 = 0; m->field_0c = 0; m->selected_owner_14 = nullptr; m->type_10 = 0; }
void for_read(HeaderedMessage* h) { for_read(&h->base); zero_header(h); }
void for_read_unit(UnitMessage* u, U image_vtable, const Context& c) {
    for_read(&u->h);
    u->h.base.delivery_04 = 1;
    u->flag_1c = 0;
    u->h.base.profile_00 = vt(c, image_vtable);
}

template <class M> M* construct_headered(M* m, HeaderedMessage* h, U type, U delivery, U image_vtable, const Context& c) {
    base(&h->base, type, c);
    zero_header(h);
    finish(&h->base, delivery, vt(c, image_vtable));
    return m;
}
template <class M> M* construct_unit(M* m, UnitMessage* u, U type, U image_vtable, const Context& c) {
    base(&u->h.base, type, c);
    zero_header(&u->h);
    u->flag_1c = 0;
    finish(&u->h.base, 1, vt(c, image_vtable));
    return m;
}
template <class M> M* construct_plain(M* m, S* s, U type, U delivery, U image_vtable, const Context& c) {
    base(s, type, c);
    finish(s, delivery, vt(c, image_vtable));
    return m;
}

// Category tests: the class type, optionally a category type, and 46h.
bool is3(U t, U a, U b) { return t == a || t == b || t == 0x46u; }
bool is2(U t, U a) { return t == a || t == 0x46u; }

// Bit helpers in the native call order: type u8 8; header id 12 + bool.
void w_type(const S& m, C* c) { write_native_byte_bits_00428ff0(c, m.type_10, 8); }
void r_type(S& m, C* c) { read_native_u8_bits_00428c70(c, &m.type_10, 8); }
void w_header(const HeaderedMessage& h, C* c) {
    w_type(h.base, c);
    write_native_word_bits_00429120(c, h.id_18, 12);
    write_native_bool_bit_004290b0(c, h.relay_1a);
}
void r_header(HeaderedMessage& h, C* c) {
    r_type(h.base, c);
    read_native_word_bits_00428e30(c, &h.id_18, 12);
    read_native_bool_bit_00428d70(c, boolp(&h.relay_1a));
}
void w_unit(const UnitMessage& u, C* c) { w_header(u.h, c); write_native_bool_bit_004290b0(c, u.flag_1c); }
void r_unit(UnitMessage& u, C* c) { r_header(u.h, c); read_native_bool_bit_00428d70(c, boolp(&u.flag_1c)); }
// Numeric floats: (zero 0, signed 1, scale [00D7A248] = FLT_MAX, 32 bits).
void w_float(C* c, float v, const Numeric& n) { write_native_numeric_float_004295c0(c, bits(v), 0, 1, kFltMaxBits_00d7a248, 32, n); }
void r_float(C* c, float* v, const Numeric& n) { read_native_numeric_float_004293f0(c, fbits(v), 0, 1, kFltMaxBits_00d7a248, 32, n); }
void w_vec3(C* c, const float* v, const Numeric& n) { write_native_numeric_float_array_00429790(c, fbits(v), 3, 0, 1, kFltMaxBits_00d7a248, 32, n); }
void r_vec3(C* c, float* v, const Numeric& n) { read_native_numeric_float_array_004294f0(c, fbits(v), 3, 0, 1, kFltMaxBits_00d7a248, 32, n); }
void w_bool(C* c, B v) { write_native_bool_bit_004290b0(c, v); }
void r_bool(C* c, B* v) { read_native_bool_bit_00428d70(c, boolp(v)); }
void w_u32(C* c, U v, U n) { write_native_unsigned_dword_bits_00429070(c, v, n); }
void r_u32(C* c, U* v, U n) { read_native_u32_bits_00428d10(c, v, n); }
void w_i32(C* c, std::int32_t v, U n) { write_native_signed_dword_bits_00429090(c, static_cast<U>(v), n); }
void r_i32(C* c, std::int32_t* v, U n) { read_native_i32_bits_00428d30(c, v, n); }
void w_word(C* c, std::uint16_t v, U n) { write_native_word_bits_00429120(c, v, n); }
void r_word(C* c, std::uint16_t* v, U n) { read_native_word_bits_00428e30(c, v, n); }
}  // namespace

// ---- ConstructForRead ----------------------------------------------------
void construct_for_read_formation_join_0075a2e0(FormationMemberMessage* m, const Context& c) { for_read_unit(&m->u, 0x00D02D30u, c); }
void construct_for_read_formation_leave_0075a330(FormationMemberMessage* m, const Context& c) { for_read_unit(&m->u, 0x00D02D44u, c); }
void construct_for_read_formation_set_0075a380(FormationSetMessage* m, const Context& c) {
    for_read_unit(&m->u, 0x00CED548u, c);
    m->zero_28 = 0; m->zero_2c = 0;
}
void construct_for_read_kill_bullet_0075a4d0(HeaderedMessage* m, const Context& c) {
    for_read(m);
    m->base.profile_00 = vt(c, 0x00D02D80u);
    m->base.delivery_04 = 1;
}
void construct_for_read_gun_set_fire_mode_0075a550(UnitMessage* m, const Context& c) { for_read_unit(m, 0x00CFDE3Cu, c); }
void construct_for_read_r_t_gun_set_contfiring_0075a580(UnitMessage* m, const Context& c) { for_read_unit(m, 0x00CFDE50u, c); }
void construct_for_read_s_t_gun_fire_0075a5c0(UnitMessage* m, const Context& c) { for_read_unit(m, 0x00CFDE8Cu, c); }
void construct_for_read_ship_sync_0075a630(HeaderedMessage* m, const Context& c) {
    for_read(m);
    finish(&m->base, 0, vt(c, 0x00D02D94u));
}
void construct_for_read_ship_helmsman_control_0075a690(ShipHelmsmanControlMessage* m, const Context& c) {
    for_read(&m->h);
    finish(&m->h.base, 0, vt(c, 0x00D02DA8u));
    m->value_1c = 0.0f; m->value_20 = 0.0f; m->zero_24 = 0;
    std::memset(&m->flag_38, 0, 4);  // native DWORD store at +38h
    m->max_30 = f32(kTwoBits_00ce3958); m->max_28 = f32(kTwoBits_00ce3958);
    m->min_34 = f32(kMinusTwoBits_00ce7d7c); m->min_2c = f32(kMinusTwoBits_00ce7d7c);
}
void construct_for_read_path_set_0075a840(UnitMessage* m, const Context& c) { for_read_unit(m, 0x00D02DBCu, c); }
void construct_for_read_path_add_0075a8b0(UnitMessage* m, const Context& c) { for_read_unit(m, 0x00D02DD0u, c); }
void construct_for_read_path_insert_0075a920(UnitMessage* m, const Context& c) { for_read_unit(m, 0x00D02DE4u, c); }
void construct_for_read_path_delete_0075a990(UnitMessage* m, const Context& c) { for_read_unit(m, 0x00D02DF8u, c); }
void construct_for_read_plane_sync_0075aab0(PlaneSyncMessage* m, const Context& c) {
    for_read(&m->h);
    finish(&m->h.base, 0, vt(c, 0x00D02E0Cu));
    m->zero_38 = 0; m->zero_3c = 0; m->zero_40 = 0; m->zero_48 = 0; m->zero_4c = 0;
    m->one_44 = f32(kOneBits_00d7a24c);
    m->zero_76 = 0; m->zero_75 = 0; m->zero_74 = 0;
    m->zero_7c = 0; m->zero_78 = 0; m->zero_1c = 0; m->zero_9d = 0;
}
void construct_for_read_plane_controlfactor_0075ab40(PlaneControlFactorMessage* m, const Context& c) {
    for_read_unit(&m->u, 0x00D02E20u, c);
    const float one = f32(kOneBits_00d7a24c);
    m->factor_20[2] = one; m->factor_20[1] = one; m->factor_20[0] = one;
    m->one_2c[3] = one; m->one_2c[2] = one; m->one_2c[1] = one; m->one_2c[0] = one;
}
void construct_for_read_plane_hit_0075abe0(PlaneHitMessage* m, const Context& c) { for_read_unit(&m->u, 0x00D02E34u, c); }
void construct_for_read_plane_pcmdlasttime_0075ac50(PlanePcmdLastTimeMessage* m, const Context& c) { for_read_unit(&m->u, 0x00D02E48u, c); }
void construct_for_read_planesquadron_attackrun_0075ad30(UnitMessage* m, const Context& c) { for_read_unit(m, 0x00CF3BB8u, c); }
void construct_for_read_submarine_dip_rise_0075ade0(UnitMessage* m, const Context& c) { for_read_unit(m, 0x00CF61FCu, c); }
void construct_for_read_submarine_dip_rise_sync_0075ae10(DipRiseSyncMessage* m, const Context& c) { for_read_unit(&m->u, 0x00D02E70u, c); }
void construct_for_read_grnd_vehicle_sync_0075aea0(HeaderedMessage* m, const Context& c) {
    for_read(m);
    finish(&m->base, 0, vt(c, 0x00CFFD4Cu));
}
void construct_for_read_landfort_effect_0075aed0(LandfortEffectMessage* m, const Context& c) {
    for_read(&m->h);
    finish(&m->h.base, 1, vt(c, 0x00CFF37Cu));
    m->zero_1c = 0; m->zero_20 = 0;
}
void construct_for_read_landingship_startlanding_0075af00(StartLandingMessage* m, const Context& c) {
    for_read(&m->h);
    finish(&m->h.base, 1, vt(c, 0x00CFF908u));
    m->zero_1c = 0; m->zero_1e = 0;
}
void construct_for_read_playerspawnpoint_setactive_0075b220(SpawnPointSetActiveMessage* m, const Context& c) {
    for_read(&m->h);
    finish(&m->h.base, 1, vt(c, 0x00D02EC0u));
    m->active_20 = 1; m->zero_1c = 0;
}
void construct_for_read_movcamnew_addposition_0075b2a0(MovieAddPositionMessage* m, const Context& c) {
    for_read(&m->base);
    finish(&m->base, 1, vt(c, 0x00D02ED4u));
    m->keyframe_18 = nullptr; m->owns_keyframe_1c = 0;
}

// ---- Construct -------------------------------------------------------------
EntityRelocateMessage* construct_entity_relocate_0075bb30(EntityRelocateMessage* m, const Context& c) { return construct_unit(m, &m->u, 0x97, 0x00D02F24u, c); }
PlayerInfoMessage* construct_playerinfo_0075c680(PlayerInfoMessage* m, const Context& c) {
    construct_plain(m, &m->base, 0xE3, 1, 0x00D02FA8u, c);
    m->minus_one_24 = -1; m->zero_18 = 0; m->zero_1c = 0; m->zero_20 = 0;
    return m;
}
MusicSetLevelMessage* construct_musiccontrol_setlevel_0075ea30(MusicSetLevelMessage* m, const Context& c) { return construct_plain(m, &m->base, 0x2A, 1, 0x00D0319Cu, c); }
BlackoutMessage* construct_blackout_0075eb40(BlackoutMessage* m, const Context& c) { return construct_plain(m, &m->base, 0x2B, 1, 0x00D031B0u, c); }
HandModeTargetedMessage* construct_movcam_handmodetargeted_0075ecb0(HandModeTargetedMessage* m, const Context& c) { return construct_plain(m, &m->base, 0x2D, 1, 0x00D031C4u, c); }
RefPosPolarMessage* construct_movcam_refposmodepolar_0075edf0(RefPosPolarMessage* m, const Context& c) { return construct_plain(m, &m->base, 0x2E, 1, 0x00D031D8u, c); }
RefPosRadiusMessage* construct_movcam_refposmoderadius_0075f000(RefPosRadiusMessage* m, const Context& c) { return construct_plain(m, &m->base, 0x2F, 1, 0x00D031ECu, c); }
PathWalkPropertyMessage* construct_movcam_pathwalkproperty_0075f1a0(PathWalkPropertyMessage* m, const Context& c) { return construct_plain(m, &m->base, 0x30, 1, 0x00D03200u, c); }
AddInterpolatorMessage* construct_movcam_addinterpolator_0075f350(AddInterpolatorMessage* m, const Context& c) { return construct_plain(m, &m->base, 0x31, 1, 0x00D03214u, c); }
SetDeviationFactorMessage* construct_movcam_setdeviationfactor_0075f540(SetDeviationFactorMessage* m, const Context& c) { return construct_plain(m, &m->base, 0x32, 1, 0x00D03228u, c); }
VoiceChangeMessage* construct_voicechange_0075f9d0(VoiceChangeMessage* m, const Context& c) { return construct_plain(m, &m->base, 0xE8, 1, 0x00D03278u, c); }
NativeSessionMessageStorage* construct_playerunlockunit_0075fb00(S* m, const Context& c) { return construct_plain(m, m, 0xDD, 1, 0x00CEB138u, c); }
ServerDlcUnlockMessage* construct_serverdlcunlock_0075fb70(ServerDlcUnlockMessage* m, const Context& c) {
    construct_plain(m, &m->base, 0xDE, 1, 0x00D0328Cu, c);
    std::memset(m->words_18, 0, sizeof(m->words_18));  // REP STOSD 7Fh dwords, then one WORD
    return m;
}
HeaderedMessage* construct_fragile_sethp_0075fc80(HeaderedMessage* m, const Context& c) { return construct_headered(m, m, 0xD2, 1, 0x00D032A0u, c); }
TGunTurnSyncMessage* construct_t_gun_turn_sync_0075fe90(TGunTurnSyncMessage* m, const Context& c) {
    construct_headered(m, &m->h, 0xAE, 0, 0x00D032B4u, c);
    m->zero_1c = 0; m->zero_1d = 0;
    for (U i = 0; i < 5; ++i) m->zero_20[i] = 0;
    m->zero_40 = 0;
    return m;
}
VehicleGunControlMessage* construct_vehicle_gun_control_0075ff60(VehicleGunControlMessage* m, const Context& c) { return construct_headered(m, &m->h, 0x79, 0, 0x00D032D0u, c); }
UnitMessage* construct_vehicle_unit_launch_007601a0(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x7B, 0x00CE53ACu, c); }
UnitMessage* construct_vehicle_add_launched_00760200(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x7C, 0x00CFAA0Cu, c); }
VehicleValueMessage* construct_vehicle_set_party_00760260(VehicleValueMessage* m, const Context& c) { return construct_headered(m, &m->h, 0x7F, 1, 0x00D032E8u, c); }
VehicleValueMessage* construct_vehicle_set_race_00760390(VehicleValueMessage* m, const Context& c) { return construct_headered(m, &m->h, 0x80, 1, 0x00D032FCu, c); }
VehicleShipyardLaunchMessage* construct_vehicle_shipyard_launch_007604c0(VehicleShipyardLaunchMessage* m, const Context& c) { return construct_unit(m, &m->u, 0x7A, 0x00D03310u, c); }
HitMarkerMessage* construct_hitmarker_00760680(HitMarkerMessage* m, const Context& c) {
    construct_plain(m, &m->base, 0xE5, 0, 0x00D03324u, c);
    m->flag_18 = 0; m->flag_19 = 0;
    return m;
}
HitWarningMessage* construct_hitwarning_007607b0(HitWarningMessage* m, const Context& c) {
    construct_plain(m, &m->base, 0xE6, 0, 0x00D03338u, c);
    m->value_18 = 3;
    return m;
}
RepairAddDamageMessage* construct_shiprepair_adddamage_007608e0(RepairAddDamageMessage* m, const Context& c) { return construct_headered(m, &m->h, 0x9E, 1, 0x00D0334Cu, c); }
RepairBodyMessage* construct_shiprepair_bodyrepair_00760a50(RepairBodyMessage* m, const Context& c) { return construct_headered(m, &m->h, 0x9F, 1, 0x00D03360u, c); }
HeaderedMessage* construct_shiprepair_repairpriority_00760b70(HeaderedMessage* m, const Context& c) { return construct_headered(m, m, 0xA0, 1, 0x00CF5C38u, c); }
LandingShipsLaunchedMessage* construct_ship_landingshipslaunched_00760e90(LandingShipsLaunchedMessage* m, const Context& c) { return construct_headered(m, &m->h, 0x95, 1, 0x00D03388u, c); }
TorpedoGenerateMessage* construct_ship_torpedogenerate_hack_00760fd0(TorpedoGenerateMessage* m, const Context& c) { return construct_headered(m, &m->h, 0x9D, 1, 0x00D0339Cu, c); }
SetTorpedoStockMessage* construct_ship_set_torpedostock_007611d0(SetTorpedoStockMessage* m, const Context& c) { return construct_unit(m, &m->u, 0x96, 0x00D033B0u, c); }
UnitMessage* construct_ship_explodetoparts_007613b0(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x9A, 0x00D033D8u, c); }
ExplodeOnePartMessage* construct_ship_explodeonepart_00761450(ExplodeOnePartMessage* m, const Context& c) { return construct_unit(m, &m->u, 0x99, 0x00D033ECu, c); }
SectionFailureMessage* construct_ship_sectionfailureefx_00761590(SectionFailureMessage* m, const Context& c) { return construct_unit(m, &m->u, 0x9B, 0x00D03400u, c); }
ShipLeakMessage* construct_ship_leak_00761f60(ShipLeakMessage* m, const Context& c) { return construct_headered(m, &m->h, 0x90, 1, 0x00D034A0u, c); }
HeaderedMessage* construct_ship_sink_00762130(HeaderedMessage* m, const Context& c) { return construct_headered(m, m, 0x92, 1, 0x00D00518u, c); }
HeaderedMessage* construct_ship_leakcheatwater_00762190(HeaderedMessage* m, const Context& c) { return construct_headered(m, m, 0x91, 1, 0x00D034B4u, c); }
ShipAvoidSideMessage* construct_ship_avoidside_007623d0(ShipAvoidSideMessage* m, const Context& c) { return construct_headered(m, &m->h, 0x8F, 1, 0x00D034DCu, c); }
PlaneSyncAckMessage* construct_plane_sync_ack_007626e0(PlaneSyncAckMessage* m, const Context& c) {
    construct_plain(m, &m->base, 0xC2, 0, 0x00D034F0u, c);
    m->count_18 = 0;
    return m;
}
PlaneChangeStateMessage* construct_plane_change_state_00762880(PlaneChangeStateMessage* m, const Context& c) { return construct_unit(m, &m->u, 0xC3, 0x00D03504u, c); }
PlaneDisappearMessage* construct_plane_disappear_00762b20(PlaneDisappearMessage* m, const Context& c) { return construct_unit(m, &m->u, 0xC7, 0x00D0352Cu, c); }
PlaneDeadmeatRotMessage* construct_plane_deadmeatrot_00762c80(PlaneDeadmeatRotMessage* m, const Context& c) { return construct_unit(m, &m->u, 0xC8, 0x00D03540u, c); }
UnitMessage* construct_airbase_setslot_00763140(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x81, 0x00CF8624u, c); }
AirbaseLaunchSlotMessage* construct_airbase_launchslot_007631a0(AirbaseLaunchSlotMessage* m, const Context& c) { return construct_unit(m, &m->u, 0x83, 0x00D03554u, c); }
UnitMessage* construct_airbase_changeslotstatus_00763320(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x82, 0x00CF64C4u, c); }
UnitMessage* construct_airbase_slotstatussync_00763380(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x84, 0x00CF8610u, c); }
UnitMessage* construct_airbase_stockitem_007633e0(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x85, 0x00CF8638u, c); }
UnitMessage* construct_airbase_appearreadyplane_00763440(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x86, 0x00CF864Cu, c); }
UnitMessage* construct_airbase_setorders_007634a0(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x88, 0x00CF8660u, c); }
UnitMessage* construct_mothership_elevator_move_00763500(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x87, 0x00D01870u, c); }
UnitMessage* construct_bombplatform_fire_00763600(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0xB1, 0x00CF9B44u, c); }
HeaderedMessage* construct_multibombplatform_generate_00763660(HeaderedMessage* m, const Context& c) { return construct_headered(m, m, 0xB3, 1, 0x00CF9650u, c); }
UnitMessage* construct_catapult_fire_007636c0(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0xB4, 0x00CFAA34u, c); }
ParatrooperLandingPosMessage* construct_paratrooper_landingpos_00763b40(ParatrooperLandingPosMessage* m, const Context& c) { return construct_unit(m, &m->u, 0x71, 0x00D03590u, c); }
UnitMessage* construct_shipyard_change_slot_class_00763cb0(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0xA7, 0x00CF64D8u, c); }
UnitMessage* construct_shipyard_change_slot_equipment_00763d10(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0xA8, 0x00CF64ECu, c); }
UnitMessage* construct_shipyard_launch_slot_00763d70(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0xA9, 0x00CF6500u, c); }
ShipyardSpawnMessage* construct_shipyard_spawn_00763dd0(ShipyardSpawnMessage* m, const Context& c) { return construct_unit(m, &m->u, 0xAA, 0x00D035A4u, c); }
HeaderedMessage* construct_commandbuilding_setowner_00764220(HeaderedMessage* m, const Context& c) { return construct_headered(m, m, 0xD3, 1, 0x00CFADBCu, c); }
HeaderedMessage* construct_commandbuilding_setlevel_00764280(HeaderedMessage* m, const Context& c) { return construct_headered(m, m, 0xD4, 1, 0x00CFADD0u, c); }
HeaderedMessage* construct_commandbuilding_updatecapture_007642e0(HeaderedMessage* m, const Context& c) { return construct_headered(m, m, 0xD5, 1, 0x00CFADE4u, c); }
SpawnBannMessage* construct_highlvlai_spawnbann_00764490(SpawnBannMessage* m, const Context& c) { return construct_plain(m, &m->base, 0xD7, 1, 0x00D035E0u, c); }
UnitMessage* construct_ship_erasewreck_007645b0(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x9C, 0x00D035F4u, c); }
HeaderedMessage* construct_watermine_arm_007648b0(HeaderedMessage* m, const Context& c) { return construct_headered(m, m, 0xB5, 1, 0x00D0361Cu, c); }
CameraMessage* construct_camera_00764960(CameraMessage* m, void* camera, std::uint8_t flag, const Context& c, CameraHooks& hooks) {
    base(&m->base, 0x55, c);
    finish(&m->base, 0, vt(c, 0x00D02F4Cu));
    const auto* cam = static_cast<const B*>(camera);
    if ((cam[0x5c] & 2) == 0) hooks.refresh_world_matrix_00b6db70(camera);
    std::memcpy(m->position_18, cam + 0x120, 12);
    if ((cam[0x5c] & 2) == 0) hooks.refresh_world_matrix_00b6db70(camera);
    std::memcpy(m->direction_24, cam + 0x110, 12);  // native copies through FLD/FSTP
    float fov;
    std::memcpy(&fov, cam + 0x1c4, 4);
    const float product = hooks.fov_aspect_product_00b6fc30(camera);
    m->flag_34 = flag;
    m->fov_30 = product > fov ? product : fov;  // FCOMIP; JBE keeps +1C4h (also unordered)
    return m;
}
ManageMessagesMessage* construct_manage_messages_00765490(ManageMessagesMessage* m, const Context& c) {
    construct_plain(m, &m->base, 0xE2, 1, 0x00D03644u, c);
    m->text_18.length = 0; m->text_18.data = nullptr;
    return m;
}
VoiceOverMessage* construct_voiceover_00765610(VoiceOverMessage* m, const Context& c) {
    construct_plain(m, &m->base, 0xE7, 1, 0x00D03658u, c);
    m->text_18.length = 0; m->text_18.data = nullptr;
    return m;
}
ManageDialogsMessage* construct_manage_dialogs_00765940(ManageDialogsMessage* m, const Context& c) {
    construct_plain(m, &m->base, 0xE1, 1, 0x00CE4B2Cu, c);
    m->zero_18 = 0; m->zero_1c = 0; m->zero_24 = 0; m->zero_28 = 0;
    return m;
}
CountdownMessage* construct_countdown_007663f0(CountdownMessage* m, const Context& c) {
    construct_plain(m, &m->base, 0x2C, 1, 0x00D0370Cu, c);
    m->text_18.length = 0; m->text_18.data = nullptr;
    return m;
}
MetricsSessionMessage* construct_metricssession_007665d0(MetricsSessionMessage* m, const Context& c) {
    construct_plain(m, &m->base, 0xE9, 1, 0x00D01324u, c);
    m->zero_18 = 0; m->zero_1c = 0;
    return m;
}
InferiorListMessage* construct_commandbuilding_inferiorlist_00766660(InferiorListMessage* m, const Context& c) {
    construct_headered(m, &m->h, 0xD6, 1, 0x00CFAE84u, c);
    m->zero_1c = 0; m->zero_20 = 0; m->zero_24 = 0;
    return m;
}
SessionManagerMessage* construct_session_manager_007666c0(SessionManagerMessage* m, const Context& c, void* (*buy_head_node_004c1f50)()) {
    construct_plain(m, &m->base, 0xE4, 1, 0x00CE7608u, c);
    m->list_head_20 = buy_head_node_004c1f50();
    m->list_size_24 = 0;
    return m;
}
VoiceDataMessage* construct_voicedata_007673b0(VoiceDataMessage* m, const Context& c) {
    construct_plain(m, &m->base, 0x29, 2, 0x00D03754u, c);
    std::vector<std::array<std::uint8_t, 10>>().swap(m->payload_18.frames_08);  // native zeroes +24h..+2Ch
    return m;
}

// ---- IsType --------------------------------------------------------------
bool is_control_00759e00(U t) { return is2(t, 0x4A); }
bool is_formation_join_0075a310(U t) { return is3(t, 0x76, 0x49); }
bool is_formation_leave_0075a360(U t) { return is3(t, 0x77, 0x49); }
bool is_ship_helmsman_control_0075a7d0(U t) { return is3(t, 0x8E, 0x4A); }
bool is_plane_sync_0075ab10(U t) { return is3(t, 0xC1, 0x48); }
bool is_plane_controlfactor_0075abb0(U t) { return is3(t, 0xC9, 0x49); }
bool is_plane_hit_0075ac20(U t) { return is3(t, 0xCA, 0x49); }
bool is_submarine_dip_rise_sync_0075ae50(U t) { return is3(t, 0xA2, 0x49); }
bool is_movcamnew_addposition_0075b330(const S* m, U t) { return t == 0x33 || t == m->type_10; }
bool is_entity_relocate_0075bb90(U t) { return is3(t, 0x97, 0x49); }
bool is_musiccontrol_setlevel_0075eab0(U t) { return t == 0x2A; }
bool is_blackout_0075ebc0(U t) { return t == 0x2B; }
bool is_movcam_handmodetargeted_0075ed30(U t) { return t == 0x2D; }
bool is_movcam_refposmodepolar_0075ee70(U t) { return t == 0x2E; }
bool is_movcam_refposmoderadius_0075f080(U t) { return t == 0x2F; }
bool is_movcam_pathwalkproperty_0075f220(U t) { return t == 0x30; }
bool is_movcam_addinterpolator_0075f3d0(U t) { return t == 0x31; }
bool is_movcam_setdeviationfactor_0075f5c0(U t) { return t == 0x32; }
bool is_voicechange_0075fa50(U t) { return t == 0xE8; }
bool is_serverdlcunlock_0075fbf0(U t) { return t == 0xDE; }
bool is_t_gun_turn_sync_0075ff10(U t) { return is3(t, 0xAE, 0x48); }
bool is_vehicle_gun_control_0075ffd0(U t) { return is3(t, 0x79, 0x4A); }
bool is_vehicle_set_party_007602c0(U t) { return is2(t, 0x7F); }
bool is_vehicle_set_race_007603f0(U t) { return is2(t, 0x80); }
bool is_vehicle_shipyard_launch_00760520(U t) { return is3(t, 0x7A, 0x49); }
bool is_hitmarker_007606f0(const S* m, U t) { return t == 0xE5 || t == m->type_10; }
bool is_hitwarning_00760830(const S* m, U t) { return t == 0xE6 || t == m->type_10; }
bool is_shiprepair_adddamage_00760940(U t) { return is2(t, 0x9E); }
bool is_shiprepair_bodyrepair_00760ab0(U t) { return is2(t, 0x9F); }
bool is_ship_guns_sync_00760c80(U t) { return is3(t, 0x8D, 0x48); }
bool is_ship_landingshipslaunched_00760ef0(U t) { return is2(t, 0x95); }
bool is_ship_torpedogenerate_hack_00761030(U t) { return is2(t, 0x9D); }
bool is_ship_set_torpedostock_00761230(U t) { return is3(t, 0x96, 0x49); }
bool is_ship_wrecked_00761370(U t) { return is3(t, 0x98, 0x49); }
bool is_ship_explodetoparts_00761410(U t) { return is3(t, 0x9A, 0x49); }
bool is_ship_explodeonepart_007614b0(U t) { return is3(t, 0x99, 0x49); }
bool is_ship_sectionfailureefx_007615f0(U t) { return is3(t, 0x9B, 0x49); }
bool is_ship_leak_00761fc0(U t) { return is2(t, 0x90); }
bool is_ship_leakcheatwater_007621f0(U t) { return is2(t, 0x91); }
bool is_ship_addtorque_00762280(U t) { return is2(t, 0x93); }
bool is_ship_avoidside_00762430(U t) { return is2(t, 0x8F); }
bool is_plane_sync_ack_00762750(const S* m, U t) { return t == 0xC2 || t == m->type_10; }
bool is_plane_change_state_007628f0(U t) { return is3(t, 0xC3, 0x49); }
bool is_plane_touchdown_00762ad0(U t) { return is3(t, 0xC5, 0x49); }
bool is_plane_disappear_00762b90(U t) { return is3(t, 0xC7, 0x49); }
bool is_plane_deadmeatrot_00762cf0(U t) { return is3(t, 0xC8, 0x49); }
bool is_airbase_launchslot_00763210(U t) { return is3(t, 0x83, 0x49); }
bool is_paratrooper_landingpos_00763ba0(U t) { return is3(t, 0x71, 0x49); }
bool is_highlvlai_spawnbann_00764510(const S* m, U t) { return t == 0xD7 || t == m->type_10; }
bool is_ship_erasewreck_00764610(U t) { return is3(t, 0x9C, 0x49); }
bool is_watermine_arm_00764920(U t) { return is2(t, 0xB5); }
bool is_manage_messages_007654f0(U t) { return t == 0xE2; }
bool is_voiceover_00765670(U t) { return t == 0xE7; }
bool is_countdown_00766450(U t) { return t == 0x2C; }
bool is_voicedata_00767450(U t) { return t == 0x29; }

// ---- Write / Read ----------------------------------------------------------
void write_movcamnew_addposition_0075b350(const MovieAddPositionMessage* m, C* c, MovieKeyframeHooks& hooks) {
    w_type(m->base, c);
    hooks.write_bits_00791360(m->keyframe_18, c);
}
void read_movcamnew_addposition_0075b380(MovieAddPositionMessage* m, R* s, MovieKeyframeHooks& hooks) {
    r_type(m->base, &s->cursor_04);
    void* storage = hooks.operator_new_00bf681b(0x104);
    void* keyframe = storage != nullptr ? hooks.construct_keyframe_00799b00(storage) : nullptr;
    m->keyframe_18 = keyframe;
    hooks.read_bits_007a2370(keyframe, s);  // the whole stream, not stream+4
    m->owns_keyframe_1c = 1;
}
void write_entity_relocate_0075bbd0(const EntityRelocateMessage* m, C* c, const Numeric& n) {
    w_unit(m->u, c);
    w_bool(c, m->has_heading_20); w_bool(c, m->has_position_21);
    if (m->has_heading_20 != 0) w_float(c, m->heading_24, n);
    if (m->has_position_21 != 0) w_vec3(c, m->position_2c, n);
    w_i32(c, m->value_28, 4);
}
void read_entity_relocate_0075bc80(EntityRelocateMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_unit(m->u, c);
    r_bool(c, &m->has_heading_20); r_bool(c, &m->has_position_21);
    if (m->has_heading_20 != 0) r_float(c, &m->heading_24, n);
    if (m->has_position_21 != 0) r_vec3(c, m->position_2c, n);
    r_i32(c, &m->value_28, 4);
}
void write_formation_join_0075c7c0(const FormationMemberMessage* m, C* c) { w_unit(m->u, c); w_word(c, m->member_id_20, 12); }
void read_formation_join_0075c810(FormationMemberMessage* m, R* s) { r_unit(m->u, &s->cursor_04); r_word(&s->cursor_04, &m->member_id_20, 12); }
void write_formation_leave_0075c860(const FormationMemberMessage* m, C* c) { w_unit(m->u, c); w_word(c, m->member_id_20, 12); }
void read_formation_leave_0075c8b0(FormationMemberMessage* m, R* s) { r_unit(m->u, &s->cursor_04); r_word(&s->cursor_04, &m->member_id_20, 12); }
void write_musiccontrol_setlevel_0075eac0(const MusicSetLevelMessage* m, C* c) { w_type(m->base, c); w_u32(c, m->level_18, 4); }
void read_musiccontrol_setlevel_0075eaf0(MusicSetLevelMessage* m, R* s) { r_type(m->base, &s->cursor_04); r_u32(&s->cursor_04, &m->level_18, 4); }
void write_blackout_0075ebd0(const BlackoutMessage* m, C* c, const Numeric& n) { w_type(m->base, c); w_float(c, m->value_18, n); w_float(c, m->value_1c, n); }
void read_blackout_0075ec30(BlackoutMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_type(m->base, c); r_float(c, &m->value_18, n); r_float(c, &m->value_1c, n);
}
void write_movcam_handmodetargeted_0075ed40(const HandModeTargetedMessage* m, C* c) {
    w_type(m->base, c); w_word(c, m->id_18, 12); w_bool(c, m->flag_1a); w_bool(c, m->flag_1b);
}
void read_movcam_handmodetargeted_0075ed90(HandModeTargetedMessage* m, R* s) {
    C* c = &s->cursor_04;
    r_type(m->base, c); r_word(c, &m->id_18, 12); r_bool(c, &m->flag_1a); r_bool(c, &m->flag_1b);
}
void write_movcam_refposmodepolar_0075ee80(const RefPosPolarMessage* m, C* c, const Numeric& n) {
    w_type(m->base, c); w_word(c, m->id_18, 12);
    w_float(c, m->value_1c, n); w_float(c, m->value_20, n); w_float(c, m->value_24, n);
    w_bool(c, m->flag_28); w_float(c, m->value_2c, n);
}
void read_movcam_refposmodepolar_0075ef30(RefPosPolarMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_type(m->base, c); r_word(c, &m->id_18, 12);
    r_float(c, &m->value_1c, n); r_float(c, &m->value_20, n); r_float(c, &m->value_24, n);
    r_bool(c, &m->flag_28); r_float(c, &m->value_2c, n);
}
void write_movcam_refposmoderadius_0075f090(const RefPosRadiusMessage* m, C* c, const Numeric& n) {
    w_type(m->base, c); w_word(c, m->id_18, 12); w_float(c, m->value_1c, n); w_bool(c, m->flag_20); w_float(c, m->value_24, n);
}
void read_movcam_refposmoderadius_0075f110(RefPosRadiusMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_type(m->base, c); r_word(c, &m->id_18, 12); r_float(c, &m->value_1c, n); r_bool(c, &m->flag_20); r_float(c, &m->value_24, n);
}
void write_movcam_pathwalkproperty_0075f230(const PathWalkPropertyMessage* m, C* c, const Numeric& n) {
    w_type(m->base, c); w_vec3(c, m->vector_18, n); w_float(c, m->value_24, n); w_bool(c, m->flag_28); w_float(c, m->value_2c, n);
}
void read_movcam_pathwalkproperty_0075f2b0(PathWalkPropertyMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_type(m->base, c); r_vec3(c, m->vector_18, n); r_float(c, &m->value_24, n); r_bool(c, &m->flag_28); r_float(c, &m->value_2c, n);
}
void write_movcam_addinterpolator_0075f3e0(const AddInterpolatorMessage* m, C* c, const Numeric& n) {
    w_type(m->base, c); w_u32(c, m->kind_18, 8);
    for (U i = 0; i < 4; ++i) w_float(c, m->value_1c[i], n);
}
void read_movcam_addinterpolator_0075f480(AddInterpolatorMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_type(m->base, c); r_u32(c, &m->kind_18, 8);
    for (U i = 0; i < 4; ++i) r_float(c, &m->value_1c[i], n);
}
void write_movcam_setdeviationfactor_0075f5d0(const SetDeviationFactorMessage* m, C* c, const Numeric& n) { w_type(m->base, c); w_float(c, m->factor_18, n); }
void read_movcam_setdeviationfactor_0075f610(SetDeviationFactorMessage* m, R* s, const Numeric& n) { r_type(m->base, &s->cursor_04); r_float(&s->cursor_04, &m->factor_18, n); }
void write_voicechange_0075fa60(const VoiceChangeMessage* m, C* c) {
    w_type(m->base, c);
    write_native_u64_bits_00429180(c, m->qword_20[0], m->qword_20[1]);
    w_bool(c, m->flag_18);
}
void read_voicechange_0075faa0(VoiceChangeMessage* m, R* s) {
    C* c = &s->cursor_04;
    r_type(m->base, c); read_native_u64_bits_00428e90(c, m->qword_20); r_bool(c, &m->flag_18);
}
void write_serverdlcunlock_0075fc00(const ServerDlcUnlockMessage* m, C* c) { w_type(m->base, c); seg1_cc13::write_native_word_array_00429140(c, m->words_18, 0xff, 8); }
void read_serverdlcunlock_0075fc30(ServerDlcUnlockMessage* m, R* s) { r_type(m->base, &s->cursor_04); read_native_word_array_00428e50(&s->cursor_04, m->words_18, 0xff, 8); }
void write_vehicle_gun_control_0075fff0(const VehicleGunControlMessage* m, C* c, const Numeric& n) {
    w_header(m->h, c);
    w_u32(c, m->mode_1c, 3);
    w_vec3(c, m->vector_20, n);
    write_native_numeric_float_004295c0(c, bits(m->angle_2c), 0, 1, kScale_00d032e4, 0x12, n);
    write_native_numeric_float_004295c0(c, bits(m->angle_30), 0, 1, kScale_00d032e4, 0x12, n);
    w_bool(c, m->flag_34); w_bool(c, m->flag_35); w_bool(c, m->has_target_36);
    if (m->has_target_36 != 0) w_word(c, m->target_id_38, 12);
}
void read_vehicle_gun_control_007600d0(VehicleGunControlMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_header(m->h, c);
    r_u32(c, &m->mode_1c, 3);
    r_vec3(c, m->vector_20, n);
    read_native_numeric_float_004293f0(c, fbits(&m->angle_2c), 0, 1, kScale_00d032e4, 0x12, n);
    read_native_numeric_float_004293f0(c, fbits(&m->angle_30), 0, 1, kScale_00d032e4, 0x12, n);
    r_bool(c, &m->flag_34); r_bool(c, &m->flag_35); r_bool(c, &m->has_target_36);
    if (m->has_target_36 != 0) r_word(c, &m->target_id_38, 12);
}
void write_vehicle_set_party_007602f0(const VehicleValueMessage* m, C* c) { w_header(m->h, c); w_u32(c, m->value_1c, 3); }
void read_vehicle_set_party_00760340(VehicleValueMessage* m, R* s) {
    C* c = &s->cursor_04;
    r_header(m->h, c);
    U value;  // native reads into its own stack argument slot, then stores
    r_u32(c, &value, 3);
    m->value_1c = value;
}
void write_vehicle_set_race_00760420(const VehicleValueMessage* m, C* c) { w_header(m->h, c); w_u32(c, m->value_1c, 4); }
void read_vehicle_set_race_00760470(VehicleValueMessage* m, R* s) { r_header(m->h, &s->cursor_04); r_u32(&s->cursor_04, &m->value_1c, 4); }
void write_vehicle_shipyard_launch_00760550(const VehicleShipyardLaunchMessage* m, C* c) { w_unit(m->u, c); w_bool(c, m->flag_20); }
void read_vehicle_shipyard_launch_007605a0(VehicleShipyardLaunchMessage* m, R* s) { r_unit(m->u, &s->cursor_04); r_bool(&s->cursor_04, &m->flag_20); }
void write_hitmarker_00760710(const HitMarkerMessage* m, C* c) { w_type(m->base, c); w_bool(c, m->flag_18); w_bool(c, m->flag_19); }
void read_hitmarker_00760750(HitMarkerMessage* m, R* s) {
    C* c = &s->cursor_04;
    r_type(m->base, c); r_bool(c, &m->flag_18); r_bool(c, &m->flag_19);
}
void write_hitwarning_00760850(const HitWarningMessage* m, C* c) { w_type(m->base, c); w_u32(c, m->value_18, 3); }
void read_hitwarning_00760880(HitWarningMessage* m, R* s) {
    C* c = &s->cursor_04;
    r_type(m->base, c);
    U value;  // stack argument slot, then stored
    r_u32(c, &value, 3);
    m->value_18 = value;
}
void write_shiprepair_adddamage_00760970(const RepairAddDamageMessage* m, C* c, const Numeric& n) {
    w_header(m->h, c); w_i32(c, m->section_24, 3); w_float(c, m->amount_1c, n); w_bool(c, m->flag_20);
}
void read_shiprepair_adddamage_007609e0(RepairAddDamageMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_header(m->h, c); r_i32(c, &m->section_24, 3); r_float(c, &m->amount_1c, n); r_bool(c, &m->flag_20);
}
void write_shiprepair_bodyrepair_00760ae0(const RepairBodyMessage* m, C* c) { w_header(m->h, c); w_bool(c, m->flag_1c); }
void read_shiprepair_bodyrepair_00760b30(RepairBodyMessage* m, R* s) { r_header(m->h, &s->cursor_04); r_bool(&s->cursor_04, &m->flag_1c); }
void write_ship_helmsman_control_00760d00(const ShipHelmsmanControlMessage* m, C* c, const Numeric& n) {
    w_header(m->h, c); w_float(c, m->value_1c, n); w_float(c, m->value_20, n); w_bool(c, m->flag_38);
}
void read_ship_helmsman_control_00760d80(ShipHelmsmanControlMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_header(m->h, c);
    // Each value is read into a stack float and clamped: lo when lo > v, else
    // hi when v > hi, else v (FCOMIP/JBE: an unordered v is kept).
    float value;
    r_float(c, &value, n);
    if (m->min_2c > value) m->value_1c = m->min_2c;
    else if (value > m->max_28) m->value_1c = m->max_28;
    else m->value_1c = value;
    r_float(c, &value, n);
    if (m->min_34 > value) m->value_20 = m->min_34;
    else if (value > m->max_30) m->value_20 = m->max_30;
    else m->value_20 = value;
    B flag;  // stack argument slot, then stored
    r_bool(c, &flag);
    m->flag_38 = flag;
}
void write_ship_landingshipslaunched_00760f20(const LandingShipsLaunchedMessage* m, C* c, const Numeric& n) { w_header(m->h, c); w_float(c, m->value_1c, n); }
void read_ship_landingshipslaunched_00760f80(LandingShipsLaunchedMessage* m, R* s, const Numeric& n) { r_header(m->h, &s->cursor_04); r_float(&s->cursor_04, &m->value_1c, n); }
void write_ship_torpedogenerate_hack_00761060(const TorpedoGenerateMessage* m, C* c, const Numeric& n) {
    w_header(m->h, c);
    w_float(c, m->value_24, n); w_float(c, m->value_28, n); w_float(c, m->value_2c, n); w_float(c, m->value_20, n);
    w_i32(c, m->side_1c, 2);
}
void read_ship_torpedogenerate_hack_00761120(TorpedoGenerateMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_header(m->h, c);
    r_float(c, &m->value_24, n); r_float(c, &m->value_28, n); r_float(c, &m->value_2c, n); r_float(c, &m->value_20, n);
    r_i32(c, &m->side_1c, 2);
}
void write_ship_set_torpedostock_00761270(const SetTorpedoStockMessage* m, C* c) { w_unit(m->u, c); w_u32(c, m->stock_20, 32); }
void read_ship_set_torpedostock_007612c0(SetTorpedoStockMessage* m, R* s) { r_unit(m->u, &s->cursor_04); r_u32(&s->cursor_04, &m->stock_20, 32); }
void write_ship_explodeonepart_007614f0(const ExplodeOnePartMessage* m, C* c) { w_unit(m->u, c); w_u32(c, m->part_20, 5); }
void read_ship_explodeonepart_00761540(ExplodeOnePartMessage* m, R* s) { r_unit(m->u, &s->cursor_04); r_u32(&s->cursor_04, &m->part_20, 5); }
void write_ship_sectionfailureefx_00761630(const SectionFailureMessage* m, C* c) {
    w_unit(m->u, c); w_u32(c, m->section_20, 5); w_u32(c, m->effect_24, 5); w_bool(c, m->flag_28);
}
void read_ship_sectionfailureefx_007616a0(SectionFailureMessage* m, R* s) {
    C* c = &s->cursor_04;
    r_unit(m->u, c); r_u32(c, &m->section_20, 5); r_u32(c, &m->effect_24, 5); r_bool(c, &m->flag_28);
}
void write_ship_leak_00761ff0(const ShipLeakMessage* m, C* c, const Numeric& n) {
    w_header(m->h, c); w_u32(c, m->leak_1c, 6);
    for (U i = 0; i < 3; ++i) w_float(c, m->vector_20[i], n);  // three scalar calls, not the array codec
}
void read_ship_leak_00762090(ShipLeakMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_header(m->h, c); r_u32(c, &m->leak_1c, 6);
    for (U i = 0; i < 3; ++i) r_float(c, &m->vector_20[i], n);
}
void write_ship_addtorque_007622b0(const ShipAddTorqueMessage* m, C* c, const Numeric& n) {
    w_header(m->h, c);
    for (U i = 0; i < 3; ++i) w_float(c, m->torque_1c[i], n);
}
void read_ship_addtorque_00762340(ShipAddTorqueMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_header(m->h, c);
    for (U i = 0; i < 3; ++i) r_float(c, &m->torque_1c[i], n);
}
void write_ship_avoidside_00762460(const ShipAvoidSideMessage* m, C* c) { w_header(m->h, c); w_word(c, m->other_id_1c, 12); w_i32(c, m->side_20, 3); }
void read_ship_avoidside_007624b0(ShipAvoidSideMessage* m, R* s) {
    C* c = &s->cursor_04;
    r_header(m->h, c); r_word(c, &m->other_id_1c, 12); r_i32(c, &m->side_20, 3);
}
void write_plane_sync_ack_00762770(const PlaneSyncAckMessage* m, C* c, const PlaneSyncAckSource& src) {
    w_type(m->base, c);
    // Element count of the vector at 00F87284 (SAR of the byte span), capped at 7Fh.
    auto size_of = [&src](const std::uint32_t* first) {
        return static_cast<U>((reinterpret_cast<std::intptr_t>(src.last_00f8728c) - reinterpret_cast<std::intptr_t>(first)) >> 2);
    };
    const std::uint32_t* first = src.first_00f87288;
    U count = 0;
    if (first != nullptr) {
        count = size_of(first);
        if (count > 0x7f) count = 0x7f;
    }
    w_u32(c, count, 7);
    for (U i = 0; i < count; ++i) {
        first = src.first_00f87288;
        if (first == nullptr || i >= size_of(first)) {
            src.invalid_parameter_noinfo_00bf6713();  // checked-iterator report; execution continues
            first = src.first_00f87288;
        }
        w_u32(c, first[i], 12);
    }
}
void read_plane_sync_ack_00762800(PlaneSyncAckMessage* m, R* s) {
    C* c = &s->cursor_04;
    r_type(m->base, c);
    r_u32(c, &m->count_18, 7);
    for (U i = 0; i < m->count_18; ++i) {
        U id;  // stack argument slot, then stored
        r_u32(c, &id, 12);
        m->ids_1c[i] = id;
    }
}
void write_plane_change_state_00762940(const PlaneChangeStateMessage* m, C* c) {
    w_unit(m->u, c);
    write_native_byte_bits_00428ff0(c, m->state_20, 3); write_native_byte_bits_00428ff0(c, m->state_21, 3);
}
void read_plane_change_state_007629a0(PlaneChangeStateMessage* m, R* s) {
    C* c = &s->cursor_04;
    r_unit(m->u, c);
    read_native_u8_bits_00428c70(c, &m->state_20, 3); read_native_u8_bits_00428c70(c, &m->state_21, 3);
}
void write_plane_disappear_00762be0(const PlaneDisappearMessage* m, C* c) { w_unit(m->u, c); w_word(c, m->id_20, 12); }
void read_plane_disappear_00762c30(PlaneDisappearMessage* m, R* s) { r_unit(m->u, &s->cursor_04); r_word(&s->cursor_04, &m->id_20, 12); }
void write_plane_deadmeatrot_00762d40(const PlaneDeadmeatRotMessage* m, C* c, const Numeric& n) { w_unit(m->u, c); w_vec3(c, m->rotation_20, n); }
void read_plane_deadmeatrot_00762da0(PlaneDeadmeatRotMessage* m, R* s, const Numeric& n) { r_unit(m->u, &s->cursor_04); r_vec3(&s->cursor_04, m->rotation_20, n); }
void write_plane_controlfactor_00762e00(const PlaneControlFactorMessage* m, C* c, const Numeric& n) {
    w_unit(m->u, c);
    // (zero 0, signed 0, scale immediate 1.0f, 8 bits): an unsigned 0..1 byte.
    for (U i = 0; i < 3; ++i) write_native_numeric_float_004295c0(c, bits(m->factor_20[i]), 0, 0, kOneBits, 8, n);
}
void read_plane_controlfactor_00762e90(PlaneControlFactorMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_unit(m->u, c);
    for (U i = 0; i < 3; ++i) read_native_numeric_float_004293f0(c, fbits(&m->factor_20[i]), 0, 0, kOneBits, 8, n);
}
void write_plane_hit_00762f20(const PlaneHitMessage* m, C* c, const Numeric& n) {
    w_unit(m->u, c); w_u32(c, m->kind_20, 3); w_vec3(c, m->point_24, n); w_vec3(c, m->vector_30, n);
}
void read_plane_hit_00762fb0(PlaneHitMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_unit(m->u, c); r_u32(c, &m->kind_20, 3); r_vec3(c, m->point_24, n); r_vec3(c, m->vector_30, n);
}
void write_plane_pcmdlasttime_00763040(const PlanePcmdLastTimeMessage* m, C* c, const Numeric& n) {
    w_unit(m->u, c); w_i32(c, m->command_24, 4); w_float(c, m->time_20, n);
}
void read_plane_pcmdlasttime_007630b0(PlanePcmdLastTimeMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04;
    r_unit(m->u, c); r_i32(c, &m->command_24, 4); r_float(c, &m->time_20, n);
}
void write_airbase_launchslot_00763260(const AirbaseLaunchSlotMessage* m, C* c) { w_unit(m->u, c); w_u32(c, m->slot_20, 6); w_bool(c, m->flag_24); }
void read_airbase_launchslot_007632c0(AirbaseLaunchSlotMessage* m, R* s) {
    C* c = &s->cursor_04;
    r_unit(m->u, c); r_u32(c, &m->slot_20, 6); r_bool(c, &m->flag_24);
}
void write_submarine_dip_rise_sync_00763560(const DipRiseSyncMessage* m, C* c) { w_unit(m->u, c); w_u32(c, m->state_20, 2); }
void read_submarine_dip_rise_sync_007635b0(DipRiseSyncMessage* m, R* s) { r_unit(m->u, &s->cursor_04); r_u32(&s->cursor_04, &m->state_20, 2); }
void write_paratrooper_landingpos_00763bd0(const ParatrooperLandingPosMessage* m, C* c, const Numeric& n) { w_unit(m->u, c); w_vec3(c, m->position_20, n); }
void read_paratrooper_landingpos_00763c30(ParatrooperLandingPosMessage* m, R* s, const Numeric& n) { r_unit(m->u, &s->cursor_04); r_vec3(&s->cursor_04, m->position_20, n); }
void write_shipyard_spawn_00763ee0(const ShipyardSpawnMessage* m, C* c) {
    w_unit(m->u, c); w_u32(c, m->value_20, 2); w_u32(c, m->value_24, 3); w_word(c, m->target_id_28, 12);
}
void read_shipyard_spawn_00763f50(ShipyardSpawnMessage* m, R* s) {
    C* c = &s->cursor_04;
    r_unit(m->u, c); r_u32(c, &m->value_20, 2); r_u32(c, &m->value_24, 3); r_word(c, &m->target_id_28, 12);
}
void write_highlvlai_spawnbann_00764530(const SpawnBannMessage* m, C* c) { w_type(m->base, c); w_bool(c, m->flag_18); }
void read_highlvlai_spawnbann_00764560(SpawnBannMessage* m, R* s) { r_type(m->base, &s->cursor_04); r_bool(&s->cursor_04, &m->flag_18); }
void write_manage_messages_00765500(const ManageMessagesMessage* m, C* c, const char* fallback) {
    w_type(m->base, c);
    write_native_owned_string_00429ac0(c, &m->text_18, fallback);
    w_u32(c, m->value_20, 8); w_u32(c, m->value_24, 2);
}
void read_manage_messages_00765540(ManageMessagesMessage* m, R* s, NativeStringRawPoolContext& pool) {
    C* c = &s->cursor_04;
    r_type(m->base, c);
    read_native_owned_string_00429f20(c, &m->text_18, pool);
    r_u32(c, &m->value_20, 8); r_u32(c, &m->value_24, 2);
}
void write_voiceover_00765680(const VoiceOverMessage* m, C* c, const char* fallback) { w_type(m->base, c); write_native_owned_string_00429ac0(c, &m->text_18, fallback); }
void read_voiceover_007656b0(VoiceOverMessage* m, R* s, NativeStringRawPoolContext& pool) { r_type(m->base, &s->cursor_04); read_native_owned_string_00429f20(&s->cursor_04, &m->text_18, pool); }
void write_countdown_00766460(const CountdownMessage* m, C* c, const char* fallback, const Numeric& n) {
    w_type(m->base, c);
    if (m->stop_28 != 0) {
        w_bool(c, 1);
        return;
    }
    w_bool(c, 0);
    write_native_owned_string_00429ac0(c, &m->text_18, fallback);
    w_float(c, m->value_20, n); w_float(c, m->value_24, n);
}
void read_countdown_007664e0(CountdownMessage* m, R* s, NativeStringRawPoolContext& pool, const Numeric& n) {
    C* c = &s->cursor_04;
    r_type(m->base, c);
    r_bool(c, &m->stop_28);
    if (m->stop_28 == 0) {
        read_native_owned_string_00429f20(c, &m->text_18, pool);
        r_float(c, &m->value_20, n); r_float(c, &m->value_24, n);
    }
}
void write_voicedata_00767460(const VoiceDataMessage* m, C* c) { w_type(m->base, c); seg1_cc13::write_voice_data_payload_00429b00(c, m->payload_18); }
void read_voicedata_00767490(VoiceDataMessage* m, R* s) { r_type(m->base, &s->cursor_04); seg1_cc13::read_voice_data_payload_0042a480(&s->cursor_04, m->payload_18); }

// ---- other ---------------------------------------------------------------
bool shipyard_spawn_target_resolves_00763e70(const ShipyardSpawnMessage* m, const EntityIdTables& t) {
    const std::int32_t id = static_cast<std::int32_t>(m->target_id_28);
    std::uint32_t row;
    if (id < t.split_00f89a10) {  // signed CMP/JGE
        std::memcpy(&row, t.low_rows_00f89a54 + static_cast<U>(id - t.low_base_00f89a0c) * 0x10 + 0xc, 4);
    } else {
        std::memcpy(&row, t.high_rows_00f89aa8 + static_cast<U>(id - t.high_base_00f89a60) * 0x10 + 0xc, 4);
    }
    return row != 0;
}
NativeSessionMessageStorage* delete_ship_sync_00762500(S* m, U flags, const Context& c) {
    m->profile_00 = vt(c, 0x00CE4974u);
    if ((flags & 1u) != 0) singleton_lifetime_free(m);
    return m;
}
void destruct_voiceover_007656e0(VoiceOverMessage* m, const Context& c, NativeStringRawPoolContext& pool) {
    m->base.profile_00 = vt(c, 0x00D03658u);
    destroy_native_string_header_0041dd20(&m->text_18, pool);  // inlined in the native body
    m->base.profile_00 = vt(c, 0x00CE4974u);
}
void destruct_voicedata_007674c0(VoiceDataMessage* m, const Context& c) {
    m->base.profile_00 = vt(c, 0x00D03754u);
    std::vector<std::array<std::uint8_t, 10>>().swap(m->payload_18.frames_08);  // _free(begin); zero three words
    m->base.profile_00 = vt(c, 0x00CE4974u);
}

void destruct_movcamnew_addposition_0075b2d0(MovieAddPositionMessage* m, const Context& c, MovieKeyframeHooks& hooks) {
    m->base.profile_00 = vt(c, 0x00D02ED4u);
    if (m->owns_keyframe_1c != 0 && m->keyframe_18 != nullptr) hooks.scalar_delete_slot0(m->keyframe_18);
    m->base.profile_00 = vt(c, 0x00CE4974u);
}
void destruct_manage_messages_00765590(ManageMessagesMessage* m, const Context& c, NativeStringRawPoolContext& pool) {
    m->base.profile_00 = vt(c, 0x00D03644u);
    destroy_native_string_header_0041dd20(&m->text_18, pool);
    m->base.profile_00 = vt(c, 0x00CE4974u);
}
void destruct_countdown_00766550(CountdownMessage* m, const Context& c, NativeStringRawPoolContext& pool) {
    m->base.profile_00 = vt(c, 0x00D0370Cu);
    destroy_native_string_header_0041dd20(&m->text_18, pool);
    m->base.profile_00 = vt(c, 0x00CE4974u);
}
bool is_ship_guns_sync_category_0075a660(U t) { return is3(t, 0x8C, 0x48); }
bool is_mnettablesyncentity_sync_00768440(U t) { return is3(t, 0xE0, 0x48); }
NativeSessionMessageStorage* construct_movcam_blackbar_0075e4d0(S* m, const Context& c) { return construct_plain(m, m, 0x34, 1, 0x00CF17F8u, c); }
UnitMessage* construct_plane_takeoff_00762a00(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0xC6, 0x00CFAA20u, c); }
UnitMessage* construct_plane_touchdown_00762a60(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0xC5, 0x00D03518u, c); }
HeaderedMessage* construct_ship_addtorque_00762220(HeaderedMessage* m, const Context& c) { return construct_headered(m, m, 0x93, 1, 0x00D034C8u, c); }
UnitMessage* construct_ship_wrecked_00761310(UnitMessage* m, const Context& c) { return construct_unit(m, m, 0x98, 0x00D033C4u, c); }
float* build_transform_0075b500(const void* source, float* out, void (*set_rotation_from_euler_00467050)(float*, float, float, float)) {
    const auto* src = static_cast<const B*>(source);
    auto at = [src](U offset) { float v; std::memcpy(&v, src + offset, 4); return v; };
    const float one = f32(kOneBits_00d7a24c);
    for (U i = 0; i < 16; ++i) out[i] = (i % 5 == 0) ? one : 0.0f;
    set_rotation_from_euler_00467050(out, at(0x30), at(0x34), at(0x38));
    out[12] = at(0x24); out[13] = at(0x28); out[14] = at(0x2c);
    if (src[0x100] != 0) {
        for (U row = 0; row < 3; ++row) {
            const float scale = at(0x104);  // reloaded per row
            out[row * 4 + 0] *= scale; out[row * 4 + 1] *= scale; out[row * 4 + 2] *= scale;
        }
    }
    return out;
}
void apply_server_info_0076a7b0(std::uint8_t* session, const NativeSessionMessage05* message) {
    const auto* msg = reinterpret_cast<const B*>(message);
    session[0x128] = msg[0x18];
    session[0x12a] = msg[0x19];
    B ch;
    U i = 0;
    do {  // inline strcpy from +1Ah to session+100h, NUL included
        ch = msg[0x1a + i];
        session[0x100 + i] = ch;
        ++i;
    } while (ch != 0);
    session[0x169] = msg[0x3b];
    session[0x129] = msg[0x3d];
    session[0x127] = msg[0x3c];
    std::memcpy(session + 0x16c, msg + 0x40, 4);
    session[0x12d] = msg[0x45];
    session[0x12c] = msg[0x44];
    std::memcpy(session + 0x170, msg + 0x48, 4);
    std::memcpy(session + 0x174, msg + 0x4c, 4);
    for (U k = 0; k < 0xe; ++k) std::memcpy(session + 0x130 + k * 4, msg + 0x50 + k * 4, 4);
}

void construct_for_read_net_entity_create_00766f60(NetEntityCreatePrefix* m, const Context& c) {
    for_read(&m->h);
    finish(&m->h.base, 1, vt(c, 0x00D03720u));
    m->pairs_first_4c = nullptr; m->pairs_last_50 = nullptr; m->pairs_end_54 = nullptr;
    m->name_c0 = nullptr;
}
namespace {
// 007668E0 body: base vtable, _free(+C0h) and _free(+4Ch) when non-null (the
// name pointer is left as is), zero the pair vector, root vtable.
void destruct_net_entity_create_body(NetEntityCreatePrefix* m, const Context& c) {
    m->h.base.profile_00 = vt(c, 0x00D03720u);
    if (m->name_c0 != nullptr) singleton_lifetime_free(m->name_c0);
    if (m->pairs_first_4c != nullptr) singleton_lifetime_free(m->pairs_first_4c);
    m->pairs_first_4c = nullptr; m->pairs_last_50 = nullptr; m->pairs_end_54 = nullptr;
    m->h.base.profile_00 = vt(c, 0x00CE4974u);
}
}  // namespace
void destruct_net_entity_create_007668e0(NetEntityCreatePrefix* m, const Context& c) { destruct_net_entity_create_body(m, c); }
void destruct_net_entity_create_00767920(NetEntityCreatePrefix* m, const Context& c) { destruct_net_entity_create_body(m, c); }
void destruct_net_entity_create_00767b40(NetEntityCreatePrefix* m, const Context& c) { destruct_net_entity_create_body(m, c); }
void destruct_net_entity_create_00767c70(NetEntityCreatePrefix* m, const Context& c) { destruct_net_entity_create_body(m, c); }
void destruct_net_entity_create_00767d80(NetEntityCreatePrefix* m, const Context& c) { destruct_net_entity_create_body(m, c); }
PathCreatePrefix* construct_for_read_path_create_007676c0(PathCreatePrefix* m, const Context& c) {
    construct_for_read_net_entity_create_00766f60(&m->base, c);  // EDX keeps this across the call
    m->base.h.base.profile_00 = vt(c, 0x00D0377Cu);
    m->first_114 = nullptr; m->last_118 = nullptr; m->end_11c = nullptr;
    return m;
}
void destruct_path_create_00767740(PathCreatePrefix* m, const Context& c) {
    m->base.h.base.profile_00 = vt(c, 0x00D0377Cu);
    if (m->first_114 != nullptr) singleton_lifetime_free(m->first_114);
    m->first_114 = nullptr; m->last_118 = nullptr; m->end_11c = nullptr;
    destruct_net_entity_create_body(&m->base, c);  // inlined 007668E0 body
}
AirfieldCreatePrefix* construct_for_read_airfield_create_007679d0(AirfieldCreatePrefix* m, const Context& c) {
    construct_for_read_net_entity_create_00766f60(&m->base, c);
    m->base.h.base.profile_00 = vt(c, 0x00CF8EBCu);
    m->sub_vtable_110 = vt(c, 0x00CF8EB8u);
    for (U i = 0; i < 3; ++i) m->zero_11c[i] = 0;
    for (U i = 0; i < 5; ++i) m->zero_12c[i] = 0;
    return m;
}
LandfortCreatePrefix* construct_for_read_landfort_create_00767a70(LandfortCreatePrefix* m, const Context& c) {
    construct_for_read_net_entity_create_00766f60(&m->base, c);
    m->base.h.base.profile_00 = vt(c, 0x00CFF358u);
    m->sub_vtable_110 = vt(c, 0x00CFF354u);
    return m;
}
MNetTableSyncMessage* construct_for_read_mnettablesyncentity_sync_007683c0(MNetTableSyncMessage* m, const Context& c, MNetTableSyncHooks& hooks) {
    for_read(&m->h);
    finish(&m->h.base, 0, vt(c, 0x00D03824u));
    m->head_20 = hooks.allocate_head_node_006edea0();
    m->head_20[0x31] = 1;  // isnil
    std::uint8_t* head = m->head_20;
    std::memcpy(head + 4, &head, 4);   // parent
    std::memcpy(head + 0, &head, 4);   // left
    std::memcpy(head + 8, &head, 4);   // right
    m->size_24 = 0;
    m->text_28.length = 0; m->text_28.data = nullptr;
    return m;
}
void destruct_mnettablesyncentity_sync_00768470(MNetTableSyncMessage* m, const Context& c, MNetTableSyncHooks& hooks, NativeStringRawPoolContext& pool) {
    m->h.base.profile_00 = vt(c, 0x00D03824u);
    destroy_native_string_header_0041dd20(&m->text_28, pool);  // inlined
    std::uint8_t out_iterator[8];
    void* leftmost;
    std::memcpy(&leftmost, m->head_20, 4);
    hooks.erase_range_006ee960(m->map_1c, out_iterator, m->map_1c, leftmost, m->map_1c, m->head_20);
    singleton_lifetime_free(m->head_20);
    m->head_20 = nullptr; m->size_24 = 0;
    m->h.base.profile_00 = vt(c, 0x00CE4974u);
}

void write_plane_create_00767810(const PlaneCreateMessage* m, C* c, const Numeric& n, CreateWriteHooks& hooks) {
    hooks.net_entity_create_write_00765000(m, c);
    w_word(c, m->id_110, 12);
    w_float(c, m->value_114, n); w_float(c, m->value_118, n); w_float(c, m->value_11c, n);
    w_i32(c, m->value_120, 4); w_i32(c, m->value_124, 4);
    write_native_byte_bits_00428ff0(c, m->byte_128, 4);
    write_native_byte_bits_00428ff0(c, m->byte_129, 3);
    w_bool(c, m->flag_12a);
    if (m->optional_12c < 0) {
        w_bool(c, 0);
        return;
    }
    w_bool(c, 1);
    w_u32(c, static_cast<U>(m->optional_12c), 8);
}
void write_torpedo_create_00767ad0(const BombDerivedCreateMessage* m, C* c, const Numeric& n, CreateWriteHooks& hooks) {
    hooks.bomb_create_write_006e0690(m, c);
    w_float(c, m->value_120, n);
    write_native_numeric_float_004295c0(c, bits(m->value_124), 0, 1, kScale_00ce3d08, 12, n);
    w_word(c, m->id_128, 12);
}
void write_paratrooper_create_00767c30(const ParatrooperCreateMessage* m, C* c, const Numeric& n, CreateWriteHooks& hooks) {
    hooks.bomb_create_write_006e0690(m, c);
    w_vec3(c, m->position_120, n);
}

void read_plane_create_007681f0(PlaneCreateMessage* m, R* s, const Numeric& n, CreateReadHooks& hooks) {
    hooks.net_entity_create_read_00767e30(m, s);
    C* c = &s->cursor_04;
    r_word(c, &m->id_110, 12);
    r_float(c, &m->value_114, n); r_float(c, &m->value_118, n); r_float(c, &m->value_11c, n);
    r_i32(c, &m->value_120, 4); r_i32(c, &m->value_124, 4);
    read_native_u8_bits_00428c70(c, &m->byte_128, 4);
    read_native_u8_bits_00428c70(c, &m->byte_129, 3);
    r_bool(c, &m->flag_12a);
    B present; r_bool(c, &present);
    if (present) { U value; r_u32(c, &value, 8); m->optional_12c = static_cast<std::int32_t>(value); }
    else m->optional_12c = -1;
}
void read_torpedo_create_00768300(BombDerivedCreateMessage* m, R* s, const Numeric& n, CreateReadHooks& hooks) {
    hooks.bomb_create_read_006e06f0(m, s);
    C* c = &s->cursor_04;
    r_float(c, &m->value_120, n);
    read_native_numeric_float_004293f0(c, fbits(&m->value_124), 0, 1, kScale_00ce3d08, 12, n);
    r_word(c, &m->id_128, 12);
}
void read_paratrooper_create_00768370(ParatrooperCreateMessage* m, R* s, const Numeric& n, CreateReadHooks& hooks) {
    hooks.bomb_create_read_006e06f0(m, s);
    r_vec3(&s->cursor_04, m->position_120, n);
}
void read_net_entity_create_thunk_007683b0(void* m, R* s, CreateReadHooks& hooks) {
    hooks.net_entity_create_read_00767e30(m, s);
}
void read_path_create_points_007b0c90(PathCreatePrefix* m, R* s, const Numeric& n, CreateReadHooks& hooks) {
    hooks.net_entity_create_read_00767e30(m, s);
    // The native checked-vector guard repeats a just-established bound.
    // Caller supplies a valid 12-byte-stride [first,last) range.
    for (std::uint32_t i = 0; m->first_114 && i < static_cast<std::uint32_t>(
        (reinterpret_cast<std::intptr_t>(m->last_118) - reinterpret_cast<std::intptr_t>(m->first_114)) / 12); ++i) {
        auto* point = reinterpret_cast<float*>(static_cast<std::uint8_t*>(m->first_114) + i * 12);
        r_vec3(&s->cursor_04, point, n);
    }
}
void broadcast_activate_if_clear_0077d500(void* owner, bool flag, const Context& c,
    void (*broadcast)(void*, const ActivateMessage*, int, void*), void* context) {
    if (flag) return;
    ActivateMessage m;
    NativeSessionMessageContext base_context{c.current_game_00e188a8};
    construct_native_session_message_0075b430(&m.u.h.base, 0x52, base_context);
    m.u.h.base.delivery_04 = 1;
    m.u.h.id_18 = 0; m.u.h.relay_1a = 0; m.u.flag_1c = 0;
    m.u.h.base.profile_00 = vt(c, 0x00d02d58);
    m.active_20 = 1;
    broadcast(owner, &m, 0, context);
}
bool is_type_cc_49_46_0075ac90(U type) noexcept { return type == 0xcc || type == 0x49 || type == 0x46; }
bool is_type_aa_49_46_00763e40(U type) noexcept { return type == 0xaa || type == 0x49 || type == 0x46; }
void write_path_set_007ad820(const PathIndexedVectorMessage* m, C* c, const Numeric& n) {
    w_unit(m->u, c); w_u32(c, m->index_20, 32); w_vec3(c, m->point_24, n);
}
void read_path_set_007ad870(PathIndexedVectorMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04; r_unit(m->u, c); r_u32(c, &m->index_20, 32); r_vec3(c, m->point_24, n);
}
void write_path_add_007ad8c0(const PathVectorMessage* m, C* c, const Numeric& n) {
    w_unit(m->u, c); w_vec3(c, m->point_20, n);
}
void read_path_add_007ad900(PathVectorMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04; r_unit(m->u, c); r_vec3(c, m->point_20, n);
}
void write_path_insert_007ad940(const PathIndexedVectorMessage* m, C* c, const Numeric& n) {
    w_unit(m->u, c); w_u32(c, m->index_20, 32); w_vec3(c, m->point_24, n);
}
void read_path_insert_007ad990(PathIndexedVectorMessage* m, R* s, const Numeric& n) {
    C* c = &s->cursor_04; r_unit(m->u, c); r_u32(c, &m->index_20, 32); r_vec3(c, m->point_24, n);
}
void write_path_delete_007ad9e0(const PathIndexMessage* m, C* c) { w_unit(m->u, c); w_u32(c, m->index_20, 32); }
void read_path_delete_007ada10(PathIndexMessage* m, R* s) { C* c = &s->cursor_04; r_unit(m->u, c); r_u32(c, &m->index_20, 32); }

}  // namespace bsp::seg42
