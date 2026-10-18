#pragma once

#include <cstdint>

// Segment 41 tail rows left by oct18-tA (packet cc15-rC1). New C++ interfaces;
// names are hypotheses, not recovered symbols. docs/CC15_SEG41_TAIL.md records
// the original ABIs, evidence and limits. Raw object pointers need the native
// Win32 layouts. No host is wired to any of these.
namespace bsp::cc15_seg41_tail {

// ---------------------------------------------------------------------------
// 00748A40 BSP_StationaryProp_Construct
// ---------------------------------------------------------------------------
// __fastcall(ECX = 1ACh-byte prop memory) -> EAX = same pointer, RET, FS:[0]
// frame with handler 00C873C6 (EH state 0 after the base, 1 after +184h).
// Callers 00748C40 (at 00748C88) and 00851500 (at 00851548) allocate and
// zero the memory first.
struct StationaryPropConstructHost {
    virtual ~StationaryPropConstructHost() = default;
    // 00925CE0 BSP_SceneNode_Construct, __fastcall(ECX = prop), RET.
    virtual void scene_node_construct_00925ce0(std::uint8_t* prop) = 0;
    // 00926BE0 BSP_SEntity_PushPending, __thiscall(ECX = prop), RET.
    virtual void sentity_push_pending_00926be0(std::uint8_t* prop) = 0;
    // Begin/end words of the prop vector at 00F8755C (+4h = 00F87560,
    // +8h = 00F87564), read after 00926BE0 returns.
    virtual std::uintptr_t prop_vector_begin_00f87560() = 0;
    virtual std::uintptr_t prop_vector_end_00f87564() = 0;
    // 00748990 STL_Vector_PushBack_00748990, __thiscall(ECX = 00F8755C,
    // stack = address of a stack word holding the prop), RET 4.
    virtual void prop_vector_push_back_00748990(std::uint8_t* const* value) = 0;
};

// Vtable words 00748A40 stores immediately after the base constructor.
inline constexpr std::uint32_t kStationaryPropVtable00 = 0x00CFF678u;
inline constexpr std::uint32_t kStationaryPropVtable10 = 0x00CFF65Cu;
inline constexpr std::uint32_t kStationaryPropVtable24 = 0x00CFF654u;
// The word stored at +C4h (00748A91).
inline constexpr std::uint32_t kStationaryPropKindC4 = 0x36u;

std::uint8_t* stationary_prop_construct_00748a40(std::uint8_t* prop,
                                                StationaryPropConstructHost& host);

// ---------------------------------------------------------------------------
// Five-byte JMP rel32 thunks (disk bytes E9 rel32 then CC padding). Each
// leaves ECX, the stack and the return address untouched, so the target's
// convention and RET width are the thunk's. The existing target bodies are
// supplied through the host; nothing is duplicated.
// ---------------------------------------------------------------------------
struct TailThunkHost {
    virtual ~TailThunkHost() = default;
    // 00765000 BSP_NetEntityCreateMessage_Write, __thiscall(message, cursor),
    // RET 4. Source: write_entity_create_00765000, src/net_entity_create_cc13.cpp.
    virtual void net_entity_create_write_00765000(void* message, void* cursor) = 0;
    // 00928860 BSP_GameEntity_PlaceInWorld, __thiscall(entity, hierarchy
    // parent, world node, matrix), RET 0Ch. Source:
    // game_entity_place_in_world_00928860, src/entity_mission_seg60_argon.cpp.
    virtual void game_entity_place_in_world_00928860(void* entity, void* hierarchy_parent,
                                                     void* world_node, const void* matrix) = 0;
    // 00828F20 BSP_ShipClass_DeriveTurnFields, bool __thiscall(descriptor),
    // RET. Source: ship_class_ai_derived_motion_00828f20, src/ship_ai_throttle_ring.cpp.
    virtual bool ship_class_derive_turn_fields_00828f20(void* descriptor) = 0;
    // 00827A90 BSP_ShipInstance_OnHealthChanged, __fastcall(ship), RET.
    // Source: ship_on_health_changed_00827a90, src/unit_damage.cpp.
    virtual void ship_on_health_changed_00827a90(void* ship) = 0;
};

// 0074CCA0: JMP 00765000. Vtable word 00CFFD1C.
void net_entity_create_write_thunk_0074cca0(void* message, void* cursor, TailThunkHost& host);
// 0074E180: JMP 00928860. Vtable words 00D00108, 00D003A0, 00D05FB8, 00D066D0,
// 00D069B8, 00D0BB18, 00D19DC0, 00D1A098, 00D1A370.
void game_entity_place_in_world_thunk_0074e180(void* entity, void* hierarchy_parent,
                                               void* world_node, const void* matrix,
                                               TailThunkHost& host);
// 00758140: JMP 00828F20. Vtable word 00D1AED0; AL is the target's result.
bool ship_class_derive_turn_fields_thunk_00758140(void* descriptor, TailThunkHost& host);
// 00758390: JMP 00827A90. Vtable word 00D017E0.
void ship_on_health_changed_thunk_00758390(void* ship, TailThunkHost& host);

} // namespace bsp::cc15_seg41_tail
