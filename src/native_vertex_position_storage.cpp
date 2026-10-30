#include "bsp/native_vertex_position_storage.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Scratch = NativeVertexPositionStorageScratch;
using Context = NativeVertexPositionStorageContext;
using Acquired = NativeVertexPositionStorageAcquired;
using Boundary = Acquired::Boundary;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4 && sizeof(float) == 4 && sizeof(double) == 8);
static_assert(sizeof(Scratch) == 12);
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits)); }
Word bits(const volatile void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
Word load(const volatile void* base, Word offset = 0) {
    const volatile void* p = pointer(bits(base) + offset);
    Word value;
    __asm {
        mov edx, p
        mov eax, dword ptr [edx]
        mov value, eax
    }
    return value;
}
void location(Acquired& a, Boundary boundary, Word site) noexcept {
    a.boundary = boundary;
    a.native_site = site;
}
void decode_float3(void* stream, volatile Word (&homes)[2], Scratch& s) {
    volatile Word* const h = homes;
    float* const local = s.decoded;
    __asm {
        mov esi, stream
        mov edx, h
        mov ecx, local
        mov eax, dword ptr [esi + 0xc]
        imul eax, dword ptr [edx + 4]
        add eax, dword ptr [esi + 0x10]
        add eax, dword ptr [esi + 8]
        movss xmm0, dword ptr [eax]
        movss dword ptr [ecx], xmm0
        movss xmm0, dword ptr [eax + 4]
        movss dword ptr [ecx + 4], xmm0
        movss xmm0, dword ptr [eax + 8]
        movss dword ptr [ecx + 8], xmm0
    }
}
void decode_byte3(void* stream, volatile Word (&homes)[2], Scratch& s) {
    volatile Word* const h = homes;
    float* const local = s.decoded;
    __asm {
        mov esi, stream
        mov edx, h
        mov edi, local
        mov ecx, dword ptr [esi + 0xc]
        imul ecx, dword ptr [edx + 4]
        add ecx, dword ptr [esi + 0x10]
        add ecx, dword ptr [esi + 8]
        xor eax, eax
    position_byte_loop:
        movzx edx, byte ptr [ecx + eax]
        cvtsi2ss xmm0, edx
        movss dword ptr [edi + eax*4], xmm0
        add eax, 1
        cmp eax, 3
        jc position_byte_loop
    }
}
void decode_normalized_byte3(void* stream, volatile Word (&homes)[2], Scratch& s, const Context& c) {
    volatile Word* const h = homes;
    float* const local = s.decoded;
    const volatile double* const divisor = &c.byte_divisor_00ce4b48;
    __asm {
        mov esi, stream
        mov edi, local
        mov edx, h
        mov ecx, dword ptr [esi + 0xc]
        mov eax, divisor
        fld qword ptr [eax] // Native BEFORE current index/offset/mapping reads
        imul ecx, dword ptr [edx + 4]
        add ecx, dword ptr [esi + 0x10]
        add ecx, dword ptr [esi + 8]
        xor eax, eax
    position_normalized_byte_loop:
        movzx esi, byte ptr [ecx + eax]
        mov dword ptr [edx + 4], esi // actual INDEX home, not another local
        add eax, 1
        cmp eax, 3
        fild dword ptr [edx + 4]
        fdiv st(0), st(1)
        fstp dword ptr [edi + eax*4 - 4]
        jc position_normalized_byte_loop
        fstp st(0)
    }
}
void decode_short3(void* stream, volatile Word (&homes)[2], Scratch& s) {
    volatile Word* const h = homes;
    float* const local = s.decoded;
    __asm {
        mov esi, stream
        mov edx, h
        mov edi, local
        mov eax, dword ptr [esi + 0xc]
        imul eax, dword ptr [edx + 4]
        add eax, dword ptr [esi + 0x10]
        add eax, dword ptr [esi + 8]
        mov ecx, dword ptr [eax]
        mov eax, dword ptr [eax + 4]
        movsx edx, cx
        shr ecx, 0x10
        cvtsi2ss xmm0, edx
        movsx ecx, cx
        movss dword ptr [edi], xmm0
        movsx edx, ax
        cvtsi2ss xmm0, ecx
        movss dword ptr [edi + 4], xmm0
        cvtsi2ss xmm0, edx
        movss dword ptr [edi + 8], xmm0
    }
}
// One shared normalization tail, like Native6A43. Both decode paths keep ST0
// live across their own integer stores without a C++ callback/FP spill.
void decode_normalized_short3(void* stream, volatile Word (&homes)[2], Scratch& s,
    Word format, const Context& c) {
    volatile Word* const h = homes;
    float* const local = s.decoded;
    const volatile double* const signed_divisor = &c.signed_short_divisor_00ce5f98;
    const volatile double* const unsigned_divisor = &c.unsigned_short_divisor_00ce5f90;
    __asm {
        mov esi, stream
        mov edi, local
        mov ebx, h
        mov eax, dword ptr [esi + 0xc]
        imul eax, dword ptr [ebx + 4]
        add eax, dword ptr [esi + 0x10]
        add eax, dword ptr [esi + 8]
        mov ecx, dword ptr [eax]
        mov eax, dword ptr [eax + 4]
        cmp format, 12
        je position_unsigned_short
        movsx edx, cx
        mov dword ptr [ebx + 4], edx
        shr ecx, 0x10
        movsx ecx, cx
        fild dword ptr [ebx + 4]
        mov dword ptr [ebx + 4], ecx
        fstp dword ptr [edi]
        movsx edx, ax
        fild dword ptr [ebx + 4]
        mov dword ptr [ebx + 4], edx
        fstp dword ptr [edi + 4]
        fild dword ptr [ebx + 4]
        fstp dword ptr [edi + 8]
        fld dword ptr [edi]
        mov eax, signed_divisor
        fld qword ptr [eax]
    position_normalize_short_tail:
        fdiv st(1), st(0)
        fxch st(1)
        fstp dword ptr [edi]
        fld dword ptr [edi + 4]
        fdiv st(0), st(1)
        fstp dword ptr [edi + 4]
        fdivr dword ptr [edi + 8]
        fstp dword ptr [edi + 8]
        jmp position_short_done
    position_unsigned_short:
        movzx edx, cx
        mov dword ptr [ebx + 4], edx
        shr ecx, 0x10
        movzx eax, ax
        fild dword ptr [ebx + 4]
        mov dword ptr [ebx + 4], ecx
        fstp dword ptr [edi]
        fild dword ptr [ebx + 4]
        mov dword ptr [ebx + 4], eax
        fstp dword ptr [edi + 4]
        fild dword ptr [ebx + 4]
        fstp dword ptr [edi + 8]
        fld dword ptr [edi]
        mov eax, unsigned_divisor
        fld qword ptr [eax]
        jmp position_normalize_short_tail
    position_short_done:
    }
}
Word scaled_output(void* stream, volatile Word (&homes)[2], Scratch& s) {
    volatile Word* const h = homes;
    float* const local = s.decoded;
    Word output;
    __asm {
        mov esi, stream
        mov edi, local
        mov edx, h
        mov eax, dword ptr [esi + 0x18] // BOTH reads AFTER decoder/import return
        mov ecx, dword ptr [esi + 0x50]
        shl eax, 5
        fld dword ptr [eax + ecx]
        add eax, ecx
        fmul dword ptr [edi]
        fadd dword ptr [eax + 0x10]
        fstp dword ptr [edi]
        fld dword ptr [eax + 4]
        fmul dword ptr [edi + 4]
        fadd dword ptr [eax + 0x14]
        fstp dword ptr [edi + 4]
        fld dword ptr [eax + 8]
        fmul dword ptr [edi + 8]
        fadd dword ptr [eax + 0x18]
        mov eax, dword ptr [edx] // LATE Native6AF7, BEFORE final decode spill
        fstp dword ptr [edi + 8]
        fld dword ptr [edi]
        fstp dword ptr [eax]
        fld dword ptr [edi + 4]
        fstp dword ptr [eax + 4]
        fld dword ptr [edi + 8]
        fstp dword ptr [eax + 8]
        mov output, eax // captured identity; no reread after output aliases
    }
    return output;
}
Word direct_output(void* stream, volatile Word (&homes)[2]) {
    volatile Word* const h = homes;
    Word output;
    __asm {
        mov esi, stream
        mov edx, h
        mov ecx, dword ptr [esi + 0xc]
        imul ecx, dword ptr [edx + 4]
        add ecx, dword ptr [esi + 0x10]
        mov eax, dword ptr [edx] // Native6B23 BEFORE mapping+08 read
        add ecx, dword ptr [esi + 8]
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fstp dword ptr [eax + 8]
        mov output, eax
    }
    return output;
}
} // namespace

