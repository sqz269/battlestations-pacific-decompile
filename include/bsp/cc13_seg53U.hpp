// cc13 segment 53 / 47 leftovers (packet cc13-seg53U, cc9 gameplay terms approved 2026-10-16):
// the four game bodies left in 00851E10-008740E0 and 007B2DD0-007D1DC0 after the earlier cc13
// passes. docs/CC13_SEG53U.md carries the evidence and the adoption notes.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in
// the image) and reaches the engine through Seg53UHost, whose methods name the image routine or
// virtual slot they stand for and give the register inputs. Constants carry their image address
// and width. New C++ interfaces, not drop-in binary replacements; nothing here is bound into
// bsp_game. Names are hypotheses, not recovered symbols.

#ifndef BSP_CC13_SEG53U_HPP
#define BSP_CC13_SEG53U_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg53U {

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
// 00859950: the BSP_Replication_BuildSyncDelta tolerance arguments, in argument order.
inline constexpr std::int32_t kGunSyncOwnerSkipKind = 6;     // owner vtable[5Ch](6), 00859964
inline constexpr std::int32_t kGunSyncNoPeerValue = 8;       // 0085997E
inline constexpr float kGunSyncTolA = 300.0f;                // float 00CE3AE8
inline constexpr float kGunSyncTolB = 800.0f;                // float 00CE3950
inline constexpr float kGunSyncTolC = 500.0f;                // float 00CFFD60
inline constexpr float kGunSyncTolD = 3.402823466e+38f;      // float 00D7A248 (FLT_MAX)

// 00864380: the two vtable words re-installed first.
inline constexpr std::uint32_t kUnitGunneryAiVtable = 0x00D0D360;    // 008643A8
inline constexpr std::uint32_t kUnitGunneryAiVtable1C = 0x00D0D348;  // 008643AF

// 007B7290: the debug-draw statics (function-local statics in .bss with guard words) and
// the constants they are built from.
inline constexpr float kOne = 1.0f;                          // float 00D7A24C
inline constexpr float kHalf = 0.5f;                         // float 00CE3800
inline constexpr float kMarkerRadius = 10.0f;                // float 00CE38B8
inline constexpr double kHalfDouble = 0.5;                   // double 00D7A280
inline constexpr std::uint32_t kStaticRed = 0x00F87208;      // guard 00F87218 {1, 0, 0, 1}
inline constexpr std::uint32_t kStaticRedGuard = 0x00F87218;
inline constexpr std::uint32_t kStaticBlue = 0x00F8721C;     // guard 00F8722C {0, 0, 1, 1}
inline constexpr std::uint32_t kStaticBlueGuard = 0x00F8722C;
inline constexpr std::uint32_t kStaticWhite = 0x00F871F4;    // guard 00F87204 {1, 1, 1, 1}
inline constexpr std::uint32_t kStaticWhiteGuard = 0x00F87204;
inline constexpr std::uint32_t kStaticGreen = 0x00F87230;    // guard 00F87240 {0, 1, .5, 1}
inline constexpr std::uint32_t kStaticGreenGuard = 0x00F87240;
inline constexpr std::uint32_t kStaticCyan = 0x00F87244;     // guard 00F87254 {0, .5, 1, 1}
inline constexpr std::uint32_t kStaticCyanGuard = 0x00F87254;

struct Seg53UHost {
    virtual ~Seg53UHost() = default;

    // The storage of image global `address` (.data / .bss).
    virtual void* global(std::uint32_t address) = 0;
    // 00BF6713, the checked-iterator invalid-parameter report.
    virtual void invalid_parameter_00bf6713() = 0;
    // 00BF65AC operator delete / 00BF6989 _free, cdecl(pointer).
    virtual void operator_delete_00bf65ac(void* p) = 0;
    virtual void crt_free_00bf6989(void* p) = 0;

    // --- 00859950 ---
    // [object]->vtable[5Ch](ECX = object; int kind), RET 4 -> AL (IsKindOf).
    virtual bool object_is_kind_of_vslot_5c(void* object, std::int32_t kind) = 0;
    // 00927850 BSP_UnitHasRoleSlotValue, __thiscall(ECX = unit; int value), RET 4 -> AL.
    virtual std::uint8_t unit_has_role_slot_value_00927850(void* unit, std::int32_t value) = 0;
    // 0077B280 BSP_Replication_BuildSyncDelta, cdecl, 13 dwords. `role` is the whole dword
    // the image pushes (00859999 writes AL over the low byte of its own argument slot).
    virtual void replication_build_sync_delta_0077b280(void* entity, void* record,
                                                       std::uint32_t role, float a0,
                                                       std::int32_t a1, float a2,
                                                       std::int32_t a3, float a4,
                                                       std::int32_t a5, float a6,
                                                       std::int32_t a7, float a8,
                                                       std::int32_t a9) = 0;

