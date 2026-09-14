#include "bsp/part_damage_reachability.hpp"

#include <cctype>

namespace bsp {

// 00E08138, in table order. Ghidra's symbol PTR_s_lwing_00e08138 names index 0.
const char* const kMeshCategoryNames[kMeshCategoryCount] = {
    "lwing",       // 0  00CE4454
    "rwing",       // 1  00CE444C
    "fuselage",    // 2  00CE4440
    "engine",      // 3  00CE4438
    "underwater",  // 4  00CE442C
    "engineroom",  // 5  00CE4420
    "fueltank",    // 6  00CE4414
    "steering",    // 7  00CE4408
    "magazine",    // 8  00CE43FC
    "body",        // 9  00CE43F4
    "none",        // 10 00CE43EC
    "runway",      // 11 00CE43E4
    "hangar",      // 12 00CE43DC
    "fizika",      // 13 00CE43D4
    "bullet",      // 14 00CE43CC
};

namespace {

// 00438E10 BSP_CString_CompareInsensitive, reduced to the predicate 007149D0
// uses: it only tests the result against zero.
bool equal_insensitive(const char* a, const char* b) noexcept {
    for (;; ++a, ++b) {
        const unsigned char ca = static_cast<unsigned char>(*a);
        const unsigned char cb = static_cast<unsigned char>(*b);
        if (std::tolower(ca) != std::tolower(cb)) return false;
        if (ca == 0) return true;
    }
}

}  // namespace

int mesh_category_from_name_007149d0(const char* name) noexcept {
    // 007149D2: the walk stops at the null pointer that terminates the table and
    // returns -1, so an unknown name is a miss rather than a fault.
    if (name == nullptr) return -1;
    for (std::size_t i = 0; i < kMeshCategoryCount; ++i) {
        if (equal_insensitive(name, kMeshCategoryNames[i])) return static_cast<int>(i);
    }
    return -1;
}

int geom_mesh_element_kind_00727310(const char* name) noexcept {
    const int kind = mesh_category_from_name_007149d0(name);
    // 007273B1 CMP EDI,0x7 / 007273B6 MOV EDI,0x9.
    return kind == 7 ? 9 : kind;
}

bool hull_segment_index_in_range(int segment_index) noexcept {
    return segment_index >= 0 && segment_index < kShipHullSegmentSlots;
}

}  // namespace bsp
