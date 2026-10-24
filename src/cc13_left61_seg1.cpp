// Packet cc13-left61: segment 1 leftovers (00413D10..0042A830). See
// include/bsp/cc13_left61.hpp and docs/CC13_LEFT61.md. Each body carries its
// address, the original ABI and every simplification next to it.

#include "bsp/cc13_x87_model.hpp"
#include "bsp/cc13_left61.hpp"

#include <cmath>

namespace bsp::cc13_left61 {
namespace {

// 00F8753C: the gameplay settings singleton pointer.
constexpr std::uint32_t kGameSettingsSingleton = 0x00f8753c;

// Release one reference-counted pointer slot: InterlockedDecrement of +4,
// vtable slot 0 at zero, then the slot is cleared (only when it was set).
void release_counted_slot(void*& slot, Left61Host& host) {
    auto* object = static_cast<std::uint8_t*>(slot);
    if (object == nullptr) return;
    if (host.interlocked_decrement_ce2220(reinterpret_cast<volatile long*>(object + 4)) == 0) {
        host.object_release_slot0(object);
    }
    slot = nullptr;
}

void release_pooled(void* data, std::uint32_t length, Left61Host& host) {
    if (data != nullptr) {
        void* pool = host.string_pool_00419cc0();
        host.string_pool_return_00bd1510(pool, data, length + 1, 1);
    }
}

}  // namespace

// 0041E490. The requested capacity is at least 1; only a larger one
// reallocates: operator new (00BF55BE) of capacity * 4 bytes, each old slot
// copied into a non-null new slot with InterlockedIncrement (import 00CE221C),
// then every old slot released, the old storage freed through _free and the
// capacity stored. The unwind action for a throwing copy is not modelled.
void counted_pointer_array_reserve_0041e490(Raw array, std::int32_t capacity, Left61Host& host) {
    if (capacity < 1) capacity = 1;
    if (array.at<std::int32_t>(8) >= capacity) return;
    auto* fresh = static_cast<void**>(host.operator_new_00bf55be(static_cast<std::uint32_t>(capacity) * 4));
    for (std::int32_t i = 0; i < array.at<std::int32_t>(4); ++i) {
        // The image tests the running slot pointer, so only slot 0 of a null
        // allocation is skipped.
        auto** slot = reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(fresh) + static_cast<std::uintptr_t>(i) * 4);
        if (slot == nullptr) continue;
        *slot = nullptr;
        void* value = static_cast<void**>(array.ptr(0))[i];
        if (value != nullptr) {
            *slot = value;
            host.interlocked_increment_ce221c(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(value) + 4));
        }
    }
    for (std::int32_t i = 0; i < array.at<std::int32_t>(4); ++i) {
        release_counted_slot(static_cast<void**>(array.ptr(0))[i], host);
    }
    host.free_00bf6989(array.ptr(0));
    array.ptr(0) = fresh;
    array.at<std::int32_t>(8) = capacity;
}

// 0041F370. Reserve (0041E490) when the capacity is below the count; new slots
// [count, n) are set to null (the slot-address null test of the image never
// fails); trailing slots are released from the back; count = n.
void counted_pointer_array_resize_0041f370(Raw array, std::int32_t count, Left61Host& host) {
    if (array.at<std::int32_t>(8) < count) counted_pointer_array_reserve_0041e490(array, count, host);
    for (std::int32_t i = array.at<std::int32_t>(4); i < count; ++i) {
        static_cast<void**>(array.ptr(0))[i] = nullptr;
    }
    while (count < array.at<std::int32_t>(4)) {
        array.at<std::int32_t>(4) -= 1;
        release_counted_slot(static_cast<void**>(array.ptr(0))[array.at<std::int32_t>(4)], host);
    }
    array.at<std::int32_t>(4) = count;
}

void counted_pointer_array_destroy_at30_00420660(Raw owner, Left61Host& host) {
    Raw array{owner.field(0x30)};
    counted_pointer_array_resize_0041f370(array, 0, host);
    host.free_00bf6989(array.ptr(0));
}

// 0041E780 / 004204A0: length + 1 bytes back to the pool when the data (+8) is set.
void native_string_at4_destroy_0041e780(Raw owner, Left61Host& host) {
    release_pooled(owner.ptr(8), owner.at<std::uint32_t>(4), host);
}

void native_string_at4_destroy_004204a0(Raw owner, Left61Host& host) {
    release_pooled(owner.ptr(8), owner.at<std::uint32_t>(4), host);
}

// 0041E7A0. The unwind states (2, 1, 0) only matter if a release throws and
// are not modelled.
void counted_record_destroy_0041e7a0(Raw record, Left61Host& host) {
    release_counted_slot(record.ptr(0x2c), host);
    release_counted_slot(record.ptr(0x28), host);
    release_counted_slot(record.ptr(0x24), host);
    release_pooled(record.ptr(4), record.at<std::uint32_t>(0), host);
}

