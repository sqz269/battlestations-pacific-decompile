// Segments 52/53, large or skipped bodies the first passes left (packet cc13-big53).
//
// 0082FE30 (the ship class's model-data binder, 19D1h bytes) and 00863130 (the unit gunnery AI's
// property reader). docs/CC13_BIG53.md carries the evidence and the status of every routine.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in the
// image) and reaches the engine through Big53Host, whose methods name the image routine or the
// virtual slot they stand for. New C++ interfaces, not drop-in binary replacements; nothing here is
// bound into bsp_game. Names are hypotheses, not recovered symbols. Marker: cc13_big53.

#ifndef BSP_CC13_BIG53_HPP
#define BSP_CC13_BIG53_HPP

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_seg33a.hpp"
#include "bsp/cc13_seg53.hpp"

namespace bsp::cc13_big53 {

using cc13_seg33a::ArchiveArg;

// ---------------------------------------------------------------------------
// Image constants.
// ---------------------------------------------------------------------------
inline constexpr float kOne_00d7a24c = 1.0f;          // 00D7A24C, float
inline constexpr float kMinusOne_00d7a260 = -1.0f;    // 00D7A260, float
inline constexpr double kHalf_00d7a280 = 0.5;         // 00D7A280, double
inline constexpr float kNegZero_00d7a208 = -0.0f;     // 00D7A208, float (bits 80000000h)

// The model point-group names 0082FE30 asks for (std::string built with 00408720 and the length).
inline constexpr const char* kDecklineName = "deckline";        // 00D09A0C, 8
inline constexpr const char* kBottomlineName = "bottomline";    // 00D09A00, 0Ah
inline constexpr const char* kDebarkationName = "debarkation";  // 00D099F4, 0Bh
inline constexpr const char* kOrrhullamName = "orrhullam";      // 00D099E8, 9
inline constexpr const char* kWaveName = "wave";                // 00D06560, 4
inline constexpr const char* kShipcenterName = "shipcenter";    // 00D099DC, 0Ah
inline constexpr const char* kWaveSternName = "wave_stern";     // 00D099D0, 0Ah
inline constexpr const char* kKemenyName = "kemeny";            // 00CFFD90, 6
inline constexpr const char* kFarvizName = "farviz";            // 00D099C8, 6
inline constexpr const char* kExplosionName = "explosion";      // 00D05A18, 9
inline constexpr const char* kPathName = "path";                // 00D099C0, 4
inline constexpr const char* kIdleName = "idle";                // 00D099B8, 4

// The keys 00863130 reads.
inline constexpr const char* kBotSectionKey = "_bot";           // 00CF904C
inline constexpr const char* kThinkTimeKey = "thinkTime";       // 00D0D33C
inline constexpr const char* kFireSectionKey = "fire";          // 00CE6798
inline constexpr const char* kEnableSectionKey = "enable";      // 00D0D334

struct Big53Host {
    virtual ~Big53Host() = default;

