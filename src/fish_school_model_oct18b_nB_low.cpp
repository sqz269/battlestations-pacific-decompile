#include "bsp/fish_school_model_oct18b_nB_low.hpp"

#include "bsp/camera_decomposition.hpp"
#include "bsp/system_camera_axes.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp::engine_seg11_35_66_cc13 {
namespace {

template<class T>
T& at_00708120(void* object, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(object) + offset);
}

// Keep the original float-store boundaries instead of allowing SSE/fused
// expressions or a widened sum to change the quad or its quantized shade.
float multiply_00708120(float a, float b) {
    float result;
    __asm { fld a }
    __asm { fmul b }
    __asm { fstp result }
    return result;
}

float add_00708120(float a, float b) {
    float result;
    __asm { fld a }
    __asm { fadd b }
    __asm { fstp result }
    return result;
}

void copy_position_00708120(void* stream, std::uint32_t index, const float* xyz) {
    const auto address = at_00708120<std::uint32_t>(stream, 8) +
        at_00708120<std::uint32_t>(stream, 0xc) * index +
        at_00708120<std::uint32_t>(stream, 0x10);
    auto* out = reinterpret_cast<float*>(address);
    __asm {
        mov eax, xyz
        mov edx, out
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax + 4]
        fstp dword ptr [edx + 4]
        fld dword ptr [eax + 8]
        fstp dword ptr [edx + 8]
    }
}

// 007083F8..00708443: dot is float-spilled, its square is float-spilled,
// then (double_at_CE4B48 - square * double_at_CFD408) is float-spilled before
// CVTTSS2SI. Do not clamp: invalid conversions retain the low byte of 80000000h.
std::uint32_t shade_00708120(const float* direction, const void* transform) {
    float spill;
    std::uint32_t result;
    __asm {
        mov ecx, direction
        mov edx, transform
        fld dword ptr [ecx + 4]
        fmul dword ptr [edx + 114h]
        fld dword ptr [ecx]
        fmul dword ptr [edx + 110h]
        faddp st(1), st(0)
        fld dword ptr [edx + 118h]
        fmul dword ptr [ecx + 8]
        faddp st(1), st(0)
        fstp spill
        fld spill
        fmul st(0), st(0)
        fstp spill
        fld spill
        mov eax, 00cfd408h
        fmul qword ptr [eax]
        mov eax, 00ce4b48h
        fsubr qword ptr [eax]
        fstp spill
        cvttss2si eax, spill
        mov result, eax
    }
    return result;
}

float normalized_channel_00708120(std::int32_t byte_value) {
    float result;
    __asm {
        fild byte_value
        mov eax, 00ce4b48h
        fdiv qword ptr [eax]
        fstp result
    }
    return result;
}

void write_color_00708120(void* stream, std::uint32_t index, std::uint32_t shade) {
    const auto base = at_00708120<std::uint32_t>(stream, 8) +
        at_00708120<std::uint32_t>(stream, 0xc) * index;
    const auto packed_offset = at_00708120<std::int32_t>(stream, 0x34);
    const auto byte_value = shade & 0xffu;
    if (packed_offset >= 0) {
        *reinterpret_cast<std::uint32_t*>(base + packed_offset) =
            0xff000000u | byte_value | (byte_value << 8) | (byte_value << 16);
    } else {
        for (std::size_t channel_offset = 0x38; channel_offset != 0x44; channel_offset += 4) {
            const auto offset = at_00708120<std::uint32_t>(stream, channel_offset);
            *reinterpret_cast<float*>(base + offset) =
                normalized_channel_00708120(static_cast<std::int32_t>(byte_value));
        }
        *reinterpret_cast<std::uint32_t*>(base + at_00708120<std::uint32_t>(stream, 0x44)) =
            *reinterpret_cast<const volatile std::uint32_t*>(0x00d7a24c);
    }
}

}  // namespace

