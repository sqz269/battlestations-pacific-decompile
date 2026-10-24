#pragma once
// cc13-pool15: three segment-9 routines from the progress-board pool whose earlier
// records carried no body credit (metadata_only, Oct24 audit). Each is rebuilt over
// raw native memory (the world's border-zone std::lists, NativeString headers); every
// call the binary makes is a Pool15Host hook at the binary's point with its arguments.
// Library code is bound, not rebuilt: memcpy (00BF7680) and _invalid_parameter_noinfo
// (00BF6713, the checked-iterator failure; it returns).
//
// The live host already has typed models of the two border-zone routines over
// BorderZoneSet vectors (src/world_map_bounds.cpp build_border_zones_004d5bd0 for the
// layout; src/plane_retreat_task.cpp border_zone_at_point_00489c40 for the lookup).
// These bodies do not replace or edit them. Names are hypotheses, not recovered
// symbols. Reconstructed and build-tested only.

#include <cstdint>

namespace bsp::pool15_cc13 {

struct Pool15Host {
    virtual ~Pool15Host() = default;

    // --- library bindings (no reconstruction credit) ---
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t bytes) = 0;
    // 00BF6713 _invalid_parameter_noinfo, cdecl, no arguments; returns.
    virtual void invalid_parameter_noinfo_00bf6713() = 0;

    // --- game code, called not rebuilt here ---
    // 004B6C40 BSP_Game_GetMapBoxSpan, __thiscall(ECX = world; edge) RET 4, result on
    // ST0. It is a single float subtraction, so under either precision control the
    // value is float-representable; it is returned as float.
    virtual float game_map_box_span_004b6c40(void* world, std::int32_t edge) = 0;
    // 0049F260 BSP_Traffic_RefreshCurrentLayersIfSet, __thiscall(ECX = traffic), reached by
    // a tail JMP.
    virtual void traffic_refresh_current_layers_if_set_0049f260(void* traffic) = 0;
    // 004260B0 BSP_NativeString_FromInt, __thiscall(ECX = header; int) RET 4.
    virtual void native_string_from_int_004260b0(void* header, std::int32_t value) = 0;
    // 0041DD40 BSP_NativeString_Resize(ECX = header; length, preserve) RET 8.
    virtual void native_string_resize_0041dd40(void* header, std::uint32_t length,
        std::uint32_t preserve) = 0;
    // 00419CC0 (no arguments, EAX = pool) then 00BD1510 ReturnBlock(ECX = pool; data,
    // length + 1, 1) RET 0Ch.
    virtual void* sized_pool_00419cc0() = 0;
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t size,
        std::uint32_t flag) = 0;
};

// 004C71C0, __fastcall(ECX = world), plain RET (or tail JMP 0049F260).
void world_layout_border_zone_records_004c71c0(void* world, Pool15Host& host);
// 004C74C0, __thiscall(ECX = world; const float point[3]*, iterator* out) RET 8.
// out is a checked std::list iterator {+0 list, +4 node}.
void world_border_zone_at_point_004c74c0(void* world, const float* point, void* out_iterator,
    Pool15Host& host);
// 004CACA0, __thiscall(ECX = string; int value) RET 4.
void native_string_append_int_004caca0(void* string, std::int32_t value, Pool15Host& host);

}  // namespace bsp::pool15_cc13