    // --- 0082FE30 callees -------------------------------------------------------------------
    // 0095F500 BSP_VehicleClass_BindModelData, __fastcall(ECX class).
    virtual void vehicle_class_bind_model_data_0095f500(void* cls) = 0;
    // 0082D700 BSP_ShipClass_AppendModelType54Pointers_Provisional, __fastcall(ECX class).
    virtual void ship_class_append_model_type54_pointers_0082d700(void* cls) = 0;
    // 00408720, __thiscall(ECX std::string (1Ch bytes), const char*, length), RET 8.
    virtual void std_string_assign_00408720(void* string, const char* text, std::uint32_t length) = 0;
    // 00BF65AC: free (the std::string tidy of a heap buffer).
    virtual void free_00bf65ac(void* block) = 0;
    // 00BF6713: the checked-iterator / checked operator[] failure report (_invalid_parameter_noinfo).
    virtual void invalid_parameter_00bf6713() = 0;
    // 00718000 BSP_GameResource_FindLastNamedPointGroup, __thiscall(ECX resource, const
    // std::string*, kind), RET 8 -> node (last match or null).
    virtual void* model_find_last_point_group_00718000(void* resource, const void* name,
                                                       std::uint32_t kind) = 0;
    // 00718870 BSP_GameResource_LookupNamedPointGroup, __thiscall(ECX resource, const
    // std::string*, index), RET 8 -> node or null.
    virtual void* model_lookup_point_group_00718870(void* resource, const void* name,
                                                    std::uint32_t index) = 0;
    // 0082D040, __thiscall(ECX class, bottom points vector, deck points vector, float ratio),
    // RET 0Ch: the buoyancy element producer (hosted by cc9, src/ship_buoyancy_elements.cpp).
    virtual void ship_build_buoyancy_elements_0082d040(void* cls, void* bottom_points,
                                                       void* deck_points, float ratio) = 0;
    // 004D1510 STL_CheckedVector12_Resize, __thiscall(ECX vector, count, 0Ch-byte prototype by
    // value), RET 10h.
    virtual void vector12_resize_004d1510(void* vector, std::uint32_t count, const void* prototype) = 0;
    // 004F9B30 BSP_Vector3f_Cross, ECX = out, EDX = a, stack b, RET 4 -> EAX = out.
    virtual float* vector3_cross_004f9b30(float* out, const float* a, const float* b) = 0;
    // 0085DC80 BSP_Matrix_OrthonormalizeBasisRows, __fastcall(ECX float[16]) in place.
    virtual void matrix_orthonormalize_rows_0085dc80(float* matrix) = 0;
    // 00829180 BSP_ShipIndexedPointArray_ReserveCapacity, __thiscall(ECX array, capacity), RET 4.
    virtual void indexed_points_reserve_00829180(void* array, std::int32_t capacity) = 0;
    // 0074D190 BSP_PointsArray_ReserveCapacity, __thiscall(ECX array, capacity), RET 4.
    virtual void points_array_reserve_0074d190(void* array, std::int32_t capacity) = 0;
    // 0082E990, __thiscall(ECX vector of 10h-byte vector headers, count, 10h-byte prototype by
    // value), RET 14h: resize.
    virtual void path_vectors_resize_0082e990(void* vector, std::uint32_t count, const void* prototype) = 0;
    // 004FBA10 BSP_PointGroup_CountPoints, __fastcall(ECX node) -> count.
    virtual std::int32_t point_group_count_points_004fba10(void* node) = 0;
    // 00484270, __thiscall(ECX node, float* out, index), RET 8 -> const float* (the point).
    virtual const float* point_group_point_00484270(void* node, float* out, std::uint32_t index) = 0;
    // 004215D0 MSVC_Vector12_PushBack, __thiscall(ECX vector, const float* point), RET 4.
    virtual void vector12_push_back_004215d0(void* vector, const float* point) = 0;
    // 0082F4E0, __thiscall(ECX vector, iterator* result, iterator first {container, ptr},
    // iterator last {container, ptr}), RET 14h: erase a range.
    virtual void vector_erase_range_0082f4e0(void* vector, void* result, void* first_container,
                                             std::uintptr_t first, void* last_container,
                                             std::uintptr_t last) = 0;
    // 0082CA40 STL_Vector_PushBack, __thiscall(ECX vector of float[16], const float* matrix), RET 4.
    virtual void matrices_push_back_0082ca40(void* vector, const float* matrix) = 0;
    // 0082E1F0, __thiscall(ECX 1Ch-byte idle record): default construct (the resize prototype).
    virtual void idle_record_construct_0082e1f0(void* record) = 0;
    // 0082F8E0, __thiscall(ECX vector of 1Ch-byte records, count, record by value), RET 20h.
    virtual void idle_records_resize_0082f8e0(void* vector, std::uint32_t count, void* prototype) = 0;
    // 0082C560 STL_Vector_Resize, __thiscall(ECX vector of float[16], count, float[16] by value),
    // RET 44h.
    virtual void matrices_resize_0082c560(void* vector, std::uint32_t count, const float* matrix) = 0;

    // --- 00863130 callees -------------------------------------------------------------------
    // Archive slots: +4 open section (key), +8 close section, +10h read field (key, value).
    virtual void archive_open_section_vt04(void* archive, ArchiveArg key) = 0;
    virtual void archive_close_section_vt08(void* archive) = 0;
    virtual void archive_read_field_vt10(void* archive, ArchiveArg key, ArchiveArg value) = 0;
    // 008FE660 BSP_GunBot_ReadBaseProperties, __thiscall(bot, archive), RET 4.
    virtual void gun_bot_read_base_properties_008fe660(void* bot, void* archive) = 0;
    // 00BF681B operator new (cdecl size).
    virtual void* operator_new_00bf681b(std::size_t size) = 0;

