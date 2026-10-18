#pragma once
// Packet cc13-front7: wiring-frontier callees, round 2 (the large bodies skipped by
// cc13-front6 and the next ranked blockers of local/output/wire_readiness.json).
// docs/CC13_FRONT7.md lists every target with its ABI, evidence, the rows it
// unblocks and its uncertainty.
//
// Objects are reached through cc13_sweepA::Raw (byte offsets into the Win32
// layout). Every call the image makes goes through a host method named for its
// target; image globals are reached through global_address / global_pointer.
// Descriptive names are hypotheses, not recovered symbols. Nothing here is wired.

#include "bsp/cc13_sweepA.hpp"

#include <cstdint>

namespace bsp::cc13_front7 {

using cc13_sweepA::Raw;

// ---------------------------------------------------------------------------
// 0078CFF0: the environment's per-frame apply to a camera (ocean effects, clear
// colour, the camera's fog owner, the ocean wave field).
// ---------------------------------------------------------------------------
struct EnvironmentApplyHost {
    virtual ~EnvironmentApplyHost() = default;
    // The pointer stored in the image global dword at `address` ([00E188A8] the game).
    virtual void* global_pointer(std::uint32_t address) = 0;

    // 00BA40F0 BSP_OceanWaterParticles_Update, __thiscall(ECX = particles; camera, float clock) RET 8.
    virtual void ocean_water_particles_update_00ba40f0(void* particles, void* camera, float clock) = 0;
    // 00BA2FF0 BSP_OceanBeams_Update, __thiscall(ECX = beams; camera, float time) RET 8.
    virtual void ocean_beams_update_00ba2ff0(void* beams, void* camera, float time) = 0;
    // 004FB850 BSP_Color_ConvertFloatRgbaToArgb, __thiscall(ECX = out dword; const float* rgba) RET 4.
    virtual void color_float_rgba_to_argb_004fb850(std::uint32_t* out, const float* rgba) = 0;
    // 00B6FE50 BSP_Camera_SetClearColorWord, __thiscall(ECX = camera; dword argb) RET 4.
    virtual void camera_set_clear_color_00b6fe50(void* camera, std::uint32_t argb) = 0;
    // 00BA0090 BSP_OceanWaveField_Update, __thiscall(ECX = wave field; camera, float t) RET 8.
    virtual void ocean_wave_field_update_00ba0090(void* wave_field, void* camera, float t) = 0;

