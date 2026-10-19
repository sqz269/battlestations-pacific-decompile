// cc13 segment 52 leftovers (packet cc13-seg52U, cc9 gameplay terms approved 2026-10-16):
// the four game bodies left in 008288D0-00851CB0 after the cc13_seg52 / cc13_seg52b /
// cc13_big53 passes, outside the struck cc9 ranges 00836000-00837FFF and 0084E000-0084EFFF.
// docs/CC13_SEG52U.md carries the evidence and the adoption notes against cc9's host.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in
// the image) and reaches the engine through Seg52UHost, whose methods name the image routine or
// virtual slot they stand for and give the register inputs. Constants carry their image address
// and width. New C++ interfaces, not drop-in binary replacements; nothing here is bound into
// bsp_game. Names are hypotheses, not recovered symbols.

#ifndef BSP_CC13_SEG52U_HPP
#define BSP_CC13_SEG52U_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg52U {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
inline void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }
inline void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

// ---------------------------------------------------------------------------
// Image constants.
// ---------------------------------------------------------------------------
// Command class descriptors compared by address (the `order` words at block+54h / +188h).
inline constexpr std::uint32_t kOrderMoveTo = 0x00E08F68;    // 0084DDFD
inline constexpr std::uint32_t kOrderStop = 0x00E08F88;      // 0084DE88
inline constexpr std::uint32_t kOrderRetreat = 0x00E08F90;   // 0084DF53
inline constexpr std::uint32_t kOrderLand = 0x00E08FA0;      // 0084DF4B
// The default command position, three floats in .bss (zero at load).
inline constexpr std::uint32_t kDefaultCommandPosition = 0x00F87574;  // 0084DE8F..0084DEA9
inline constexpr double kLevelFlightFraction = 0.5;          // double 00D7A280, 0084DE76
inline constexpr float kAltitudeJitterHigh = 100.0f;         // float 00CE3D08, 0084DEF3
inline constexpr std::uint32_t kRandomStream = 1;            // ECX for 00BD2F10, 0084DF00
inline constexpr std::int32_t kSessionModeNoResume = 2;      // game+1FE4h, 0084DF3E
inline constexpr std::int32_t kMsgResumeAiControl = 0x57;    // 0084DF93
inline constexpr std::uint32_t kMsgResumeAiControlVtable = 0x00CECCB4;  // 0084DFB3
inline constexpr std::uint32_t kMsgRootVtable = 0x00CE4974;             // 0084DFDF
inline constexpr std::int32_t kPlaneStateOnWater = 6;        // leader+900h, 0084DE55

// 00846A80: the shipyard's eight vtable words, in store order.
inline constexpr std::uint32_t kShipyardVtables[8][2] = {
    {0x000, 0x00D0B770}, {0x010, 0x00D0B754}, {0x024, 0x00D0B74C}, {0x170, 0x00D0B748},
    {0x1E4, 0x00D0B740}, {0x310, 0x00D0B728}, {0x38C, 0x00D0B724}, {0x72C, 0x00D0B718},
};

struct Seg52UHost {
    virtual ~Seg52UHost() = default;

    // The dword / float stored at image global `address`.
    virtual void* global_ptr(std::uint32_t address) = 0;
    virtual float global_float(std::uint32_t address) = 0;
    // 00BF6713, the checked-iterator invalid-parameter report.
    virtual void invalid_parameter_00bf6713() = 0;

