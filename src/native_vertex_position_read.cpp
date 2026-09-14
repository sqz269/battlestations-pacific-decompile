#include "bsp/native_vertex_position_read.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native vertex position reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::uint32_t word(const void* object, std::size_t offset) noexcept {
    std::uint32_t result;
    std::memcpy(&result, static_cast<const unsigned char*>(object) + offset, 4);
    return result;
}
const unsigned char* bytes(std::uint32_t address) noexcept {
    return reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(address));
}
void copy_float3_x87(const void* input, float* output) noexcept {
    __asm {
        mov ecx, input
        mov eax, output
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fstp dword ptr [eax + 8]
    }
}
void byte3(const unsigned char* input, float* output) noexcept {
    __asm {
        mov ecx, input
        mov edx, output
        xor eax, eax
    byte_loop:
        movzx esi, byte ptr [ecx + eax]
        cvtsi2ss xmm0, esi
        movss dword ptr [edx + eax * 4], xmm0
        inc eax
        cmp eax, 3
        jb byte_loop
    }
}
void normalized_byte3(const unsigned char* input, float* output) noexcept {
    const double divisor = 255.0;
    std::int32_t value;
    __asm {
        mov ecx, input
        mov edx, output
        xor eax, eax
        fld divisor
    byte_loop:
        movzx esi, byte ptr [ecx + eax]
        mov value, esi
        fild value
        fdiv st(0), st(1)
        fstp dword ptr [edx + eax * 4]
        inc eax
        cmp eax, 3
        jb byte_loop
        fstp st(0)
    }
}
void short3(const unsigned char* input, float* output) noexcept {
    __asm {
        mov eax, input
        mov ecx, dword ptr [eax]
        mov eax, dword ptr [eax + 4]
        movsx edx, cx
        shr ecx, 16
        cvtsi2ss xmm0, edx
        movsx ecx, cx
        mov edx, output
        movss dword ptr [edx], xmm0
        cvtsi2ss xmm0, ecx
        movss dword ptr [edx + 4], xmm0
        movsx eax, ax
        cvtsi2ss xmm0, eax
        movss dword ptr [edx + 8], xmm0
    }
}
void normalized_short3(const unsigned char* input, float* output,
    bool unsigned_values) noexcept {
    const double divisor = unsigned_values ? 65535.0 : 32767.0;
    std::int32_t value;
    __asm {
        mov eax, input
        mov ecx, dword ptr [eax]
        mov eax, dword ptr [eax + 4]
        cmp unsigned_values, 0
        je signed_values
        movzx edx, cx
        shr ecx, 16
        movzx eax, ax
        jmp convert_values
    signed_values:
        movsx edx, cx
        shr ecx, 16
        movsx ecx, cx
        movsx eax, ax
    convert_values:
        mov value, edx
        mov edx, output
        fild value
        mov value, ecx
        fstp dword ptr [edx]
        fild value
        mov value, eax
        fstp dword ptr [edx + 4]
        fild value
        fstp dword ptr [edx + 8]
        fld dword ptr [edx]
        fld divisor
        fdiv st(1), st(0)
        fxch st(1)
        fstp dword ptr [edx]
        fld dword ptr [edx + 4]
        fdiv st(0), st(1)
        fstp dword ptr [edx + 4]
        fdivr dword ptr [edx + 8]
        fstp dword ptr [edx + 8]
    }
}
void scale_bias(float* local, const void* record) noexcept {
    // All three transformed locals spill BEFORE publishing caller output.
    // Scale is loaded before multiplying the decoded component; no FMA.
    __asm {
        mov ecx, record
        mov edx, local
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fadd dword ptr [ecx + 10h]
        fstp dword ptr [edx]
        fld dword ptr [ecx + 4]
        fmul dword ptr [edx + 4]
        fadd dword ptr [ecx + 14h]
        fstp dword ptr [edx + 4]
        fld dword ptr [ecx + 8]
        fmul dword ptr [edx + 8]
        fadd dword ptr [ecx + 18h]
        fstp dword ptr [edx + 8]
    }
}
void scale_bias_normal(float* local, const void* record) noexcept {
    // 0070FFDF loads lane0 first; operand order matters for NaN payloads.
    __asm {
        mov ecx, record
        mov edx, local
        fld dword ptr [edx]
        fmul dword ptr [ecx]
        fadd dword ptr [ecx + 10h]
        fstp dword ptr [edx]
        fld dword ptr [ecx + 4]
        fmul dword ptr [edx + 4]
        fadd dword ptr [ecx + 14h]
        fstp dword ptr [edx + 4]
        fld dword ptr [ecx + 8]
        fmul dword ptr [edx + 8]
        fadd dword ptr [ecx + 18h]
        fstp dword ptr [edx + 8]
    }
}
void scale_bias_uv(float* local, const void* record) noexcept {
    __asm {
        mov ecx, record
        mov edx, local
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fadd dword ptr [ecx + 10h]
        fstp dword ptr [edx]
        fld dword ptr [ecx + 4]
        fmul dword ptr [edx + 4]
        fadd dword ptr [ecx + 14h]
        fstp dword ptr [edx + 4]
    }
}
void copy_float2_x87(const void* input, float* output) noexcept {
    __asm {
        mov ecx, input
        mov eax, output
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
    }
}
void float_colour(void* stream, std::uint32_t vertex_base, void* output) noexcept {
    const double multiplier = 255.0;
    std::uint16_t saved_control, trunc_control;
    std::int32_t converted;
    std::uint32_t packed;
    __asm {
        mov ecx, stream
        mov edx, vertex_base
        lea edi, packed
        mov eax, dword ptr [ecx + 38h]
        fnstcw saved_control
        fld dword ptr [eax + edx]
        fld multiplier
        movzx eax, saved_control
        or eax, 0c00h
        mov trunc_control, ax
        fmul st(1), st(0)
        fxch st(1)
        fldcw trunc_control
        fistp converted
        mov eax, converted
        mov byte ptr [edi + 2], al
        mov eax, dword ptr [ecx + 3ch]
        fldcw saved_control
        fld dword ptr [eax + edx]
        fnstcw saved_control
        fmul st(0), st(1)
        movzx eax, saved_control
        or eax, 0c00h
        mov trunc_control, ax
        fldcw trunc_control
        fistp converted
        mov eax, converted
        mov byte ptr [edi + 1], al
        mov eax, dword ptr [ecx + 40h]
        fldcw saved_control
        mov ecx, dword ptr [ecx + 44h]
        fld dword ptr [eax + edx]
        fnstcw saved_control
        fmul st(0), st(1)
        movzx eax, saved_control
        or eax, 0c00h
        mov trunc_control, ax
        fldcw trunc_control
        fistp converted
        mov eax, converted
        mov byte ptr [edi], al
        fldcw saved_control
        fmul dword ptr [ecx + edx]
        fnstcw saved_control
        movzx eax, saved_control
        or eax, 0c00h
        mov trunc_control, ax
        fldcw trunc_control
        fistp converted
        mov eax, converted
        mov byte ptr [edi + 3], al
        mov ecx, packed
        mov eax, output
        fldcw saved_control
        mov dword ptr [eax], ecx
    }
}
} // namespace