void* read_native_vertex_position_storage_004768d0(void* stream,
    volatile Word (&homes)[2], Scratch& scratch, const Context& context, Acquired& a) {
    using Phase = Acquired::Phase;
    if (a.phase != Phase::fresh || a.failure)
        throw std::logic_error("4768D0 requires one fresh retained acquired record");
    a.phase = Phase::entered;
    a.actual_stream = stream;
    a.home_identity = homes;
    a.scratch_identity = &scratch;
    try {
        location(a, Boundary::entry, 0x004768d6);
        Word output;
        if (load(stream, 0x50) == 0) {
            location(a, Boundary::direct_copy, 0x00476b18);
            output = direct_output(stream, homes);
        } else {
            const Word format = load(stream, 0x14);
            a.captured_format = format;
            location(a, Boundary::decode, 0x004768e0);
            // Translation of the authenticated immutable Native jump graph
            // (476B44 targets /476B68 byte map), not a type/domain guard.
            switch (format) {
            case 2: case 3: decode_float3(stream, homes, scratch); break;
            case 5: decode_byte3(stream, homes, scratch); break;
            case 7: decode_short3(stream, homes, scratch); break;
            case 8: decode_normalized_byte3(stream, homes, scratch, context); break;
            case 10: case 12: decode_normalized_short3(stream, homes, scratch, format, context); break;
            case 13: {
                Word relative = load(stream, 0xc);
                relative *= load(homes, 4);
                relative += load(stream, 0x10);
                const Word base = load(stream, 8);
                const Word packed = load(pointer(base + relative * 4u));
                a.captured_source = pointer(base + relative * 4u);
                location(a, Boundary::unpack, 0x004769aa);
                a.unpack_entered = true;
                unpack_native_position_9bit_00475f80(packed, scratch.decoded);
                a.unpack_returned = true;
                break;
            }
            case 16: {
                Word relative = load(stream, 0xc);
                relative *= load(homes, 4);
                relative += load(stream, 0x10);
                const Word source = relative + load(stream, 8);
                a.captured_source = pointer(source);
                location(a, Boundary::half_import, 0x00476ac1);
                a.half_entered = true;
                (void)context.half_import.convert(scratch.decoded,
                    static_cast<const std::uint16_t*>(pointer(source)), 3);
                a.half_returned = true;
                break;
            }
            default: break; // NO decode, preserving actual12-byte preimage
            }
            location(a, Boundary::scale_bias, 0x00476ac6);
            output = scaled_output(stream, homes, scratch);
        }
        a.returned_output = pointer(output);
        a.phase = Phase::complete;
        return pointer(output);
    } catch (...) {
        a.failure = std::current_exception();
        a.escaped_boundary = a.boundary;
        a.escaped_site = a.native_site;
        a.phase = Phase::failed;
        throw; // passive retention only; no cleanup/origin inference
    }
}
} // namespace bsp
