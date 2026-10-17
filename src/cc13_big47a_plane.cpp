// Packet cc13-big47a (cc13_big47a marker). See include/bsp/cc13_big47a_plane.hpp
// and docs/CC13_BIG47A.md. Each function carries its address, original ABI,
// thread / precision assumption and the simplifications it makes.
// Reconstructed and build-tested only.

#include "bsp/cc13_big47a_plane.hpp"

#include <cmath>
#include <cstdint>

namespace bsp::cc13_big47a {

namespace {

float global_float(Big47aHost& host, std::uint32_t address) {
    return *static_cast<const float*>(host.global_address(address));
}

// The intrusive handle release every holder inlines: when set,
// InterlockedDecrement(p+4) == 0 -> p->vtable[0](), then the slot is cleared.
void release_handle(Big47aHost& host, void*& slot) {
    void* p = slot;
    if (p == nullptr) return;
    Raw obj = raw(p);
    if (host.interlocked_decrement_ce2220(reinterpret_cast<long*>(obj.field(4))) == 0)
        host.refcounted_vcall_destroy_00(obj);
    slot = nullptr;
}

// The handle assignment 007C4F5D..007C4F94 / 007C53C1..007C53F1: when the
// value differs, store it, retain the new one, release the old one.
void assign_handle(Big47aHost& host, void*& slot, void* value) {
    void* old = slot;
    if (old == value) return;
    slot = value;
    if (value != nullptr) host.interlocked_increment_ce221c(reinterpret_cast<long*>(raw(value).field(4)));
    if (old != nullptr && host.interlocked_decrement_ce2220(reinterpret_cast<long*>(raw(old).field(4))) == 0)
        host.refcounted_vcall_destroy_00(raw(old));
}

// The stop sequence 007C5115..007C5145 (and its vector copies): 00867B10 on
// the effect, effect+9 = 1, then release and clear the record's handle (the
// image stores the zero twice).
void stop_record_effect(Big47aHost& host, Raw record) {
    Raw effect = record.deref(0);
    host.point_effect_stop_children_00867b10(effect);
    effect.at<std::uint8_t>(9) = 1;
    release_handle(host, record.ptr(0));
    record.ptr(0) = nullptr;
}

// 007C4E62..007C4E8F / 007C526D..007C52D6: the identity matrix the creation
// builds on the stack (ESP+44h..80h); the translation row is filled later.
void identity_matrix(float (&m)[16]) {
    for (float& v : m) v = 0.0f;
    m[0] = kOne;
    m[5] = kOne;
    m[10] = kOne;
    m[15] = kOne;
}

// The inlined world-pose refresh 007C5869..007C5915 (and 007C59A6..007C5A53):
// when +C8h is clear, refresh the parent (+3Ch) with 00414DB0, compose
// 00413920(ECX = plane+74h; tmp, parent+CCh) or take +74h, copy the 16 floats
// to +CCh with FLD/FSTP float pairs, then +C8h = 1 and +10Ch = 0. Returns
// plane+CCh. The FLD/FSTP copy quiets a signalling NaN; the C++ copy does not.
const float* refresh_world_inline(Big47aHost& host, Raw plane) {
    if (plane.at<std::uint8_t>(0xC8) == 0) {
        Raw parent = plane.deref(0x3C);
        if (parent) host.entity_pose_refresh_world_00414db0(parent);
        parent = plane.deref(0x3C);
        float product[16];
        const float* source;
        if (parent)
            source = static_cast<const float*>(
                host.matrix_multiply_00413920(plane.sub(0x74), product, parent.field(0xCC)));
        else
            source = &plane.at<float>(0x74);
        float* world = &plane.at<float>(0xCC);
        for (int i = 0; i < 16; ++i) world[i] = source[i];
        plane.at<std::uint8_t>(0xC8) = 1;
        plane.at<std::uint8_t>(0x10C) = 0;
    }
    return &plane.at<float>(0xCC);
}

// Creates the record's effect at its point: 007C4E62..007C4FC1 (wing tips,
// definition class+5A4h) and 007C526D..007C541E (engines, definition from the
// class+564h vector). `read_definition` reads the handle the image then
// copy-constructs into the by-value argument (retain; the callee consumes it).
// The vector form's iterator checks sit between the steps; the caller passes
// them as `before_transform` / `before_assign` (no-ops for the wing-tip form).
template <class BeforeTransform, class ReadDefinition, class BeforeAssign>
void create_record_effect(Big47aHost& host, Raw plane, Raw record, BeforeTransform&& before_transform,
                          ReadDefinition&& read_definition, BeforeAssign&& before_assign) {
    float matrix[16];
    identity_matrix(matrix);
    if (plane.at<std::uint8_t>(0xC8) == 0) host.entity_pose_refresh_world_00414db0(plane);
    before_transform();
    float point[3];
    host.vector3_transform_affine_point_004142e0(&record.at<float>(4), point, plane.field(0xCC));
    matrix[12] = point[0];
    matrix[13] = point[1];
    matrix[14] = point[2];
    const std::uint32_t group = raw(host.global_pointer(kGameGlobalsPointer)).at<std::uint32_t>(0x19EC);
    void* definition = read_definition();
    if (definition != nullptr) host.interlocked_increment_ce221c(reinterpret_cast<long*>(raw(definition).field(4)));
    void* created = nullptr;  // ESP+14h / ESP+20h; 00868420 writes it
    void** result = host.point_effect_create_from_matrix_00868420(&created, group, definition, matrix, 0, 0);
    before_assign();
    assign_handle(host, record.ptr(0), *result);
    release_handle(host, created);
}

}  // namespace

// ============================================================================
// 007C4DD0 BSP_Plane_UpdateEngineEffects
// __thiscall(ECX = plane; float dt at [ESP+4]), RET 4. SEH frame (handler
// 00C8C7F2); the EH states 0..5 only cover the temporary handles, and their
// unwind releases are not modelled (no exception path is reconstructed).
// Thread: called from 007C6820 (plane tick, unit fixed step) and 0089E670 (the
// Lua SetTipEffect binding, main thread, with dt = 0.0 or 1.0). Precision:
// the one multi-op x87 chain (the wing-tip level, 007C503C..007C5058) is
// modelled at 24-bit PC (each op rounded to float); at 53-bit PC the chain is
// float(double(1/(sum-base)) * double(y-base)) and can differ by one ulp.
// Single x87 ops (v - 0.5, v +/- dt) round identically at either precision.
// No shared-RNG (00BD2F10) draws.
// ============================================================================
void plane_update_engine_effects_007c4dd0(Big47aHost& host, Raw plane, float dt) {
    // --- wing-tip records: class+5A0h of them at plane+A5Ch (10h each) ------
    if (plane.deref(0x538).at<std::int32_t>(0x5A0) != 0) {                         // 007C4DF8
        if (plane.at<std::uint8_t>(0xC8) == 0) host.entity_pose_refresh_world_00414db0(plane);  // 007C4E10
        const float base = global_float(host, kTipEffectAltitudeBase);                 // 007C4E18
        if (plane.at<float>(0x100) > base) {                                           // 007C4E2E JBE -> stop
            for (std::int32_t i = 0; i < plane.deref(0x538).at<std::int32_t>(0x5A0); ++i) {
                Raw record = plane.sub(0xA5C + 0x10 * static_cast<std::size_t>(i));
                if (record.ptr(0) == nullptr) {                                         // 007C4E5C
                    create_record_effect(
                        host, plane, record, [] {},
                        [&]() -> void* { return plane.deref(0x538).ptr(0x5A4); },      // 007C4F38
                        [] {});
                }
                if (record.ptr(0) != nullptr) {                                         // 007C4FD4
                    Raw effect = record.deref(0);
                    effect.at<std::uint8_t>(9) = 0;                                     // 007C4FE2
                    void* child = nullptr;
                    host.point_effect_find_child_by_type_00866e60(effect, &child, 3);   // 007C4FE9
                    if (child != nullptr) {                                             // 007C5008
                        // 007C5011: FLD [E0B048]; FLD [E0B044]; FLD ST0; FADDP ST2; FSTP float sum.
                        const float span_value = global_float(host, kTipEffectAltitudeSpan);
                        float tip_base = global_float(host, kTipEffectAltitudeBase);
                        const float sum = span_value + tip_base;
                        if (plane.at<std::uint8_t>(0xC8) == 0) {
                            host.entity_pose_refresh_world_00414db0(plane);              // 007C502D
                            tip_base = global_float(host, kTipEffectAltitudeBase);       // 007C5032 reload
                        }
                        // 007C503C..007C5058 at 24-bit PC: (1 / (sum - base)) * (y - base) + 0.
                        const float span = sum - tip_base;
                        const float inverse = 1.0f / span;
                        const float above = plane.at<float>(0x100) - tip_base;
                        float t = inverse * above;
                        t = t + 0.0f;
                        float level;
                        if (0.0f > t)
                            level = 0.0f;                                               // 007C5068
                        else if (t > kOne)
                            level = kOne;                                               // 007C50EA
                        else
                            level = t;                                                  // NaN keeps t (JBE)
                        host.effect_child_set_level_vcall_14(raw(child), level);        // 007C507E
                    }
                    release_handle(host, child);                                        // 007C5084
                }
            }
        } else {
            for (std::int32_t i = 0; i < plane.deref(0x538).at<std::int32_t>(0x5A0); ++i) {  // 007C50F5
                Raw record = plane.sub(0xA5C + 0x10 * static_cast<std::size_t>(i));
                if (record.ptr(0) != nullptr) stop_record_effect(host, record);        // 007C5115
            }
        }
    }

    // --- engine records: the vector {A4Ch container, A50h first, A54h last} --
    auto first = [&] { return plane.at<std::uint8_t*>(0xA50); };
    auto last = [&] { return plane.at<std::uint8_t*>(0xA54); };
    auto check_deref = [&](std::uint8_t* it) {
        if (!(it < last())) host.invalid_parameter_00bf6713();
    };
    auto check_range = [&] {
        if (first() > last()) host.invalid_parameter_00bf6713();
    };
    auto check_container = [&] {
        // LEA EAX,[EBX+A4Ch]; TEST EAX,EAX; JZ -> 00BF6713 (the iterator's container).
        if (plane.field(0xA4C) == nullptr) host.invalid_parameter_00bf6713();
    };

    bool run;
    if (plane.at<std::uint8_t>(0x61) != 0 && plane.at<std::uint8_t>(0x96A) != 0) {  // 007C515F
        run = true;
    } else {
        const float speed = plane.at<float>(0xB1C);                                     // 007C516F
        if (kEngineRunMinSpeed > static_cast<double>(speed)) {                         // 007C5187 JA
            run = false;
        } else {
            const float throttle = plane.at<float>(0x9E8);                              // 007C518D
            const float magnitude = throttle > 0.0f ? throttle : kNegativeZero - throttle;
            if (kEngineRunMinThrottle > static_cast<double>(magnitude))                // 007C51C2 JA
                run = false;
            else
                run = plane.at<std::uint8_t>(0x5D) == 0;                                // 007C51C8
        }
    }

    if (run) {
        // 007C51D2: (re)create missing engine effects and drive the level to 0.5.
        std::uint8_t* it = first();
        if (it > last()) host.invalid_parameter_00bf6713();
        std::uint32_t index = 0;        // ESP+24h
        std::uint32_t byte_offset = 0;  // ESP+14h
        for (;;) {
            std::uint8_t* const end = last();                                           // 007C5200
            check_range();
            check_container();                                                          // 007C5216
            if (it == end) break;                                                       // 007C522E
            check_container();                                                          // 007C5234
            check_deref(it);                                                            // 007C5246
            Raw record{it};
            if (record.ptr(0) == nullptr) {                                             // 007C5267
                create_record_effect(
                    host, plane, record, [&] { check_deref(it); },                      // 007C52E8
                    [&]() -> void* {
                        // 007C533E..007C5380: the class+564h vector of 10h records,
                        // checked (begin != 0, index < (end - begin) >> 4 unsigned),
                        // then [begin + offset] read even after a failed check.
                        Raw cls = plane.deref(0x538);
                        const std::uint8_t* begin = cls.at<std::uint8_t*>(0x564);
                        if (begin == nullptr ||
                            !(index < static_cast<std::uint32_t>(
                                          static_cast<std::int32_t>(cls.at<std::uint8_t*>(0x568) - begin) >> 4)))
                            host.invalid_parameter_00bf6713();
                        begin = cls.at<std::uint8_t*>(0x564);
                        return *reinterpret_cast<void* const*>(begin + byte_offset);
                    },
                    [&] { check_deref(it); });                                          // 007C53A3
                check_deref(it);                                                        // 007C5426
                if (record.ptr(0) != nullptr) {                                         // 007C5445
                    check_deref(it);                                                    // 007C544B
                    record.deref(0).at<std::uint8_t>(9) = 0;                            // 007C5460
                    check_deref(it);                                                    // 007C5464
                    void* child = nullptr;
                    host.point_effect_find_child_by_type_00866e60(record.deref(0), &child, 3);  // 007C5477
                    if (child != nullptr) host.effect_child_set_level_vcall_14(raw(child), 0.0f);  // 007C54A3 FLDZ
                    release_handle(host, child);                                        // 007C54A9
                }
            } else {
                check_deref(it);                                                        // 007C54E1
                void* child = nullptr;
                host.point_effect_find_child_by_type_00866e60(record.deref(0), &child, 3);  // 007C54FD
                if (child != nullptr) {                                                 // 007C551C
                    const float v = raw(child).deref(0x1C).at<float>(0x19C);           // 007C5525
                    const float gap = std::fabs(static_cast<float>(static_cast<double>(v) - kEngineRunLevel));
                    float level;
                    if (dt > gap)                                                       // 007C5563 JBE
                        level = kEngineRunTarget;
                    else if (kEngineRunTarget > v)                                      // 007C5585 JBE
                        level = v + dt;
                    else
                        level = v - dt;
                    host.effect_child_set_level_vcall_14(raw(child), level);           // 007C557C / 007C559E
                } else {
                    check_deref(it);                                                    // 007C55A2
                    Raw effect = record.deref(0);
                    host.point_effect_stop_children_00867b10(effect);                   // 007C55B9
                    effect.at<std::uint8_t>(9) = 1;
                    check_deref(it);                                                    // 007C55C2
                    release_handle(host, record.ptr(0));
                    record.ptr(0) = nullptr;
                }
                release_handle(host, child);                                            // 007C55F4
            }
            check_deref(it);                                                            // 007C5627
            ++index;
            it += 0x10;
            byte_offset += 0x10;
        }
    } else {
        // 007C564F: fade the engine effects to 0 and stop each one that reaches it.
        std::uint8_t* it = first();
        if (it > last()) host.invalid_parameter_00bf6713();
        for (;;) {
            std::uint8_t* const end = last();                                           // 007C5670
            check_range();
            if (it == end) break;                                                       // 007C5694 (container compare only)
            check_deref(it);                                                            // 007C569A
            Raw record{it};
            if (record.ptr(0) != nullptr) {                                             // 007C56B3
                check_deref(it);                                                        // 007C56B9
                void* child = nullptr;
                host.point_effect_find_child_by_type_00866e60(record.deref(0), &child, 3);  // 007C56CF
                bool stop = true;
                if (child != nullptr) {                                                 // 007C56EA
                    const float v = raw(child).deref(0x1C).at<float>(0x19C);           // 007C56F3
                    const float gap = std::fabs(static_cast<float>(static_cast<double>(v) - kEngineFadeLevel));
                    float level;
                    if (dt > gap)                                                       // 007C572C JBE
                        level = 0.0f;
                    else if (0.0f > v)                                                  // 007C573F JBE
                        level = v + dt;
                    else
                        level = v - dt;
                    host.effect_child_set_level_vcall_14(raw(child), level);           // 007C5758
                    stop = level == kEngineFadeStop;                                    // 007C5760 UCOMISS / JP
                }
                if (stop) {
                    check_deref(it);                                                    // 007C576D
                    Raw effect = record.deref(0);
                    host.point_effect_stop_children_00867b10(effect);                   // 007C577E
                    effect.at<std::uint8_t>(9) = 1;
                    check_deref(it);                                                    // 007C5787
                    release_handle(host, record.ptr(0));
                    record.ptr(0) = nullptr;
                }
                release_handle(host, child);                                            // 007C57BC
            }
            check_deref(it);                                                            // 007C57EF
            it += 0x10;
        }
    }

    // --- 007C5804: move every live engine effect to its world point ---------
    {
        std::uint8_t* it = first();
        if (it > last()) host.invalid_parameter_00bf6713();
        for (;;) {
            std::uint8_t* const end = last();                                           // 007C5820
            check_range();
            if (it == end) break;                                                       // 007C5844
            check_deref(it);                                                            // 007C584A
            Raw record{it};
            if (record.ptr(0) != nullptr) {                                             // 007C5863
                const float* world = refresh_world_inline(host, plane);
                check_deref(it);                                                        // 007C591C
                float point[3];
                host.vector3_transform_affine_point_004142e0(&record.at<float>(4), point, world);
                const float copy[3] = {point[0], point[1], point[2]};                   // ESP+38h
                check_deref(it);                                                        // 007C5937
                host.point_effect_set_world_point_004842c0(record.deref(0), copy);      // 007C596F
            }
            check_deref(it);                                                            // 007C5974
            it += 0x10;
        }
    }

    // --- 007C5989: the four fixed wing-tip records, whatever the class count --
    for (int k = 0; k < 4; ++k) {
        Raw record = plane.sub(0xA5C + 0x10 * static_cast<std::size_t>(k));
        if (record.ptr(0) != nullptr) {                                                 // 007C59A0
            const float* world = refresh_world_inline(host, plane);
            float point[3];
            host.vector3_transform_affine_point_004142e0(&record.at<float>(4), point, world);  // 007C5A63
            const float copy[3] = {point[0], point[1], point[2]};                       // ESP+2Ch
            host.point_effect_set_world_point_004842c0(record.deref(0), copy);          // 007C5A93
        }
    }
}

// ============================================================================
// 007BEEE0 BSP_PlaneTickElement_CommitStepPose
// __thiscall(ECX = tick element), RET 0; plane tick-element vtable slot +0Ch
// (00D00038, 00D002D0, 00D05EE8, 00D06600, 00D068E8, 00D0BA48, 00D19CF0,
// 00D19FC8, 00D1A2A0). Offsets are element-relative as in the image; with the
// element at plane+310h they are plane+3Ch (parent), +6A4h (point), +918h
// (published point), +674h / +924h / +74h / +4E0h (matrices), +4D8h (flag).
// Thread: unit fixed step. No arithmetic beyond the callees (the no-parent
// copy is FLD/FSTP float, which quiets a signalling NaN; the C++ copy does not).
// The body ends at 007BEFDC; the 007BEFE0..007C0686 and 007C0690..007C0796
// code after it is two separate functions Ghidra has not defined (docs/CC13_BIG47A.md).
// ============================================================================
void plane_tick_element_commit_step_pose_007beee0(Big47aHost& host, Raw element) {
    auto at_element = [&](std::ptrdiff_t off) { return element.p + off; };
    auto parent_of = [&] { return raw(*reinterpret_cast<void**>(at_element(-0x2D4))); };
    float* const point = reinterpret_cast<float*>(at_element(0x394));
    float* const published = reinterpret_cast<float*>(at_element(0x608));
    Raw const local_matrix{at_element(0x364)};
    float product[16];  // ESP+14h
    const void* source;
    Raw parent = parent_of();                                                           // 007BEEE7
    if (parent) {
        if (parent.at<std::uint8_t>(0xC8) == 0) host.entity_pose_refresh_world_00414db0(parent);  // 007BEF00
        float transformed[3];
        host.vector3_transform_affine_point_004142e0(point, transformed, parent.field(0xCC));  // 007BEF17
        published[0] = transformed[0];
        published[1] = transformed[1];
        published[2] = transformed[2];
        parent = parent_of();                                                           // 007BEF46
        if (parent.at<std::uint8_t>(0xC8) == 0) host.entity_pose_refresh_world_00414db0(parent);  // 007BEF57
        source = host.matrix_multiply_00413920(local_matrix, product, parent.field(0xCC));  // 007BEF70
    } else {
        published[0] = point[0];                                                        // 007BEF78
        published[1] = point[1];
        published[2] = point[2];
        source = local_matrix.p;
    }
    host.matrix_assign_004134f0(Raw{at_element(0x614)}, source);                         // 007BEFA9
    if (*at_element(0x1C8) == 0)                                                        // 007BEFAE
        host.matrix_assign_004134f0(local_matrix, at_element(-0x29C));                  // 007BEFC0
    else
        host.matrix_assign_004134f0(local_matrix, at_element(0x1D0));                   // 007BEFD2
}

// ============================================================================
// 007BE9B0 BSP_Plane_ApplyPointEventLocal
// __thiscall(ECX = target; float a[3] at [ESP+4], b[3] at +10h, c[3] at +1Ch,
// float time at +28h), RET 28h. Only caller 007C0CE0 in 007C0C60 (plane vtable
// +4Ch) with ECX = plane+84Ch. The target begins with a 4x4 matrix whose rows
// +10h / +20h / +30h receive c / b / a; +40h re-seed flag, +44h speed, four
// rate limiters at +48h / +60h / +78h / +90h (007BAE90 records), +A0h / +A4h,
// +A8h the last event time, +ACh the owner plane (class at owner+538h).
// Thread: the 007C0C60 caller's (not established). Precision: every x87 result
// is stored to a float after a single operation (the three limit products are
// float * 3.0, exact before the store), so 24- and 53-bit PC give the same
// floats. No shared-RNG (00BD2F10) draws.
// ============================================================================
void plane_apply_point_event_local_007be9b0(Big47aHost& host, Raw target, const float (&a)[3],
                                            const float (&b)[3], const float (&c)[3], float time) {
    const float last = target.at<float>(0xA8);                                          // 007BE9C3
    if (last > 0.0f && last > time) {                                                   // 007BE9D6 / 007BE9DE JBE
        target.at<float>(0xA8) = kMinusOne;                                             // 007BE9E8
        target.at<std::uint8_t>(0x40) = 1;
    }
    const float previous = target.at<float>(0xA8);                                      // 007BE9F3
    target.at<float>(0xA8) = time;                                                      // 007BEA0D
    if (0.0f > previous) {                                                              // 007BEA15 JBE -> advance
        // 007BEA1B: re-seed. +44h = 0, +40h = 1, then the class limits.
        target.at<float>(0x44) = 0.0f;
        target.at<std::uint8_t>(0x40) = 1;
        Raw owner = target.deref(0xAC);
        if (owner && owner.ptr(0x538) != nullptr) {                                     // 007BEA2D / 007BEA3A
            target.at<float>(0xA0) = kPointEventA0;
            target.at<float>(0xA4) = kPointEventA4;
            const float v1c4 = target.deref(0xAC).deref(0x538).at<float>(0x1C4);        // 007BEA6C
            target.at<float>(0x58) = v1c4;
            target.at<float>(0x5C) = static_cast<float>(static_cast<double>(v1c4) * kPointEventLimitScale);
            const float v1c0 = target.deref(0xAC).deref(0x538).at<float>(0x1C0);        // 007BEA9C
            target.at<float>(0x70) = v1c0;
            target.at<float>(0x74) = static_cast<float>(kPointEventLimitScale * static_cast<double>(v1c0));
            const float v1bc = target.deref(0xAC).deref(0x538).at<float>(0x1BC);        // 007BEAC4
            target.at<float>(0x88) = v1bc;
            target.at<float>(0x8C) = static_cast<float>(kPointEventLimitScale * static_cast<double>(v1bc));
        }
    } else {
        const float step = time - previous;                                             // 007BEAE9 FSUB float
        if (step > 0.0f) {                                                              // 007BEAFB JBE -> end
            float local_b[3];  // ESP+18h: first the a-delta, then b in the local frame
            local_b[0] = target.at<float>(0x30) - a[0];                                 // 007BEB01
            local_b[1] = target.at<float>(0x34) - a[1];
            local_b[2] = target.at<float>(0x38) - a[2];
            const float length = host.vector3_length_threshold_0042b2f0(local_b);       // 007BEB2F
            target.at<float>(0x44) = length / step;                                     // 007BEB34 FDIV, FSTP float
            const float* origin = static_cast<const float*>(host.global_address(kPointEventResetOrigin));
            target.at<float>(0x30) = origin[0];                                         // 007BEB44
            target.at<float>(0x34) = origin[1];
            target.at<float>(0x38) = origin[2];
            alignas(16) float inverse[16];  // ESP+B0h
            host.matrix_build_affine_inverse_00b63d50(raw(inverse), target);            // 007BEB6B
            host.vector3_transform_affine_point_004142e0(b, local_b, inverse);          // 007BEB84
            float local_c[3];  // ESP+24h
            host.vector3_transform_affine_point_004142e0(c, local_c, inverse);          // 007BEB9D
            float identity[16];  // ESP+30h
            for (float& v : identity) v = 0.0f;
            identity[0] = kOne;
            identity[5] = kOne;
            identity[10] = kOne;
            identity[15] = kOne;
            alignas(16) float basis[16];  // ESP+70h
            host.matrix_assign_004134f0(raw(basis), identity);                          // 007BEC16
            basis[8] = local_b[0];                                                      // row +20h
            basis[9] = local_b[1];
            basis[10] = local_b[2];
            basis[4] = local_c[0];                                                      // row +10h
            basis[5] = local_c[1];
            basis[6] = local_c[2];
            host.matrix_orthonormalize_basis_rows_0085dc80(raw(basis));                 // 007BEC79
            float angle_edx = 0.0f;   // ESP+0Ch (EDX)
            float angle_arg1 = 0.0f;  // ESP+10h (first stack argument)
            float angle_arg2 = 0.0f;  // ESP+08h (second stack argument)
            host.matrix_extract_normalized_basis_angles_0042d2e0(basis, &angle_edx, &angle_arg1, &angle_arg2);  // 007BEC90
            // 007BEC95..007BECCB: each angle divided by the step, stored as float.
            angle_edx = angle_edx / step;
            angle_arg1 = angle_arg1 / step;
            angle_arg2 = angle_arg2 / step;
            Raw cls = target.deref(0xAC).deref(0x538);                                  // 007BECAD
            // Level = (floor > r) ? floor : ((r > 1) ? 1 : r); a NaN r passes through
            // (every compare is JBE / JA ordered).
            const float speed_ratio = target.at<float>(0x44) / cls.at<float>(0x188);    // 007BECCF
            float speed_level;
            if (kPointEventSpeedFloorTest > static_cast<double>(speed_ratio))           // 007BECEA
                speed_level = kPointEventSpeedFloor;
            else
                speed_level = speed_ratio > kOne ? kOne : speed_ratio;
            const float arg2_ratio = angle_arg2 / cls.at<float>(0x1A8);                 // 007BED13
            float arg2_level;
            if (kMinusOne > arg2_ratio)
                arg2_level = kMinusOne;
            else
                arg2_level = arg2_ratio > kOne ? kOne : arg2_ratio;
            const float edx_ratio = angle_edx / cls.at<float>(0x1AC);                   // 007BED54
            float edx_level;
            if (kMinusOne > edx_ratio)
                edx_level = kMinusOne;
            else
                edx_level = edx_ratio > kOne ? kOne : edx_ratio;
            const float arg1_ratio = angle_arg1 / cls.at<float>(0x1B0);                 // 007BED8C
            float arg1_level;
            if (kMinusOne > arg1_ratio)
                arg1_level = kMinusOne;
            else
                arg1_level = arg1_ratio > kOne ? kOne : arg1_ratio;                     // 007BEDB5 JA
            if (target.at<std::uint8_t>(0x40) != 0) {                                   // 007BEDBA
                target.at<float>(0x48) = arg1_level;
                target.at<float>(0x60) = edx_level;
                target.at<float>(0x78) = arg2_level;
                target.at<float>(0x90) = speed_level;
                target.at<std::uint8_t>(0x40) = 0;
            } else {
                cc13_seg47::rate_limiter_step_007bae90(host, target.sub(0x48), arg1_level, step);   // 007BEDF6
                cc13_seg47::rate_limiter_step_007bae90(host, target.sub(0x60), edx_level, step);    // 007BEE10
                cc13_seg47::rate_limiter_step_007bae90(host, target.sub(0x78), arg2_level, step);   // 007BEE2A
                cc13_seg47::rate_limiter_step_007bae90(host, target.sub(0x90), speed_level, step);  // 007BEE47
            }
        }
    }
    // 007BEE4C: store the three points as rows, then 0085DC80 on the target.
    target.at<float>(0x30) = a[0];
    target.at<float>(0x34) = a[1];
    target.at<float>(0x38) = a[2];
    target.at<float>(0x20) = b[0];
    target.at<float>(0x24) = b[1];
    target.at<float>(0x28) = b[2];
    target.at<float>(0x10) = c[0];
    target.at<float>(0x14) = c[1];
    target.at<float>(0x18) = c[2];
    host.matrix_orthonormalize_basis_rows_0085dc80(target);                             // 007BEECC
}

}  // namespace bsp::cc13_big47a