NativeD3dx9Float16Import::NativeD3dx9Float16Import(HMODULE module) {
    if (!module) throw std::invalid_argument("actual d3dx9_40 module required");
    const auto address = GetProcAddress(module, "D3DXFloat16To32Array");
    if (!address) throw std::runtime_error("actual D3DXFloat16To32Array export required");
    static_assert(sizeof(address) == sizeof(function_));
    std::memcpy(&function_, &address, sizeof(function_));
}
float* NativeD3dx9Float16Import::convert(float* output,
    const std::uint16_t* input, UINT count) const {
    return function_(output, input, count);
}

void unpack_native_position_9bit_00475f80(std::uint32_t packed, float* output) {
    std::int32_t value;
    __asm {
        mov eax, packed
        mov edx, eax
        and edx, 1ffh
        mov value, edx
        fild value
        mov ecx, output
        fstp dword ptr [ecx]
        mov edx, eax
        shr edx, 10
        and edx, 1ffh
        mov value, edx
        fild value
        shr eax, 20
        fstp dword ptr [ecx + 4]
        and eax, 1ffh
        mov value, eax
        fild value
        fstp dword ptr [ecx + 8]
    }
}

bool read_native_vertex_position_004768d0(void* stream, std::uint32_t index,
    float (&output)[3], const NativeD3dx9Float16Import& half_import) {
    if (word(stream, 0x50) == 0) {
        auto address = word(stream, 0x0c) * index;
        address += word(stream, 0x10);
        address += word(stream, 0x08);
        copy_float3_x87(bytes(address), output);
        return true;
    }
    const auto type = word(stream, 0x14);
    switch (type) {
    case 2: case 3: case 5: case 7: case 8: case 10: case 12: case 13: case 16: break;
    default: return false; // Native skips initialization of its float3 local.
    }
    auto relative = word(stream, 0x0c) * index;
    relative += word(stream, 0x10);
    const auto address = word(stream, 0x08) + (type == 13 ? relative * 4u : relative);
    const auto* input = bytes(address);
    float local[3]; // SAME three scalar native locals, not a vertex-array copy.
    switch (type) {
    case 2: case 3:
        // Native MOVSS preserves source bits until scale/bias x87 loads.
        std::memcpy(local, input, sizeof(local));
        break;
    case 5: byte3(input, local); break;
    case 7: short3(input, local); break;
    case 8: normalized_byte3(input, local); break;
    case 10: normalized_short3(input, local, false); break;
    case 12: normalized_short3(input, local, true); break;
    case 13: unpack_native_position_9bit_00475f80(word(input, 0), local); break;
    case 16:
        half_import.convert(local, reinterpret_cast<const std::uint16_t*>(input), 3);
        break;
    }
    // Native reloads BOTH the element index and actual record pointer here.
    const auto element_index = word(stream, 0x18);
    const auto current_records = word(stream, 0x50);
    const auto record = (element_index << 5) + current_records;
    scale_bias(local, bytes(record));
    copy_float3_x87(local, output);
    return true;
}