    // --- 00864380 ---
    // [object]->vtable[0](ECX = object; 1), RET 4: the scalar deleting destructor.
    virtual void object_delete_vslot_0(void* object, std::int32_t flags) = 0;
    // 00862110, the tree iterator ++, __thiscall(ECX = iterator {container, node}).
    virtual void tree_iterator_increment_00862110(void* iterator) = 0;
    // 00863BF0, the tree erase(iterator), __thiscall(ECX = tree; out iterator*, container,
    // node), RET 0Ch (ledger name STL_xlen_throw_00863bf0 does not fit this call).
    virtual void tree_erase_00863bf0(void* tree, void* out, void* container, void* node) = 0;
    // 00864090 STL_Tree_EraseRange, __thiscall(ECX = tree; out*, container, first, container,
    // last), RET 14h.
    virtual void tree_erase_range_00864090(void* tree, void* out, void* container0,
                                           void* first, void* container1, void* last) = 0;
    // 007293D0 BSP_TickSubnodeOwner_DestructBase, __fastcall(ECX = object).
    virtual void tick_subnode_owner_destruct_base_007293d0(void* object) = 0;

    // --- 007B7290 ---
    // 0099B6E0, a trivial body, __fastcall(ECX = task).
    virtual void trivial_0099b6e0(void* task) = 0;
    // 00860BE0 BSP_DebugDraw_WireSphereMarker, __fastcall(ECX = center, EDX = rgba; float
    // radius, const float* second, char detailed), RET 0Ch.
    virtual void debug_wire_sphere_marker_00860be0(const float* center, const float* rgba,
                                                   float radius, const float* second,
                                                   std::int32_t detailed) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, __fastcall(ECX = entity).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 0042B2F0 BSP_Vector3_LengthFloatThreshold, __fastcall(ECX = vec) -> ST0.
    virtual double vector3_length_0042b2f0(const float* vec) = 0;
    // 0085F990 BSP_DebugDraw_LineRgbaFloats, __fastcall(ECX = p0, EDX = p1; rgba0 by value,
    // rgba1 by value), RET 20h.
    virtual void debug_line_rgba_0085f990(const float* p0, const float* p1, const float* rgba0,
                                          const float* rgba1) = 0;

    // --- 007D06A0 ---
    // 00408720 BSP_CharacterString_AssignCounted, __thiscall(ECX = string; const char*, count),
    // RET 8; 007B7CC0 STL_String_AppendPtrCount, same ABI.
    virtual void string_assign_counted_00408720(void* string, const char* text,
                                                std::size_t count) = 0;
    virtual void string_append_007b7cc0(void* string, const char* text, std::size_t count) = 0;
    // 00BF9010 __itoa, cdecl(int value, char* buffer, int radix).
    virtual void itoa_00bf9010(std::int32_t value, char* buffer, std::int32_t radix) = 0;
    // 0071AD50 BSP_Model_FindNoteNode, __thiscall(ECX = model; const char* name), RET 4 -> EAX.
    virtual void* model_find_note_node_0071ad50(void* model, const char* name) = 0;
    // 00505D50 STL_CheckedDwordVector_InsertOne, __thiscall(ECX = vector; out*, container,
    // where, const value*), RET 10h.
    virtual void dword_vector_insert_one_00505d50(void* vector, void* out, void* container,
                                                  void* where, const void* value) = 0;
};

// 00859950, the turning gun's sync-delta slot (vtables 00CFBEA0, 00CFC0D8, 00CFC310,
// 00CFE6C8): __thiscall(ECX = gun; record), RET 4.
void gun_build_sync_delta_00859950(void* gun, void* record, Seg53UHost& host);

// 00864380 BSP_UnitGunneryAi_Destruct: __thiscall(ECX = gunnery ai), plain RET.
void unit_gunnery_ai_destruct_00864380(void* ai, Seg53UHost& host);

// 007B7290 BSP_BotTaskRocket_DebugDraw: __fastcall(ECX = task), plain RET.
void bot_task_rocket_debug_draw_007b7290(void* task, Seg53UHost& host);

// 007D06A0 BSP_Model_CollectNumberedNoteNodes: ECX = model, EDX = prefix; vector* on the
// stack, RET 4.
void model_collect_numbered_note_nodes_007d06a0(void* model, const char* prefix, void* vector,
                                                Seg53UHost& host);

}  // namespace bsp::cc13_seg53U

#endif  // BSP_CC13_SEG53U_HPP
