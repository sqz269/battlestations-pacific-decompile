#include "bsp/ship_ai_avoid_zone_search.hpp"
#include "bsp/avoid_zone_manager_queries.hpp"
#include "bsp/geometry_helpers.hpp"
#include <stdexcept>

namespace bsp {
namespace {
// CEC160 is3FF3333340000000, the exact double expansion of binary32 1.2.
// Replacing it with a double literal1.2 changes the native products.
constexpr double expansion = 1.2000000476837158203125;
constexpr double half = 0.5;
constexpr double minimum = 500.0; // CE3840; CE397C is the float500 replacement.
float spill(float input) noexcept {
    __asm {
        fld input
        fstp input
    }
    return input;
}
float add(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fadd right
        fstp result
    }
    return result;
}
float subtract(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fsub right
        fstp result
    }
    return result;
}
float product(float input, const double& factor) noexcept {
    float result;
    __asm {
        mov eax,factor
        fld input
        fmul qword ptr [eax]
        fstp result
    }
    return result;
}
bool above(float left, float right) noexcept {
    unsigned char result;
    __asm {
        fld right
        fld left
        fcomip st(0),st(1)
        fstp st(0)
        seta result
    }
    return result!=0;
}
bool minimum_above(float right) noexcept {
    unsigned char result;
    __asm {
        fld right
        fld minimum
        fcomip st(0),st(1)
        fstp st(0)
        seta result
    }
    return result!=0;
}
std::array<float,4> bounds(const ShipAiAvoidZoneSearcher& cache) noexcept {
    return {cache.min_x,cache.min_z,cache.max_x,cache.max_z};
}
} // namespace

void ship_ai_avoid_list_set_layer_00419fa0(ShipAiAvoidZoneSegmentList& list,
    std::int32_t layer, const AvoidZoneAllocationAccess& allocation) {
    if (list.layer!=layer) //00419FA8..AB; clear is called even for an empty head.
        avoid_zone_selected_segments_clear_004158a0(list.head,allocation);
    list.layer=layer; //00419FB2, after clear.
}

void ship_ai_avoid_list_refill_004224c0(ShipAiAvoidZoneSegmentList& list,
    const ShipAiAvoidZoneSearcher& cache, ShipAiAvoidZoneSearchAccess& access) {
    if (list.head) avoid_zone_selected_segments_clear_004158a0(list.head,access.allocation());
    const auto& table=access.manager_004218e0(); //004224CD, before the live key load.
    if (table.groups.empty())
        throw std::logic_error("Native avoid-zone search requires manager group slot zero");
    const auto index=avoid_zone_group_for_layer_004120d0(table,list.layer);
    if (index<0) return;
    const auto native=access.native_group(table.groups[static_cast<std::size_t>(index)]);
    list.head=avoid_zone_group_select_segments_00417a40(native,bounds(cache),access.allocation());
}

bool ship_ai_avoid_query_refresh_009d7050(ShipAiAvoidZoneSearcher& cache,
    ShipAiAvoidZoneSegmentList& list, const ShipAiAvoidZoneQuery& query,
    ShipAiAvoidZoneSearchAccess& access) {
    if (!cache.enabled) return false;
    if (cache.layer_key==query.layer_key) { //009D7068 is integer CMP, not float equality.
        const float x=spill(query.x),z=spill(query.z);
        const std::array<float,2> upper{spill(add(x,query.half_width)),
            spill(add(z,query.half_height))};
        const std::array<float,2> lower{spill(subtract(x,query.half_width)),
            spill(subtract(z,query.half_height))};
        const auto old_bounds=bounds(cache);
        if (contains_point_00414f50(old_bounds,upper.data())
            && contains_point_00414f50(old_bounds,lower.data())) return false;
    }
    const float width=subtract(cache.max_x,cache.min_x); //009D70F9..7101.
    const float height=subtract(cache.max_z,cache.min_z); //009D7105..710B.
    const float old_x=product(width,half),old_z=product(height,half);
    float half_x=product(query.half_width,expansion);
    if (above(old_x,half_x)) half_x=old_x; //JBE selects query on unordered.
    if (minimum_above(half_x)) half_x=500.0f;
    float half_z=product(query.half_height,expansion);
    if (above(old_z,half_z)) half_z=old_z;
    if (minimum_above(half_z)) half_z=500.0f;

    const float x=spill(query.x);
    cache.min_x=subtract(x,half_x); //009D71F1 precedes the native z spill at009D71F5.
    const float z=spill(query.z);
    const float low_z=subtract(z,half_z);
    const float high_x=add(x,half_x),high_z=add(z,half_z);
    cache.min_z=spill(low_z); //009D7221.
    cache.max_x=spill(high_x); //009D7228.
    cache.max_z=spill(high_z); //009D722F.
    const auto layer=query.layer_key;
    cache.layer_key=layer; //009D7235, before either helper call.
    ship_ai_avoid_list_set_layer_00419fa0(list,layer,access.allocation());
    ship_ai_avoid_list_refill_004224c0(list,cache,access);
    return true;
}
} // namespace bsp