    // The camera's fog owner [camera+184h] setters, each __thiscall(ECX = owner; ...).
    // 00B84C40 BSP_SystemFogOwner_SetColor08 (const float* rgba) RET 4.
    virtual void fog_set_color_08_00b84c40(void* fog, const float* rgba) = 0;
    // 00B84FA0 BSP_SystemFogOwner_SetDirectionalColor28 (const float* rgba, int index) RET 8.
    virtual void fog_set_directional_color_28_00b84fa0(void* fog, const float* rgba, std::int32_t index) = 0;
    // 00B84C70 BSP_SystemFogOwner_SetUnderwaterColor18 (const float* rgba) RET 4.
    virtual void fog_set_underwater_color_18_00b84c70(void* fog, const float* rgba) = 0;
    // Scalar setters (float) RET 4.
    virtual void fog_set_scalar_68_00b84d00(void* fog, float value) = 0;
    virtual void fog_set_scalar_6c_00b84d10(void* fog, float value) = 0;
    virtual void fog_set_scalar_70_00b84d20(void* fog, float value) = 0;
    virtual void fog_set_scalar_74_00b84d30(void* fog, float value) = 0;
    virtual void fog_set_scalar_78_00b84d40(void* fog, float value) = 0;
    virtual void fog_set_scalar_7c_00b84d50(void* fog, float value) = 0;
    virtual void fog_set_scalar_80_00b84d60(void* fog, float value) = 0;
    virtual void fog_set_scalar_84_00b84d80(void* fog, float value) = 0;
    virtual void fog_set_scalar_88_00b84dc0(void* fog, float value) = 0;
    virtual void fog_set_scalar_8c_00b84de0(void* fog, float value) = 0;
    virtual void fog_set_scalar_90_00b84e00(void* fog, float value) = 0;
};

// 0078CFF0, __thiscall(ECX = environment; camera, float clock) RET 8.
void environment_apply_to_camera_0078cff0(Raw environment, void* camera, float clock,
                                          EnvironmentApplyHost& host);

// The VC9 checked iterator the image passes around: {_Mycont, _Ptr}.
struct MapIterator {
    void* container;
    std::uint8_t* node;
};
static_assert(sizeof(MapIterator) == 8);

// A VC9 std::vector<dword> image: +0 (not written by 0061B0F0), first, last, end.
struct DwordVectorImage {
    std::uint32_t head;
    std::uint32_t* first;
    std::uint32_t* last;
    std::uint32_t* end;
};
static_assert(sizeof(DwordVectorImage) == 16);

// ---------------------------------------------------------------------------
// 0061B0F0: the powerup-manager HUD screen's list rebuild.
// ---------------------------------------------------------------------------
struct PowerupListsHost {
    virtual ~PowerupListsHost() = default;
    // The pointer stored in the image global dword at `address` ([00E188A8] game,
    // [00E198C4] the in-game interface manager, [00F88C30] the powerup manager).
    virtual void* global_pointer(std::uint32_t address) = 0;
    // 00BF6713, the checked-iterator failure (_invalid_parameter_noinfo); returns here.
    virtual void invalid_parameter_00bf6713() = 0;
    // 00BF7FBF _stricmp, __cdecl.
    virtual std::int32_t stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 00BF65AC operator delete, __cdecl.
    virtual void operator_delete_00bf65ac(void* block) = 0;
    // 008E9AF0 BSP_PowerUpManager_CollectActive, __thiscall(ECX = manager; seven dwords) as pushed.
    virtual void powerup_manager_collect_active_008e9af0(void* manager, void* list_184, void* list_144,
                                                         void* list_154, void* list_164, void* list_174,
                                                         std::uint32_t zero_a, std::uint32_t zero_b) = 0;
    // 00441F30 std::vector<dword>::insert(where, value), __thiscall(ECX = vector; out
    // iterator*, iterator where {container, ptr} by value, const dword*) RET 10h.
    virtual MapIterator* vector_insert_one_00441f30(DwordVectorImage* vector, MapIterator* out,
                                                    MapIterator where, const std::uint32_t* value) = 0;
    // 00614060 STL_IntMap_Find, __thiscall(ECX = map; out*, const int* key) RET 8 -> EAX = out.
    virtual MapIterator* int_map_find_00614060(void* map, MapIterator* out, const void* key) = 0;
    // 006171B0 int-keyed map operator[], __thiscall(ECX = map; const int* key) RET 4 -> value*.
    virtual std::uint32_t* int_map_subscript_006171b0(void* map, const void* key) = 0;
    // 00613900 STL_StringMapCI_LowerBound, __thiscall(ECX = map; const NativeString* key) RET 4 -> node.
    virtual std::uint8_t* string_map_ci_lower_bound_00613900(void* map, const void* key) = 0;
    // 00617870 STL_StringMapCI_Subscript, __thiscall(ECX = map; const NativeString* key) RET 4 -> value*.
    virtual std::uint32_t* string_map_ci_subscript_00617870(void* map, const void* key) = 0;
    // 006170F0 map operator[], __thiscall(ECX = map; const dword* key) RET 4 -> value*.
    virtual std::uint32_t* map_subscript_006170f0(void* map, const std::uint32_t* key) = 0;
    // 00AB64C0 BSP_GuiIcon_AddState, __thiscall(ECX = icon list; dword, const void*) RET 8 -> EAX.
    virtual std::uint32_t gui_icon_add_state_00ab64c0(void* icons, std::uint32_t id, const void* data) = 0;
    // 004F6210 std::vector<dword> copy constructor, __thiscall(ECX = dest; const vector*) RET 4.
    virtual void dword_vector_copy_construct_004f6210(DwordVectorImage* dest, const DwordVectorImage* source) = 0;
    // 0061A400 BSP_HudPowerupManagerScreen_AssignLists, __thiscall(ECX = screen+100h;
    // vector a, vector b by value) RET 20h; the callee owns and destroys both copies.
    virtual void powerup_screen_assign_lists_0061a400(void* lists, DwordVectorImage* by_value_pair) = 0;
};

// 0061B0F0, __fastcall(ECX = screen) RET.
void powerup_screen_rebuild_lists_0061b0f0(Raw screen, PowerupListsHost& host);

}  // namespace bsp::cc13_front7