    // --- seg 53 debug geometry callees ------------------------------------------------------
    // 00B62D10 BSP_Vector4f_Transform, __thiscall(ECX const float v[4], float out[4], const
    // float m[16]), RET 8 -> out.
    virtual const float* vector4_transform_00b62d10(const float* v, float* out, const float* m) = 0;
    // 004142E0, __thiscall(ECX const float point[3], float out[3], const float m[16]), RET 8 -> out.
    virtual const float* vector3_transform_affine_point_004142e0(const float* point, float* out,
                                                                 const float* m) = 0;
    // 0042B260 BSP_Geometry_NormalizeVectorWithFloor, __fastcall(ECX float v[3]), in place.
    virtual void normalize_vector_with_floor_0042b260(float* v) = 0;
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix, __fastcall(ECX transform).
    virtual void transform_refresh_world_matrix_00b6db70(void* transform) = 0;
    // 0043A870 BSP_Camera_ProjectWorldToScreenXY, __fastcall(ECX const float world[3], EDX float
    // out[2]; int, int), RET 8 -> EAX.
    virtual std::int32_t camera_project_0043a870(const float* world, float* out, std::int32_t a,
                                                 std::int32_t b) = 0;
    // 004C7070 BSP_DiagnosticPrintf, cdecl(game, float x, float y, format, ...).
    virtual void diagnostic_printf_004c7070(void* game, float x, float y, const char* format) = 0;
    // [00E188A8]: the game object.
    virtual void* game_00e188a8() = 0;
    // 00F875A8: the function-static colour of 008608B0 (float rgba[4] at +0, guard dword at +10h
    // = 00F875B8, bit 0 = initialised). Image storage, zero-filled .bss at load.
    virtual std::uint8_t* heading_colour_static_00f875a8() = 0;
};

// 0082FE30, __fastcall(ECX ship class), plain RET; slot 8 (vtable+20h) of every ship-kind class
// vtable. Binds the class's named model point groups to class fields (see the .cpp).
void ship_class_bind_model_data_0082fe30(Big53Host& host, void* cls);

// 00863130, __thiscall(ECX gunnery AI, archive), RET 4: the property reader.
void unit_gunnery_ai_load_properties_00863130(Big53Host& host, void* ai, void* archive);

// --- seg 53 debug geometry (main thread; x87 modelled at 24-bit precision control) -----------
// The renderer slot B4h and 0085F990 are reached through cc13_seg53 (its Seg53Host and its
// debug_draw_line_rgba_floats_0085f990 model).

// 0085C5D0, __fastcall(ECX const float m[16], EDX const float origin[3]; const float delta[3],
// float* t_plus, float* t_minus, bool allow_inside_start), RET 10h -> AL. Segment origin..origin+
// delta against the unit sphere in the space of m (row-vector transform 00B62D10, then /w).
bool ellipsoid_intersect_segment_0085c5d0(Big53Host& host, const float* m, const float* origin,
                                          const float* delta, float* t_plus, float* t_minus,
                                          bool allow_inside_start);

// 0085FB70, __fastcall(ECX const float from[3], EDX const float to[3]; r0, g0, b0, a0, r1, g1,
// b1, a1), RET 20h. The line (inlined 0085F990), then two barbs at `to` in colour 1.
void debug_draw_arrow_rgba_floats_0085fb70(cc13_seg53::Seg53Host& draw, Big53Host& host,
                                           const float* from, const float* to, float r0, float g0,
                                           float b0, float a0, float r1, float g1, float b1,
                                           float a1);

// 0085FFF0, __fastcall(ECX from, EDX to; float rgba[4] by value), RET 10h: 0085FB70 with the
// colour twice.
void debug_draw_arrow_rgba_floats_one_colour_0085fff0(cc13_seg53::Seg53Host& draw, Big53Host& host,
                                                      const float* from, const float* to,
                                                      const float* rgba);

// 00860140, __fastcall(ECX const float frame[16], EDX const float rgba[4]; float radius, float
// start, float sweep, int segments), RET 10h.
void debug_draw_arc_in_frame_00860140(cc13_seg53::Seg53Host& draw, Big53Host& host,
                                      const float* frame, const float* rgba, float radius,
                                      float start, float sweep, std::int32_t segments);

// 008605C0, __fastcall(ECX const float box[6] {min, max}, EDX const float rgba[4]), RET: the
// twelve edges.
void debug_draw_aabb_wireframe_008605c0(cc13_seg53::Seg53Host& draw, const float* box,
                                        const float* rgba);

// 00860710, __fastcall(ECX const float centre[3], EDX const float rgba[4]; float radius), RET 4:
// 64 segments in the XZ plane.
void debug_draw_circle64_00860710(cc13_seg53::Seg53Host& draw, const float* centre,
                                  const float* rgba, float radius);

// 008608B0, __fastcall(ECX const float centre[3]; float radius, float px, float py, float pz),
// RET 10h. Circle64 in the static orange, then a radius-long line from the centre away from p
// (XZ only).
void debug_draw_circle_with_heading_008608b0(cc13_seg53::Seg53Host& draw, Big53Host& host,
                                             const float* centre, float radius, float px,
                                             float py, float pz);

// 00861260, __fastcall(ECX const float world[3], EDX text object {+4 const char*}), RET.
void debug_draw_text_at_world_point_00861260(Big53Host& host, const float* world,
                                             const void* text);

}  // namespace bsp::cc13_big53

#endif  // BSP_CC13_BIG53_HPP
