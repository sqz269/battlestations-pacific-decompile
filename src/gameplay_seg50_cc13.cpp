// Segment 50 reference models: barrel counts, ship command availability,
// control factors, the avoid-zone point test. Packet cc13-gpr50;
// docs/CC13_GPR_50.md. Read from the installed executable's bytes (Capstone
// disasm-raw); the listing decides every comparison. The barrel sums are FILD
// of an int plus a float accumulator, stored as a float each step: exact for
// integer totals below 2^24, so the iteration order does not change them.

#include "bsp/gameplay_seg50_cc13.hpp"

#include "bsp/entity_identity.hpp"        // compare_insensitive_00438e10
#include "bsp/native_string_compare.hpp"  // equal_native_string_header_00425850

#include <cstring>
#include <string.h>  // _stricmp

namespace bsp {
namespace gameplay_seg50_cc13 {

namespace {

inline float f32(double v) { return static_cast<float>(v); }

template <std::size_t N>
bool in_list(const std::int32_t (&list)[N], std::int32_t category) {
    for (std::int32_t c : list) {
        if (c == category) return true;
    }
    return false;
}

// The walk: categories in list order, devices of each category in turn.
template <std::size_t N, typename Add>
float sum_over(const std::int32_t (&list)[N], const std::vector<UnitDevice>& devices, Add add) {
    float s = 0.0f;
    for (std::int32_t c : list) {
        for (const UnitDevice& d : devices) {
            if (d.category == c) s = add(d, s);
        }
    }
    return s;
}

float add_barrels(const UnitDevice& d, float s) {
    return f32(static_cast<double>(d.barrels_448) + s);  // FILD [dev+448h]; FADD; FSTP
}

float add_live_barrels(const UnitDevice& d, float s) {
    const std::int32_t n = d.disabled_3b8 ? 0 : d.barrels_448;  // CMP [dev+3B8h],0
    return f32(static_cast<double>(n) + s);
}

struct NativeStringHeader {
    std::uint32_t length;
    const char* data;
};

// The engine test: __stricmp on the data when it is non-null (008131E6..00813204);
// the steering test: 00425850 on the native string (0081321B..00813227).
bool names_engine(const char* name) {
    return name != nullptr && _stricmp(name, "engine") == 0;  // 00CE4438
}
bool names_steering(const char* name, std::uint32_t length) {
    const NativeStringHeader h{length, name};
    return equal_native_string_header_00425850(&h, "steering");  // 00CE4408
}

}  // namespace

float total_artillery_barrels_00812310(const std::vector<UnitDevice>& devices) {
    return sum_over(kArtilleryCategories_00e0a4f8, devices, add_barrels);
}
float total_aa_barrels_008123a0(const std::vector<UnitDevice>& devices) {
    return sum_over(kAaCategories_00e0a510, devices, add_barrels);
}
float total_torpedo_barrels_00812430(const std::vector<UnitDevice>& devices) {
    return sum_over(kTorpedoCategories_00e0a520, devices, add_barrels);
}
float total_depth_charge_barrels_008124c0(const std::vector<UnitDevice>& devices) {
    return sum_over(kDepthChargeCategories_00e0a528, devices, add_barrels);
}
float live_artillery_barrels_00812550(const std::vector<UnitDevice>& devices) {
    return sum_over(kArtilleryCategories_00e0a4f8, devices, add_live_barrels);
}
float aa_mount_count_00812600(const std::vector<UnitDevice>& devices) {
    // 00812639..0081264F: FLD sum; FADD qword 1.0 [00D7A210]; FSTP float.
    return sum_over(kAaCategories_00e0a510, devices,
                    [](const UnitDevice&, float s) { return f32(static_cast<double>(s) + 1.0); });
}
float live_torpedo_barrels_00812680(const std::vector<UnitDevice>& devices) {
    return sum_over(kTorpedoCategories_00e0a520, devices, add_live_barrels);
}
float live_depth_charge_barrels_00812730(const std::vector<UnitDevice>& devices) {
    return sum_over(kDepthChargeCategories_00e0a528, devices, add_live_barrels);
}

// 00812B00-00812C87. Every compare is 00438E10 (0 on a match), in this order.
bool ship_command_available_00812b00(const char* command,
                                     const std::vector<ShipChildDevice>& children,
                                     ShipCommandHost& host) {
    auto is = [command](const char* literal) {
        return compare_insensitive_00438e10(command, literal) == 0;
    };
    if (is("settarget")) return true;                       // 00CF8B0C
    if (is("clearorders")) return true;                     // 00CF8AF4
    if (is("cleartarget")) return host.director_has_target();  // 00CF8B00, 00812B4A SETNE
    if (is("stop")) return true;                            // 00CFB5B8
    if (is("artillery")) {                                  // 00CFB3F4
        // 00812B76..00812BAF: the first child of kind 20h that is an artillery
        // kind (005459B0) and operational (00729F10).
        for (const ShipChildDevice& c : children) {
            if (c.kind_20 && c.artillery_kind_005459b0 && c.operational_00729f10) return true;
        }
        return false;
    }
    if (is("moveto")) return true;                          // 00CFB54C
    if (is("attackmove")) return true;                      // 00CECCA8
    if (is("stop")) return true;                            // 00CFB5B8 again (00812BE0), unreachable
    if (is("retreat")) return true;                         // 00CFB5D4
    if (is("repair")) return true;                          // 00CFD994
    if (is("ZigZag")) return true;                          // 00D09458
    if (is("torpedo")) {                                    // 00CFB414
        // 00812C40..00812C81: kind 20h, class function 7, operational.
        for (const ShipChildDevice& c : children) {
            if (c.kind_20 && c.class_function_80 == 7 && c.operational_00729f10) return true;
        }
        return false;
    }
    return false;                                           // 00812C83
}

// 008131E0-00813244 (RET 0Ch; the unused middle argument is not modelled).
bool set_control_factor_008131e0(ControlFactors& f, const char* name,
                                 std::uint32_t name_length, float value) {
    if (names_engine(name)) {
        f.engine_9d8 = value;     // 0081320D
        return true;
    }
    if (names_steering(name, name_length)) {
        f.steering_9dc = value;   // 00813232
        return true;
    }
    return false;
}

// 00813250-008132B0, the same tests; FLD/FSTP copies the float out.
bool get_control_factor_00813250(const ControlFactors& f, const char* name,
                                 std::uint32_t name_length, float& out) {
    if (names_engine(name)) {
        out = f.engine_9d8;
        return true;
    }
    if (names_steering(name, name_length)) {
        out = f.steering_9dc;
        return true;
    }
    return false;
}

// 00812C90: copies point x (+0h) and z (+8h), then 0082ADA0(0) and 004178F0.
bool point_outside_avoid_zones_00812c90(float x, float z, AvoidZoneHost& host) {
    return !host.zone_contains_004178f0(x, z);
}

}  // namespace gameplay_seg50_cc13
}  // namespace bsp
