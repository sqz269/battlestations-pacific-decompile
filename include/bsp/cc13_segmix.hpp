#pragma once

// cc13_segmix: undone game bodies from segments 51, 58, 30, 41, 42, 50, 61, 54
// and 46 that have no cc9 host (packet cc13-seg40F segmix, docs/CC13_SEGMIX.md).
// Names are hypotheses, not recovered symbols. These are new C++ interfaces over
// the native records, not drop-in replacements. Nothing is wired. Same
// conventions as cc13_seg40: raw byte blocks touched only at the offsets the image
// uses, and one named host hook per native call, at the same point with the same
// arguments.

#include <cstdint>

namespace bsp {

class Cc13SegmixHost {
public:
    virtual ~Cc13SegmixHost() = default;

    // ---- CRT ----
    // 00BF65AC _free (cdecl, block).
    virtual void crt_free_00bf65ac(void* block) = 0;

    // ---- SHIP_CREATE base message (native) ----
    // 00813A20: __thiscall(ECX = message; word, ship, type), RET 0Ch.
    virtual void ship_create_construct_00813a20(void* message, std::uint32_t word, void* ship,
                                                std::uint32_t type) = 0;
    // 0080E2C0 BSP_SessionMessage_SHIP_CREATE_ConstructForRead: __fastcall(ECX = message).
    virtual void ship_create_construct_for_read_0080e2c0(void* message) = 0;
    // 0080E380, the SHIP_CREATE destructor: __thiscall(ECX = message).
    virtual void ship_create_destruct_0080e380(void* message) = 0;
    // 00813B10: __thiscall(ECX = message; writer), RET 4.
    virtual void ship_create_write_00813b10(void* message, void* writer) = 0;
    // 006CC800 BSP_AirOpsStockSnapshot_Capture: __thiscall(ECX = snapshot; block or null), RET 4.
    virtual void air_ops_stock_snapshot_capture_006cc800(void* snapshot, void* block) = 0;
    // 006D44F0 BSP_AirOpsStockSnapshot_Write: __thiscall(ECX = snapshot; writer), RET 4.
    virtual void air_ops_stock_snapshot_write_006d44f0(void* snapshot, void* writer) = 0;
    // 00779FC0 BSP_Entity_SendMessageToPeer: __thiscall(ECX = entity; peer, message), RET 8.
    virtual void entity_send_message_to_peer_00779fc0(void* entity, std::uint32_t peer, void* message) = 0;

    // ---- 007593D0 mother-ship init ----
    // 00822C20 BSP_UnitInstance_SEntityInit: __thiscall(ECX = unit).
    virtual void unit_instance_sentity_init_00822c20(void* unit) = 0;
    // 0071AD50 BSP_Model_FindNoteNode: __thiscall(ECX = model; const char* key), RET 4; EAX = node.
    virtual void* model_find_note_node_0071ad50(void* model, const char* key) = 0;
    // 00B6DB60 BSP_Node_GetLocalMatrix: __thiscall(ECX = node); EAX = the 4x4.
    virtual const void* node_get_local_matrix_00b6db60(void* node) = 0;
    // 004134F0 BSP_Matrix_Copy4x4X87: __thiscall(ECX = destination; source), RET 4.
    virtual void matrix_copy_4x4_004134f0(void* destination, const void* source) = 0;
    // 006C0D20 BSP_AirOpsBlock_CreateHolder: __thiscall(ECX = block; const float* point,
    // float a, float b), RET 0Ch.
    virtual void air_ops_block_create_holder_006c0d20(void* block, const float* point, float a, float b) = 0;
    // 006CADD0 BSP_AirOps_LoadFromScene: __thiscall(ECX = block; properties), RET 4.
    virtual void air_ops_load_from_scene_006cadd0(void* block, void* properties) = 0;
    // 006D0A10 BSP_MotherShipSite_Create: __thiscall(ECX = ship); EAX = the site.
    virtual void* mothership_site_create_006d0a10(void* ship) = 0;
    // 009238A0 SEntity_GetSavedEntityLuaData: __thiscall(ECX = entity; out 14h LuaObject), RET 4.
    virtual void saved_entity_lua_data_009238a0(void* entity, void* out_lua_object) = 0;
    // 004425C0 BSP_LuaReader_Construct: __thiscall(ECX = reader; 14h LuaObject by value), RET 14h.
    virtual void lua_reader_construct_004425c0(void* reader, void* lua_object_by_value) = 0;
    // 00BD8E20 BSP_LuaReader_EnterKey: __thiscall(ECX = reader; int kind, key), RET 8.
    virtual void lua_reader_enter_key_00bd8e20(void* reader, std::int32_t kind, const char* key) = 0;
    // 00BD7A20 BSP_LuaReader_LeaveKeyThunk: __thiscall(ECX = reader).
    virtual void lua_reader_leave_key_00bd7a20(void* reader) = 0;
    // 006BEF30 BSP_AirOpsBlock_ReadTrafficTable: __thiscall(ECX = block; reader), RET 4.
    virtual void air_ops_block_read_traffic_table_006bef30(void* block, void* reader) = 0;
    // 00441A20 BSP_LuaReader_DestroyStackInstance: __thiscall(ECX = reader).
    virtual void lua_reader_destroy_00441a20(void* reader) = 0;
    // 006BA670 BSP_AirOpsBlock_ForwardToTraffic08: __thiscall(ECX = block; float dt), RET 4.
    virtual void air_ops_block_tick_006ba670(void* block, float dt) = 0;

    // ---- session messages ----
    // 0075B430, the session message base constructor: __thiscall(ECX = message; type), RET 4.
    virtual void session_message_construct_0075b430(void* message, std::uint32_t type) = 0;
};

// ---- segment 42: the mother ship and its MOTHERSHIP_CREATE message ----
// The message is SHIP_CREATE (12Ch bytes) plus an air-ops stock snapshot at +12Ch
// (vtable 00CF8EB8, vectors +138h..+140h and +148h..+150h); 154h bytes in all.
void cc13_segmix_mothership_create_destruct_00759020(void* message, Cc13SegmixHost& host);
void cc13_segmix_mothership_create_write_007590e0(void* message, void* writer, Cc13SegmixHost& host);
void* cc13_segmix_mothership_create_construct_007592a0(void* message, std::uint32_t word, void* ship,
                                                       Cc13SegmixHost& host);
void cc13_segmix_mothership_send_create_message_00759330(void* ship, std::uint32_t peer, Cc13SegmixHost& host);
void cc13_segmix_mothership_init_all_007593d0(void* ship, Cc13SegmixHost& host);
void* cc13_segmix_landingship_create_construct_for_read_00759920(void* message, Cc13SegmixHost& host);
void* cc13_segmix_mothership_create_construct_for_read_00766ef0(void* message, Cc13SegmixHost& host);

// ---- segment 61: ship systems and failures ----
void* cc13_segmix_failure_message9b_construct_0093a2c0(void* message, std::uint32_t value_20, std::uint32_t value_24,
                                                       std::uint8_t flag_28, Cc13SegmixHost& host);

}  // namespace bsp
