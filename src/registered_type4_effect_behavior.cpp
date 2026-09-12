#include "bsp/registered_type4_effect_behavior.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Registered type4 effect behavior requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> volatile T& field(void* actual, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(static_cast<std::byte*>(actual) + offset);
}
template<class T> const volatile T& field(const void* actual, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(actual) + offset);
}
PointEffectInstanceStorage& subject(NativeRegisteredType4EffectStorage& event) noexcept {
    return *static_cast<PointEffectInstanceStorage*>(field<void*>(&event, 0x10));
}
RegisteredType4TracerView tracer(NativeRegisteredType4EffectStorage& event) noexcept {
    return {field<void*>(&event, 0x34)};
}
// The x87 comparisons retain native unordered branching (JBE includes NaN).
__declspec(naked) bool __fastcall above(const float*, const float*) noexcept {
    __asm {
        fld dword ptr [edx]
        fld dword ptr [ecx]
        fcomip st(0),st(1)
        fstp st(0)
        seta al
        ret
    }
}
__declspec(naked) bool __fastcall at_least(const float*, const float*) noexcept {
    __asm {
        fld dword ptr [edx]
        fld dword ptr [ecx]
        fcomip st(0),st(1)
        fstp st(0)
        setae al
        ret
    }
}
bool above(float left, float right) noexcept { return above(&left, &right); }
bool at_least(float left, float right) noexcept { return at_least(&left, &right); }
__declspec(naked) void __fastcall add(float*, const float*) noexcept {
    __asm {
        fld dword ptr [ecx]
        fadd dword ptr [edx]
        fstp dword ptr [ecx]
        ret
    }
}
__declspec(naked) void __fastcall subtract(float*, const float*) noexcept {
    __asm {
        fld dword ptr [ecx]
        fsub dword ptr [edx]
        fstp dword ptr [ecx]
        ret
    }
}
float product(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fmul right
        fstp result
    }
    return result;
}
float argument_spill(float value) noexcept {
    // Native FLD/FSTP argument marshalling, including masked signaling NaNs.
    // Direct MOVSS setters themselves retain the incoming word unchanged.
    __asm {
        fld value
        fstp value
    }
    return value;
}
float length_argument(float native_length, float scale) noexcept {
    //872935..872959; do not reduce length/(1/scale) to multiplication.
    const double half = 0.5; // immutable D7A280, 3FE0000000000000
    float result;
    __asm {
        fld native_length
        fld scale
        fld1
        fdivrp st(1),st(0)
        fdivp st(1),st(0)
        fadd half
        fstp result
    }
    return result;
}
float absolute_bits(float value) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    bits &= 0x7fffffffu;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
void copy_world_words(CameraMatrix& output, const CameraMatrix& input) noexcept {
    // Native REP MOVSD, not the x87 canonical matrix-copy helper.
    std::memcpy(output.data(), input.data(), sizeof(output));
}
} // namespace

void deactivate_registered_type4_tracer_00ba9820(RegisteredType4TracerView view) noexcept {
    field<std::uint8_t>(view.actual_owner, 0x200) = 0;
    field<float>(view.actual_owner, 0x7a8) = 1.0f;
    void* const linked = field<void*>(view.actual_owner, 0x1b8);
    if (linked) field<float>(linked, 0x24) = 0.1f;
}
void set_registered_type4_tracer_reciprocal_00ba9900(RegisteredType4TracerView view,
    float value) noexcept {
    float result;
    __asm {
        fld value
        fld1
        fdivrp st(1),st(0)
        fstp result
    }
    field<float>(view.actual_owner, 0x1ec) = result;
}
void set_registered_type4_tracer_texture_rate_00ba9920(RegisteredType4TracerView view,
    float value) noexcept { field<float>(view.actual_owner, 0x1f0) = value; }
void set_registered_type4_tracer_alpha_scale_00ba9940(RegisteredType4TracerView view,
    float value) noexcept { field<float>(view.actual_owner, 0x1f4) = value; }
void set_registered_type4_tracer_width_scale_00ba99c0(RegisteredType4TracerView view,
    float value) noexcept { field<float>(view.actual_owner, 0x1e0) = value; }
void set_registered_type4_tracer_width_scaler_00ba99e0(RegisteredType4TracerView view,
    float value) noexcept { field<float>(view.actual_owner, 0x1e4) = value; }