    // --- 0084DDA0 ---
    // 0071F600 BSP_WeaponDirector_BeginCommandBase, __thiscall(ECX = block; dword flag),
    // RET 4 -> AL.
    virtual bool weapon_director_begin_command_base_0071f600(void* block, std::uint32_t flag) = 0;
    // 00521EA0 BSP_CommandTarget_ResolveObject, __fastcall(ECX = target record) -> EAX.
    virtual void* command_target_resolve_object_00521ea0(void* target) = 0;
    // [plane]->vtable[38h](ECX = plane) -> ST0, the current speed.
    virtual double plane_speed_vslot_38(void* plane) = 0;
    // 007C47F0 BSP_PlaneClass_LevelFlightSpeed, __thiscall(ECX = plane class) -> ST0.
    virtual double plane_class_level_flight_speed_007c47f0(void* plane_class) = 0;
    // 00427EB0 BSP_EntityPose_GetWorldPositionRefreshed, __fastcall(ECX = entity) -> EAX.
    virtual void* entity_world_position_00427eb0(void* entity) = 0;
    // 007EF8F0 BSP_World_ClampPointToMapBounds, __thiscall(ECX = [00E188A8]; float* out,
    // const float* point), RET 8.
    virtual void world_clamp_point_to_map_bounds_007ef8f0(void* game, float* out,
                                                          const void* point) = 0;
    // 0042E740 BSP_GameTuning_GetSingleton() -> EAX.
    virtual void* game_tuning_singleton_0042e740() = 0;
    // 00BD2F10 BSP_Random_UniformFloatRange, __thiscall(ECX = stream; float lo, float hi),
    // RET 8 -> ST0. The process-wide generator.
    virtual double random_uniform_float_range_00bd2f10(std::uint32_t stream, float lo,
                                                        float hi) = 0;
    // 00467C80 BSP_ScenePropertyValue_ConstructVector3, __thiscall(ECX = value; const float*
    // vec, float w), RET 8.
    virtual void scene_property_value_construct_vector3_00467c80(void* value, const float* vec,
                                                                 float w) = 0;
    // 0077C470, __thiscall(ECX = plane; 2, 0), RET 8.
    virtual void plane_session_prepare_0077c470(void* plane, std::int32_t a, std::int32_t b) = 0;
    // 0075B430 BSP_SessionMessage_ConstructBase, __thiscall(ECX = message; int type), RET 4.
    virtual void session_message_construct_base_0075b430(void* message, std::int32_t type) = 0;
    // 0077C2A0 BSP_Session_RouteMessage, __thiscall(ECX = plane; message*, 0, 0), RET 0Ch.
    virtual void session_route_message_0077c2a0(void* plane, void* message, std::int32_t a,
                                                std::int32_t b) = 0;
    // [block]->vtable[40h](ECX = block; 0), RET 4.
    virtual void block_vslot_40(void* block, std::int32_t arg) = 0;

    // --- 008390C0 / 00839160 ---
    // [cls]->vtable[18h] / [cls]->vtable[5Ch](ECX = cls; int kind), RET 4 -> AL.
    virtual bool class_accepts_vslot_18(void* cls, std::int32_t kind) = 0;
    virtual bool class_accepts_vslot_5c(void* cls, std::int32_t kind) = 0;

    // --- 00846A80 ---
    // 008468F0 / 008468A0 / 00846850, the slot / hangar / stock vector tidies,
    // __fastcall(ECX = vector).
    virtual void shipyard_slots_tidy_008468f0(void* vector) = 0;
    virtual void shipyard_hangars_tidy_008468a0(void* vector) = 0;
    virtual void shipyard_stock_tidy_00846850(void* vector) = 0;
    // 00959940 BSP_UnitGameObject_Destruct, __fastcall(ECX = object).
    virtual void unit_game_object_destruct_00959940(void* object) = 0;
};

// 0084DDA0, slot +78h of the plane squadron command block vtable 00D0BD98:
// __thiscall(ECX = block; byte flag), RET 4 -> AL.
bool squadron_block_begin_command_0084dda0(void* block, std::uint32_t flag, Seg52UHost& host);

// 008390C0 / 00839160, unreferenced kind-value lookups over the {kind, float} vector at
// owner+1DCh: __thiscall(ECX = owner; class), RET 4 -> ST0.
float kind_value_lookup_008390c0(void* owner, void* cls, Seg52UHost& host);
float kind_value_lookup_00839160(void* owner, void* cls, Seg52UHost& host);

// 00846A80 BSP_Shipyard_Destruct, __fastcall(ECX = shipyard), plain RET.
void shipyard_destruct_00846a80(void* shipyard, Seg52UHost& host);

}  // namespace bsp::cc13_seg52U

#endif  // BSP_CC13_SEG52U_HPP
