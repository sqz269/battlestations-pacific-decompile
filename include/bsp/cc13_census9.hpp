// Census of segments 30/41/42/46/50/51/54/58/61, packet cc13-census9 (marker cc13_census9).
//
// After the census (docs/CC13_CENSUS9.md) the nine segments held no untagged game routines;
// these seven bodies are the library-tagged rows that turned out to be game code (or a game
// destructor the compiler kept out of line). Everything else the census flagged is a
// compiler wrapper around an already reconstructed body and is bound by ledger record only.
//
// Model: each routine works on the original object as raw bytes (Win32 layout, image
// offsets); image vtable addresses are stored as their 32-bit image values, and every callee
// is reached through Census9Host, whose methods name the image routine they stand for. A new
// C++ interface, not a drop-in binary replacement; nothing here is wired into bsp_game. Names
// are hypotheses, not recovered symbols. SEH state stores are not modelled.

#ifndef BSP_CC13_CENSUS9_HPP
#define BSP_CC13_CENSUS9_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_census9 {

// MT_SHIP_GUNS_SYNC (8Dh, 1AB4h bytes; factory row in include/bsp/oct18_tail_a_factory.inc).
inline constexpr std::uint32_t kShipGunsSyncType = 0x8D;
inline constexpr std::uint32_t kShipGunsSyncVtable = 0x00D03374;
inline constexpr std::uint32_t kShipGunsSyncBaseVtable = 0x00CE4974;   // stored by 00760CB0
inline constexpr std::size_t kShipGunsSyncRecords = 0x24;               // 64h records of 44h
inline constexpr std::uint32_t kShipGunsSyncRecordSize = 0x44;
inline constexpr std::uint32_t kShipGunsSyncRecordCount = 0x64;

// Unit motion controller (unit+1018h, 390h bytes from 0080DEC0).
inline constexpr std::uint32_t kControllerVtable = 0x00D19630;
inline constexpr std::uint32_t kControllerListenerVtable = 0x00D1961C;   // subobject at +20h
inline constexpr std::uint32_t kControllerListenerMask = 0x7FF9;
inline constexpr std::size_t kControllerSlots = 0x8C;                   // 14h slots of 20h
inline constexpr std::uint32_t kControllerSlotSize = 0x20;
inline constexpr std::uint32_t kControllerSlotCount = 0x14;
// The process-wide zero vector (in .data's uninitialised tail; every xref reads it, see
// docs/AI_COMMAND_TICK.md). Read through the host rather than assumed zero.
inline constexpr std::uint32_t kControllerVec3Global = 0x00F87574;

// Sound request queue singleton (published at 00F89B34, getter 004C1B90).
inline constexpr std::uint32_t kSoundQueueVtable = 0x00D198F8;
inline constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818;

// Air-ops holder (scalar deleting destructor 006C23D0).
inline constexpr std::uint32_t kAirOpsHolderVtable = 0x00CF86A4;

class Census9Host {
public:
    virtual ~Census9Host() = default;
    // 0075B430, __thiscall(ECX = message, type), RET 4: the session-message base constructor.
    virtual void session_message_construct_base_0075b430(void* message, std::uint32_t type) = 0;
    // 00BF7CD1 eh_vector_constructor_iterator(ptr, size, count, ctor, dtor).
    virtual void eh_vector_construct_00bf7cd1(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t ctor, std::uint32_t dtor) = 0;
    // 00BF7C6E eh_vector_destructor_iterator(ptr, size, count, dtor).
    virtual void eh_vector_destruct_00bf7c6e(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t dtor) = 0;
    // 00BF65AC _free.
    virtual void free_00bf65ac(void* block) = 0;
    // [00F87574 + 4*i], i = 0..2, read at 00939DC0..00939E10.
    virtual float controller_default_vec3_00f87574(int i) = 0;
    // 00937C90, __thiscall(ECX = controller): BSP_UnitController_BuildSubObjectsAndHullBody.
    virtual void controller_build_sub_objects_00937c90(void* controller) = 0;
    // 009316F0 / 00931760: the element-range destroy of a std::vector (ECX = first, EDX = last,
    // stack (vector, owner)); STL instantiations.
    virtual void vector_destroy_range_009316f0(void* vector, void* owner, void* first, void* last) = 0;
    virtual void vector_destroy_range_00931760(void* vector, void* owner, void* first, void* last) = 0;
    // [00F89B34] = 0 (the sound request queue publication), 00940E81.
    virtual void clear_sound_queue_publication_00f89b34() = 0;
    // 006BF880, __thiscall(ECX = 00E19948 holder list, holder): BSP_AirOpsHolderList_Unlink.
    virtual void air_ops_holder_list_unlink_006bf880(void* holder) = 0;
    // The tail-JMP targets of the out-of-line destructors below, each __thiscall(ECX = object).
    virtual void net_entity_create_message_destruct_007668e0(void* message) = 0;
    virtual void simple3d_scene_release_0093cc20(void* scene) = 0;
    virtual void ref_counted_destroy_base_00bd30f0(void* object) = 0;
    virtual void ordered_map_destruct_0094ca40(void* map) = 0;
};

// Out-of-line destructors of the form MOV [ECX], vtable (plus a second subobject vtable for
// 00743E30); JMP base. __thiscall(ECX = object), the JMP target's RET.
void landfort_create_message_destruct_00743e30(Census9Host& host, void* message);   // 00CFF358
void land_vehicle_create_message_destruct_0074cc90(Census9Host& host, void* message); // 00CFFD18
void simple3d_scene_destruct_0093cc80(Census9Host& host, void* scene);               // 00D19724
void path_source_destruct_007ac950(Census9Host& host, void* source);                 // 00D052DC
void ordered_map_destruct_0094cad0(Census9Host& host, void* map);                   // 00D25170

// 0080FCF0, __thiscall(ECX = message, unit, phase, force), RET 0Ch, returns message.
void* ship_guns_sync_construct_0080fcf0(Census9Host& host, void* message, void* unit,
    std::int32_t phase, bool force);
// 00760C10, __thiscall(ECX = message), RET, returns message: the factory (ConstructForRead) form.
void* ship_guns_sync_construct_for_read_00760c10(Census9Host& host, void* message);
// 00760CB0, __thiscall(ECX = message), RET.
void ship_guns_sync_destruct_00760cb0(Census9Host& host, void* message);
// 00939CB0, __thiscall(ECX = controller, unit), RET 4, returns controller.
void* unit_controller_construct_00939cb0(Census9Host& host, void* controller, void* unit);
// 00932840, __thiscall(ECX = controller), RET.
void unit_controller_destruct_00932840(Census9Host& host, void* controller);
// 00940E40, __thiscall(ECX = queue), RET.
void sound_request_queue_destruct_00940e40(Census9Host& host, void* queue);
// 006C23D0, __thiscall(ECX = holder, flags), RET 4, returns holder.
void* air_ops_holder_scalar_deleting_destruct_006c23d0(Census9Host& host, void* holder,
    std::uint32_t flags);

}  // namespace bsp::cc13_census9

#endif