// 00424C40. Double-checked: when [00F8753C] is null, the lifetime manager's
// critical section (+10h) is entered and its depth (+18h) raised (when the
// section exists); if still null, operator new(76Ch) + 00424A10 (null when new
// fails) is stored and registered through 00BD0C30 on a fresh 00415350 result;
// the depth drops and the section is left. The image keeps a lock-guard object
// (vtable 00CE37FC) on the stack whose unwind action is not modelled.
void* game_settings_get_singleton_00424c40(Left61Host& host) {
    std::uint32_t& singleton = host.global_u32(kGameSettingsSingleton);
    if (singleton == 0) {
        auto* section = static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(host.singleton_manager_00415350())}.ptr(0x10));
        if (section != nullptr) {
            host.enter_critical_section_ce2218(section);
            Raw{section}.at<std::int32_t>(0x18) += 1;
        }
        if (host.global_u32(kGameSettingsSingleton) == 0) {
            void* memory = host.operator_new_00bf681b(0x76c);
            void* settings = memory != nullptr ? host.game_settings_construct_00424a10(memory) : nullptr;
            host.global_u32(kGameSettingsSingleton) =
                static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(settings));
            void* manager = host.singleton_manager_00415350();
            host.singleton_register_00bd0c30(
                manager, reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(kGameSettingsSingleton))));
        }
        if (section != nullptr) {
            Raw{section}.at<std::int32_t>(0x18) -= 1;
            host.leave_critical_section_ce2210(section);
        }
    }
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(kGameSettingsSingleton)));
}

// 00427AC0: 0 when first is null, else the signed byte span SAR 3.
std::int32_t native_string_vector_size_00427ac0(Raw vector) {
    auto* first = static_cast<std::uint8_t*>(vector.ptr(4));
    if (first == nullptr) return 0;
    return static_cast<std::int32_t>(static_cast<std::uint8_t*>(vector.ptr(8)) - first) >> 3;
}

void vector3_subtract_to_00427c90(const float* a, float* out, const float* b) {
    out[0] = a[0] - b[0];
    out[1] = a[1] - b[1];
    out[2] = a[2] - b[2];
}

// 00427E30. The association is read from the bytes (D9 C1 DE CA D9 C2 DE CB D9
// C9 DE C2 DC C8 DE C1): (x*x + y*y) + z*z, stored to float. The decompiler
// prints z*z + x*x + y*y.
float vector3_length_squared_00427e30(const float* v) {
    const float xy = v[0] * v[0] + v[1] * v[1];
    return xy + v[2] * v[2];
}

float* entity_world_position_refreshed_00427eb0(Raw entity, Left61Host& host) {
    if (entity.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_00414db0(entity.p);
    return reinterpret_cast<float*>(entity.field(0xfc));
}

// 00428090. Traced from the bytes (00428090..0042822C). Only the FSTP dword stores round to
// float; everything between them stays on the x87 stack. The caller (00428420
// ProjectileTrace_HitLandingTroops, from 0084BF00 Projectile_TraceSegmentAndImpact) is not
// worker-only, so the model follows the main thread's 24-bit precision control: each
// operation rounds to a 24-bit significand with the extended exponent (x87_*24). Stored:
// d = end - start ([8],[C],[10]), f = start - centre ([14],[18],[1C]);
// a = (dx*dx + dy*dy) + dz*dz ([0]); half_b = (dy*fy + dx*fx) + dz*fz, then
// b = half_b + half_b ([4]); c_sum = (fx*fx + fy*fy) + fz*fz, then c = c_sum - r*r;
// disc = b*b - (a * 4.0 (double 00D7A328)) * c. False when 0 > disc; q = sqrt(disc)
// through _CIsqrt (00BF7030) stored to float; two_a = a + a stays on the stack;
// t1 = (-b - q) / two_a stored: false when t1 > 1, true unless 0 > t1 (COMISS);
// t2 = (q - b) / two_a stored: true unless 0 > t2. NaN compares take the "continue"
// side (JBE) everywhere, as written.
bool segment_intersects_sphere_00428090(const float* centre, const float* start, float radius,
                                        const float* end) {
    using namespace cc13x87;
    const float dx = end[0] - start[0];
    const float dy = end[1] - start[1];
    const float dz = end[2] - start[2];
    const float fx = start[0] - centre[0];
    const float fy = start[1] - centre[1];
    const float fz = start[2] - centre[2];
    const float a = static_cast<float>(x87_add24(x87_add24(x87_mul24(dx, dx), x87_mul24(dy, dy)), x87_mul24(dz, dz)));
    const float half_b =
        static_cast<float>(x87_add24(x87_add24(x87_mul24(dy, fy), x87_mul24(dx, fx)), x87_mul24(dz, fz)));
    const float b = static_cast<float>(x87_add24(half_b, half_b));
    const float c_sum = static_cast<float>(x87_add24(x87_add24(x87_mul24(fx, fx), x87_mul24(fy, fy)), x87_mul24(fz, fz)));
    const float c = static_cast<float>(x87_sub24(c_sum, x87_mul24(radius, radius)));
    const float disc = static_cast<float>(x87_sub24(x87_mul24(b, b), x87_mul24(x87_mul24(a, 4.0), c)));
    if (0.0f > disc) return false;
    const float q = static_cast<float>(std::sqrt(static_cast<double>(disc)));
    const double two_a = x87_add24(a, a);
    const float t1 = static_cast<float>(x87_div24(x87_sub24(-static_cast<double>(b), q), two_a));
    if (t1 > 1.0f) return false;
    if (!(0.0f > t1)) return true;
    const float t2 = static_cast<float>(x87_div24(x87_sub24(q, b), two_a));
    return !(0.0f > t2);
}

std::uint32_t plane_is_on_ground_0042a7e0(Raw plane) {
    const std::int32_t state = plane.at<std::int32_t>(0x900);
    return state == 4 || state == 5 ? 1u : 0u;
}

}  // namespace bsp::cc13_left61
