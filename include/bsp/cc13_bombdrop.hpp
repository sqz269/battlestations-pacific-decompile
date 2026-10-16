// 006E4D50 BSP_MBombPlatform_Drop, full-body reconstruction (packet cc13-bombdrop).
//
// The rack ("bomb platform") drops its first child round: optional launch effect, the cone and
// drift draws, the round's launch through its mover, the rack's copy of the round's pose, the
// paratrooper landing scatter, the drop's owner stamp and registration, the rack's follow-up slot,
// then the ammo-type warning and the per-slot drop-interval record. docs/CC13_BOMBDROP.md carries
// the evidence, the draw list and the callee list.
//
// Model as in cc13_seg33a: the routine works on the original objects as raw bytes (Win32 layout,
// offsets as in the image) and reaches the engine through BombDropHost, whose methods name the
// image routine, global or virtual slot they stand for, invoked at the binary's point with its
// arguments. A new C++ interface, not a drop-in binary replacement; nothing here is bound into
// bsp_game. Names are hypotheses, not recovered symbols. Marker: cc13_bombdrop.

#ifndef BSP_CC13_BOMBDROP_HPP
#define BSP_CC13_BOMBDROP_HPP

#include <cstdint>

#include "bsp/cc13_seg33a.hpp"

namespace bsp::cc13_bombdrop {

inline constexpr float kPi_00d7a264 = 3.14159274f;                // 00D7A264, float 40490FDBh
inline constexpr double kPointThree_00ce3dc8 = 0.30000001192092896; // 00CE3DC8, double 3FD3333340000000h
inline constexpr double kHalf_00d7a280 = 0.5;                      // 00D7A280, double
inline constexpr float kParaScatterLo_00ceca0c = -50.0f;           // 00CECA0C, float
inline constexpr float kParaScatterHi_00ceb4d4 = 50.0f;            // 00CEB4D4, float

// The six shared-RNG draws 006E4D50 can take, in call order. Each is one 00BD2F10 call.
enum class DrawSite : std::uint8_t {
    kConeAngle_006e4f91,    // stream 1, U(0, pi)          -> rack+400h
    kConeSpread_006e4fb1,   // stream 1, U(0, throw)       -> tan -> rack+404h
    kDriftSide_006e513c,    // stream 1, U(-w, w)          scales the side axis
    kDriftAlong_006e51ac,   // stream 1, U(-0.5w, 0.3w)    along the owner velocity
    kParaX_006e5479,        // stream 0, U(-50, 50)        landing x scatter
    kParaZ_006e549e,        // stream 0, U(-50, 50)        landing z scatter
};

struct BombDropHost {
    virtual ~BombDropHost() = default;

    // Neighbour host for the direct call of cc13_seg33a::bomb_projectile_set_visible_006e0b40.
    virtual cc13_seg33a::Seg33aHost& seg33a_host() = 0;

    // 00BD2F10 BSP_Random_UniformFloatRange, ECX = stream, (lo, hi) RET 8, ST0. The injectable
    // draw: every draw of this routine goes through it, tagged with its site.
    virtual float random_uniform_float_range_00bd2f10(std::int32_t stream_ecx, float lo, float hi,
                                                      DrawSite site) = 0;

    // Globals.
    virtual float game_time_00f876a4() = 0;              // float [00F876A4]
    virtual void* game_00e188a8() = 0;                   // [00E188A8]
    virtual void* local_player_entity_00e188d8() = 0;    // [00E188D8]
    virtual void* warning_manager_00f8a0c4() = 0;        // [00F8A0C4]

    // Image routines.
    virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* value) = 0;   // IAT
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) = 0;   // IAT
    // 008687C0: ECX = out handle, EDX = parent, stack (consumed definition, matrix*, a, b, c), RET 14h.
    virtual void point_effect_create_with_parent_matrix_008687c0(void** out, void* parent,
                                                                 void* consumed_definition,
                                                                 const float* matrix, std::int32_t a,
                                                                 std::int32_t b, std::int32_t c) = 0;
    virtual bool party_slot_is_ai_held_00927f10(std::int32_t slot) = 0;          // __stdcall RET 4
    virtual float pilot_bot_throw_mul_00999b70(void* bot, void* record) = 0;     // __thiscall RET 4, ST0
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;           // __thiscall
    virtual void paratrooper_set_landing_pos_007ab230(void* paratrooper, const float* pos) = 0;  // RET 4
    virtual void owner_register_projectile_009555a0(void* owner, void* mover) = 0;              // RET 4
    virtual void* entity_ancestor_00923810(void* entity, std::int32_t levels) = 0;             // RET 4
    virtual void warning_dispatch_ammo_type_00984ba0(void* manager, void* squadron) = 0;      // RET 4
    // 0090E670: __thiscall(table = [game+21A0h], slot <= 7, entity, float seconds) RET 0Ch.
    virtual void slot_drop_interval_record_0090e670(void* table, std::int32_t slot, void* entity,
                                                    float seconds) = 0;

    // Virtual slots.
    virtual bool is_kind_vslot_5c(void* object, std::int32_t kind) = 0;          // vtable+5Ch, RET 4
    virtual void owner_vslot_200(void* owner, std::int32_t a, std::int32_t b) = 0;   // vtable+200h
    virtual const float* owner_world_velocity_vslot_34(void* owner, float* out) = 0; // vtable+34h
    virtual void* round_mover_vslot_108(void* round) = 0;                         // vtable+108h
    // vtable+34h of the mover: 006E1F00 for MBomb/MTorpedo (cone direction, drift).
    virtual void mover_launch_vslot_34(void* mover, const float* direction, float* drift) = 0;
    // vtable+224h of the rack: 006E3E70 (single rack), 006E5AB0 (multiple rack).
    virtual void rack_vslot_224(void* rack) = 0;
    virtual void effect_handle_destroy_vslot_0(void* handle) = 0;                 // vtable+0
};

// 006E4D50, __thiscall(rack, char use_given, float angle, float spread), RET 0Ch.
void bomb_platform_drop_006e4d50(BombDropHost& host, void* rack, std::uint8_t use_given, float angle,
                                 float spread);

}  // namespace bsp::cc13_bombdrop

#endif