void set_registered_type4_tracer_width_offset_00ba9a00(RegisteredType4TracerView view,
    float value) noexcept { field<float>(view.actual_owner, 0x1e8) = value; }
void set_registered_type4_tracer_bone_length_00ba9a20(RegisteredType4TracerView view,
    float value) noexcept { field<float>(view.actual_owner, 0x1d8) = value; }
void set_registered_type4_tracer_height_scale_00ba9a40(RegisteredType4TracerView view,
    float value) noexcept { field<float>(view.actual_owner, 0x1fc) = value; }
void set_registered_type4_tracer_local_space_00ba9a60(RegisteredType4TracerView view,
    std::uint8_t value) noexcept { field<std::uint8_t>(view.actual_owner, 0x202) = value; }
void set_registered_type4_tracer_fade_in_00ba9a70(RegisteredType4TracerView view,
    float value) noexcept { field<float>(view.actual_owner, 0x210) = value; }
void set_registered_type4_tracer_width_endpoints_00ba9a90(RegisteredType4TracerView view,
    float at_zero, float at_one) noexcept {
    field<float>(view.actual_owner, 0x184) = at_zero;
    field<float>(view.actual_owner, 0x188) = at_one;
}

std::uint8_t complete_registered_type4_effect_00872020(
    const NativeRegisteredType4EffectStorage& event) noexcept {
    const volatile auto& actual = event;
    if (actual.state_1c != 0) return 0;
    const void* const definition = actual.prefix_00.borrowed_14;
    if (above(actual.value_28, field<float>(definition, 0x80))) return 1;
    const void* const current = actual.node_34;
    return static_cast<std::uint8_t>(current && actual.prefix_00.active_0c == 0 &&
        field<std::uint8_t>(current, 0x201) == 0);
}

void deactivate_registered_type4_effect_00872060(NativeRegisteredType4EffectStorage& event) noexcept {
    volatile auto& actual = event;
    actual.state_1c = 0;
    void* const current = actual.node_34;
    if (current) deactivate_registered_type4_tracer_00ba9820({current});
}