void __fastcall FishSchoolModel_Render_00708120(
    void* self, void*, void* context, float lod, std::uint32_t,
    std::uint32_t visibility_u32) {
    void* transform = at_00708120<void*>(context, 8);
    if (at_00708120<std::uint32_t>(transform, 0x198) == 0) return;

    auto* globals = *reinterpret_cast<void* const volatile*>(0x00e188a8);
    float delta;
    __asm {
        mov eax, globals
        fld dword ptr [eax + 21f0h]
        fstp delta
    }
    reinterpret_cast<void(__thiscall*)(void*, float)>(0x00707b00)(
        at_00708120<void*>(self, 0x184), delta);
    // The original UCOMISS/LAHF/TEST/JNP skips only ordered equality; NaN
    // continues to render. It does not test the visibility_u32 stack argument.
    std::uint8_t skip;
    __asm {
        mov eax, self
        movss xmm0, dword ptr [eax + 0ach]
        mov eax, 00d7a218h
        ucomiss xmm0, dword ptr [eax]
        lahf
        test ah, 44h
        setnp skip
    }
    if (skip != 0) return;

    void* geometry = reinterpret_cast<void*(__thiscall*)(void*, std::uint32_t)>(0x00b74640)(self, 0);
    void* stream = reinterpret_cast<void*(__thiscall*)(void*, std::uint32_t)>(0x00b73260)(geometry, 0);
    auto** table = *reinterpret_cast<void***>(stream);
    reinterpret_cast<void(__thiscall*)(void*, std::uint32_t, std::uint32_t, std::uint32_t)>(
        table[0x10 / 4])(stream, 0, 0, 0);

    void* school = at_00708120<void*>(self, 0x184);
    const auto count = at_00708120<std::int32_t>(school, 4);
    const float length = at_00708120<float>(school, 0x2c);
    const float height = at_00708120<float>(school, 0x34);
    auto* particle = at_00708120<std::uint8_t*>(school, 0x64);
    const std::array<float, 3> up{0.0f, *reinterpret_cast<const volatile float*>(0x00d7a24c), 0.0f};
    std::uint32_t vertex = 0;
    for (std::int32_t remaining = count; remaining > 0; --remaining, particle += 0x28) {
        std::array<float, 3> forward;
        std::array<float, 3> cross;
        std::array<float, 3> side;
        for (std::size_t axis = 0; axis != 3; ++axis)
            forward[axis] = multiply_00708120(at_00708120<float>(particle, 0x18 + axis * 4), length);
        camera_vector_cross_004f9b30(cross.data(), up.data(), forward.data());
        camera_vector_cross_004f9b30(side.data(), forward.data(), cross.data());
        normalize_camera_basis_0042b260(side);
        for (float& component : side) component = multiply_00708120(component, height);

        copy_position_00708120(stream, vertex++, reinterpret_cast<const float*>(particle));
        std::array<float, 3> next;
        for (std::size_t axis = 0; axis != 3; ++axis)
            next[axis] = add_00708120(at_00708120<float>(particle, axis * 4), forward[axis]);
        copy_position_00708120(stream, vertex++, next.data());
        // Original computes and spills position+forward again before adding side.
        for (std::size_t axis = 0; axis != 3; ++axis)
            next[axis] = add_00708120(at_00708120<float>(particle, axis * 4), forward[axis]);
        for (std::size_t axis = 0; axis != 3; ++axis)
            next[axis] = add_00708120(next[axis], side[axis]);
        copy_position_00708120(stream, vertex++, next.data());
        for (std::size_t axis = 0; axis != 3; ++axis)
            next[axis] = add_00708120(at_00708120<float>(particle, axis * 4), side[axis]);
        copy_position_00708120(stream, vertex++, next.data());
    }

    if ((at_00708120<std::uint8_t>(transform, 0x5c) & 2u) == 0)
        reinterpret_cast<void(__thiscall*)(void*)>(0x00b6db70)(transform);
    school = at_00708120<void*>(self, 0x184);
    particle = at_00708120<std::uint8_t*>(school, 0x64);
    vertex = 0;
    for (std::int32_t remaining = count; remaining > 0; --remaining, particle += 0x28) {
        const auto shade = shade_00708120(reinterpret_cast<const float*>(particle + 0x18), transform);
        for (unsigned corner = 0; corner != 4; ++corner) write_color_00708120(stream, vertex++, shade);
    }
    table = *reinterpret_cast<void***>(stream);
    reinterpret_cast<void(__thiscall*)(void*)>(table[0x14 / 4])(stream);

    float visibility;
    __asm {
        fild visibility_u32
        cmp visibility_u32, 0
        jge visibility_nonnegative
        mov eax, 00ce3978h
        fadd dword ptr [eax]
    visibility_nonnegative:
        fstp visibility
    }
    float forwarded_lod;
    __asm { fld lod }
    __asm { fstp forwarded_lod }
    reinterpret_cast<void(__thiscall*)(void*, void*, float, float, std::uint32_t)>(0x00b748e0)(
        self, context, forwarded_lod, visibility, 0);
}

}  // namespace bsp::engine_seg11_35_66_cc13