bool read_native_vertex_normal_0070fdb0(void* stream, std::uint32_t index,
    float (&output)[3], const NativeD3dx9Float16Import& half_import) {
    if (word(stream, 0x50) == 0) {
        const auto address = word(stream, 0x0c) * index + word(stream, 0x1c) + word(stream, 0x08);
        copy_float3_x87(bytes(address), output);
        return true;
    }
    const auto type = word(stream, 0x20);
    switch (type) {
    case 2: case 4: case 5: case 7: case 8: case 10: case 12: case 13: case 16: break;
    default: return false;
    }
    const auto relative = word(stream, 0x0c) * index + word(stream, 0x1c);
    const auto* input = bytes(word(stream, 0x08) + (type == 13 ? relative * 4u : relative));
    float local[3];
    switch (type) {
    case 2: std::memcpy(local, input, sizeof(local)); break;
    case 4: case 8: normalized_byte3(input, local); break;
    case 5: byte3(input, local); break;
    case 7: short3(input, local); break;
    case 10: normalized_short3(input, local, false); break;
    case 12: normalized_short3(input, local, true); break;
    case 13: unpack_native_position_9bit_00475f80(word(input, 0), local); break;
    case 16: half_import.convert(local, reinterpret_cast<const std::uint16_t*>(input), 3); break;
    }
    const auto element_index = word(stream, 0x24);
    const auto current_records = word(stream, 0x50);
    scale_bias_normal(local, bytes((element_index << 5) + current_records));
    copy_float3_x87(local, output);
    return true;
}

bool read_native_vertex_uv_007100a0(void* stream, std::uint32_t index,
    float (&output)[2], const NativeD3dx9Float16Import& half_import) {
    if (word(stream, 0x50) == 0) {
        const auto address = word(stream, 0x0c) * index + word(stream, 0x28) + word(stream, 0x08);
        copy_float2_x87(bytes(address), output);
        return true;
    }
    const auto type = word(stream, 0x2c);
    switch (type) {
    case 1: case 6: case 9: case 11: case 15: break;
    default: return false;
    }
    const auto address = word(stream, 0x0c) * index + word(stream, 0x28) + word(stream, 0x08);
    const auto* input = bytes(address);
    float local[3]; // Native short formats read and convert the third lane too.
    switch (type) {
    case 1: std::memcpy(local, input, 2 * sizeof(float)); break;
    case 6: short3(input, local); break;
    case 9: normalized_short3(input, local, false); break;
    case 11: normalized_short3(input, local, true); break;
    case 15: half_import.convert(local, reinterpret_cast<const std::uint16_t*>(input), 2); break;
    }
    const auto element_index = word(stream, 0x30);
    const auto current_records = word(stream, 0x50);
    scale_bias_uv(local, bytes((element_index << 5) + current_records));
    copy_float2_x87(local, output);
    return true;
}

void read_native_vertex_colour_00476180(void* stream, std::uint32_t index,
    std::uint32_t& output) {
    const auto vertex_base = word(stream, 0x0c) * index + word(stream, 0x08);
    const auto packed_offset = word(stream, 0x34);
    if (static_cast<std::int32_t>(packed_offset) >= 0) {
        const auto value = word(bytes(vertex_base + packed_offset), 0);
        std::memcpy(&output, &value, sizeof(value));
        return;
    }
    float_colour(stream, vertex_base, &output);
}
} // namespace bsp