void update_registered_type4_effect_00872790(NativeRegisteredType4EffectStorage& event,
    float delta, void* actual_reference, RegisteredType4EffectBehaviorBindings bindings) {
    (void)actual_reference; // Second native word is consumed by RET8 but unread.
    volatile auto& actual = event;
    auto& initial_subject = subject(event);
    const void* const definition = actual.prefix_00.borrowed_14;
    add(&event.value_24, &delta);
    std::array<float, 3> captured_vector;
    std::memcpy(captured_vector.data(), initial_subject.fields_5c.data() + 3,
        sizeof(captured_vector));
    const float speed = initial_subject.fields_50[1];
    // D7A3A0 double3FB99999A0000000 is exactly the promoted float0.1.
    constexpr float direction_threshold = 0.1f;
    if (actual.state_1c != 0 && actual.prefix_00.active_0c != 0) {
        if (!above(speed, field<float>(definition, 0x7c))) {
            actual.value_2c = 0.0f;
        } else if (!above(speed, direction_threshold)) {
            if (field<std::uint32_t>(definition, 0x24) == 0) actual.value_2c = 0.0f;
            else {
                if (above(actual.value_2c, 0.0f)) actual.value_2c = 0.0f;
                subtract(&event.value_2c, &delta);
            }
        } else {
            if (field<std::uint32_t>(definition, 0x20) == 0) actual.value_2c = 0.0f;
            else {
                if (above(0.0f, actual.value_2c)) actual.value_2c = 0.0f;
                add(&event.value_2c, &delta);
            }
        }
        if (above(absolute_bits(actual.value_2c), field<float>(definition, 0x78))) {
            actual.untouched_20 = above(actual.value_2c, 0.0f) ? 1u : 0u;
            actual.state_1c = 0;
            auto& section = bindings.actual_lock_00f87684;
            singleton_enter_critical_section(section);
            ++section.recursion_18;
            void* const raw = bindings.tracers.allocate_00bac660();
            void* constructed = nullptr;
            if (raw) {
                try {
                    //8728DC..87290D: preserve current global load and native
                    //argument evaluation order across the actual constructor.
                    void* const argument_a4 = field<void*>(definition, 0xa4);
                    void* const game = bindings.actual_game_00e188a8;
                    void* const owner_19f0 = field<void*>(game, 0x19f0);
                    void* const root_19ec = field<void*>(game, 0x19ec);
                    void* const owner_a8 = field<void*>(owner_19f0, 0xa8);
                    void* const texture_9c = field<void*>(definition, 0x9c);
                    void* const texture_98 = field<void*>(definition, 0x98);
                    constructed = bindings.tracers.construct_00bad6f0(raw,
                        owner_a8, root_19ec, texture_98, texture_9c, argument_a4);
                } catch (...) {
                    // DC8190 state0 -> C96170 -> BAC2B0. No outer unlock.
                    bindings.tracers.return_slot_00bac2b0(raw);
                    throw;
                }
            }
            actual.node_34 = constructed;
            --section.recursion_18;
            singleton_leave_critical_section(section);
            // Native allocation-null path continues into these required owner
            //writes. No null-allocation success or substitute owner is added.
            const float length = field<float>(definition, 0xb8);
            const float scale = subject(event).fields_50[0];
            set_registered_type4_tracer_reciprocal_00ba9900(tracer(event),
                length_argument(length, scale));
            set_registered_type4_tracer_local_space_00ba9a60(tracer(event),
                field<std::uint8_t>(definition, 0xa0));
            set_registered_type4_tracer_texture_rate_00ba9920(tracer(event), argument_spill(field<float>(definition, 0x44)));
            set_registered_type4_tracer_alpha_scale_00ba9940(tracer(event), argument_spill(field<float>(definition, 0x58)));
            set_registered_type4_tracer_width_scale_00ba99c0(tracer(event), argument_spill(field<float>(definition, 0x5c)));
            set_registered_type4_tracer_width_scaler_00ba99e0(tracer(event), argument_spill(field<float>(definition, 0xac)));
            set_registered_type4_tracer_width_offset_00ba9a00(tracer(event), argument_spill(field<float>(definition, 0xa8)));
            set_registered_type4_tracer_bone_length_00ba9a20(tracer(event), argument_spill(field<float>(definition, 0xb0)));
            set_registered_type4_tracer_height_scale_00ba9a40(tracer(event), argument_spill(field<float>(definition, 0x60)));
            set_registered_type4_tracer_fade_in_00ba9a70(tracer(event), argument_spill(field<float>(definition, 0x74)));
            void* const curve = field<void*>(definition, 0x8c);
            const float at_one = bindings.tracers.sample_current_curve_08(curve, 1.0f);
            const float at_zero = bindings.tracers.sample_current_curve_08(curve, 0.0f);
            set_registered_type4_tracer_width_endpoints_00ba9a90(tracer(event), at_zero, at_one);
        }
    }
    if (actual.state_1c != 0) return;
    auto& current_subject = subject(event);
    float scaled_speed = product(current_subject.fields_50[0], speed);
    if (above(scaled_speed, 1.0f)) scaled_speed = 1.0f;
    if (actual.node_34) {
        auto* const first_node = current_subject.node_110;
        const float subject_58 = current_subject.fields_50[2];
        if ((first_node->transform.valid_flags & 2u) == 0)
            refresh_camera_world_00b6db70(first_node->transform);
        CameraMatrix first_world;
        copy_world_words(first_world, first_node->transform.world);
        auto* const second_node = subject(event).node_110; // Actual110 reloaded.
        if ((second_node->transform.valid_flags & 2u) == 0)
            refresh_camera_world_00b6db70(second_node->transform);
        CameraMatrix second_world;
        copy_world_words(second_world, second_node->transform.world);
        bindings.tracers.update_00baabb0(tracer(event), second_world,
            actual.value_24, first_world.data() + 12, captured_vector,
            scaled_speed, argument_spill(subject_58));
    }
    if (actual.prefix_00.active_0c == 1) {
        if (!above(field<float>(definition, 0x84), speed)) actual.value_30 = 0.0f;
        else add(&event.value_30, &delta);
        if (above(actual.value_30, field<float>(definition, 0x80)) ||
            (at_least(direction_threshold, speed) && actual.untouched_20 == 1) ||
            (above(speed, direction_threshold) && actual.untouched_20 == 0)) {
            // Canonical operation captures CURRENT30, clears actual0C, invokes.
            bindings.children.deactivate_virtual_30(bindings.events.existing_reference(&event));
        }
    }
    const auto current = tracer(event); // Reload after current30 can reenter.
    if (current.actual_owner && bindings.tracers.predicate_00baa510(current) != 0)
        actual.value_28 = 0.0f;
    else add(&event.value_28, &delta);
}
} // namespace bsp
