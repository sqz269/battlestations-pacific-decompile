#include "bsp/native_gui_text_glyph_quad_storage.hpp"

#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_wide_string.hpp"

#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Acquired = NativeGuiTextGlyphQuadStorageAcquired;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(float) == 4 && sizeof(double) == 8);
Word bits(const volatile void* p) noexcept { return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p)); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(p)); }
Word word(Word p, Word n) noexcept {
    Word result;
    __asm { mov eax, p }
    __asm { mov edx, n }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
void put(Word p, Word n, Word v) noexcept {
    __asm { mov eax, p }
    __asm { mov edx, n }
    __asm { mov ecx, v }
    __asm { mov dword ptr [eax + edx], ecx }
}
std::uint16_t half(Word p, Word n) noexcept {
    Word result;
    __asm { mov eax, p }
    __asm { mov edx, n }
    __asm { movzx eax, word ptr [eax + edx] }
    __asm { mov result, eax }
    return static_cast<std::uint16_t>(result);
}
void put_half(Word p, Word n, Word v) noexcept {
    __asm { mov eax, p }
    __asm { mov edx, n }
    __asm { mov ecx, v }
    __asm { mov word ptr [eax + edx], cx }
}
void home_byte(volatile Word* h, Word n) noexcept { *reinterpret_cast<volatile unsigned char*>(static_cast<std::uintptr_t>(bits(h) + n)) = 0xff; }

void color(Word stream, Word vertex, Word offset, bool outer,
    volatile Word* h, Word white) {
    const Word stride = word(stream, 0x0c);
    if ((offset & 0x80000000u) == 0) {
        Word packed = 0; // Private integer scratch, overwritten before its use.
        if (outer) packed = h[7];
        const Word base = stride * vertex + word(stream, 8);
        if (!outer) packed = h[7];
        put(base, offset, packed);
    } else {
        Word first = 0;
        if (outer) first = word(stream, 0x38);
        const Word base = stride * vertex + word(stream, 8);
        if (!outer) first = word(stream, 0x38);
        put(base, first, white);
        put(base, word(stream, 0x3c), white);
        put(base, word(stream, 0x40), white);
        put(base, word(stream, 0x44), white);
    }
}

void prefix(void* text, volatile Word (&homes)[10],
    NativeGuiTextGlyphQuadStorageContext& c, Acquired& a) {
    volatile Word* const h = homes;
    volatile Word* const s = a.locals; // F14 -> s[0], F18 -> s[1], ..., F48 -> s[13].
    const volatile float* const one = &c.one_00d7a24c;
    const volatile float* const scale = &c.vertical_scale_00e12fd4;
    const volatile double* const width = &c.width_00cec380;
    const volatile double* const height = &c.height_00cef1b8;
    Word glyph, stream, first, first_position, white;
    // AB9912..AB99FA, including every physical home overwrite and x87 spill.
    // No calls or EBP use inside this arithmetic interval.
    __asm {
        mov edx, h
        mov ebx, dword ptr [edx]
        mov glyph, ebx
        movsx eax, word ptr [ebx + 10h]
        mov dword ptr [edx], eax
        mov ecx, dword ptr [edx + 4]
        movss xmm0, dword ptr [ecx + 4]
        fild dword ptr [edx]
        fadd dword ptr [ecx]
        movzx eax, word ptr [edx + 1ch]
        movzx ecx, word ptr [ebx + 14h]
        mov esi, s
        fst dword ptr [esi + 4]
        mov ebx, text
        fld dword ptr [ebx + 1d8h]
        mov dword ptr [edx], ecx
        movss dword ptr [esi], xmm0
        mov dword ptr [edx + 1ch], eax
        fild dword ptr [edx]
        movss dword ptr [esi + 10h], xmm0
        mov eax, dword ptr [edx + 8]
        mov stream, eax
        mov ecx, dword ptr [eax + 0ch]
        fmulp st(1), st(0)
        mov ebx, dword ptr [edx + 20h]
        mov first, ebx
        xorps xmm0, xmm0
        imul ecx, ebx
        faddp st(1), st(0)
        fstp dword ptr [edx]
        fld dword ptr [esi]
        fild dword ptr [edx + 1ch]
        add ecx, dword ptr [eax + 10h]
        mov edi, one
        movss xmm3, dword ptr [edi]
        movd white, xmm3
        faddp st(1), st(0)
        fld1
        add ecx, dword ptr [eax + 8]
        fadd st(1), st(0)
        mov first_position, ecx
        movss dword ptr [ecx + 8], xmm0
        fsubp st(1), st(0)
        fstp dword ptr [edx + 1ch]
        fld dword ptr [esi + 4]
        mov edi, width
        fld qword ptr [edi]
        fdiv st(1), st(0)
        fxch st(1)
        fstp dword ptr [esi + 4]
        fld dword ptr [esi + 10h]
        mov edi, height
        fld qword ptr [edi]
        fdiv st(1), st(0)
        mov edi, scale
        fld dword ptr [edi]
        fld st(0)
        fmulp st(3), st(0)
        fxch st(2)
        fstp dword ptr [esi + 8]
        fld dword ptr [edx]
        fdivrp st(3), st(0)
        fxch st(2)
        fstp dword ptr [esi + 10h]
        fld dword ptr [edx + 1ch]
        fdivrp st(2), st(0)
        mov byte ptr [edx + 1eh], 0ffh
        mov byte ptr [edx + 1dh], 0ffh
        mov byte ptr [edx + 1ch], 0ffh
        mov byte ptr [edx + 1fh], 0ffh
        fmulp st(1), st(0)
        fstp dword ptr [esi + 14h]
        fld dword ptr [esi + 4]
        fstp dword ptr [ecx]
        fld dword ptr [esi + 8]
        fstp dword ptr [ecx + 4]
    }
    a.captured_glyph = glyph;
    a.captured_stream = stream;
    a.captured_first_vertex = first;
    a.first_position = first_position;
    a.captured_white = white;

    // Native SSE moves below are raw DWORD bit copies, including NaN payloads.
    Word stride = word(stream, 0x0c);
    Word u = word(glyph, 4);
    Word base = stride * first + word(stream, 8);
    Word v = word(glyph, 0);
    Word uv = base + word(stream, 0x28);
    put(uv, 0, u); put(uv, 4, v);
    Word offset = word(stream, 0x34);
    color(stream, first, offset, true, h, white);

    stride = word(stream, 0x0c);
    const Word top = s[2], right = s[4];
    const Word second = first + 1u;
    Word pos = stride * second + word(stream, 0x10);
    h[0] = second;
    pos += word(stream, 8);
    home_byte(h, 30);
    put(pos, 0, right); put(pos, 4, top); put(pos, 8, 0);
    stride = word(stream, 0x0c);
    u = word(glyph, 0x0c);
    base = stride * second + word(stream, 8);
    v = word(glyph, 0);
    home_byte(h, 29);
    uv = base + word(stream, 0x28);
    home_byte(h, 28);
    put(uv, 0, u); put(uv, 4, v);
    offset = word(stream, 0x34);
    home_byte(h, 31);
    color(stream, second, offset, false, h, white);

    stride = word(stream, 0x0c);
    const Word bottom = s[5];
    const Word third = first + 2u;
    pos = stride * third + word(stream, 0x10);
    h[2] = third;
    pos += word(stream, 8);
    home_byte(h, 30);
    put(pos, 0, right); put(pos, 4, bottom); put(pos, 8, 0);
    stride = word(stream, 0x0c);
    u = word(glyph, 0x0c);
    base = stride * third + word(stream, 8);
    v = word(glyph, 8);
    home_byte(h, 29);
    home_byte(h, 28);
    uv = base + word(stream, 0x28);
    put(uv, 0, u); put(uv, 4, v);
    offset = word(stream, 0x34);
    home_byte(h, 31);
    color(stream, third, offset, false, h, white);

    stride = word(stream, 0x0c);
    const Word left = s[1];
    const Word fourth = first + 3u;
    pos = stride * fourth + word(stream, 0x10);
    home_byte(h, 30);
    pos += word(stream, 8);
    home_byte(h, 29);
    put(pos, 0, left); put(pos, 4, bottom); put(pos, 8, 0);
    stride = word(stream, 0x0c);
    u = word(glyph, 4);
    base = stride * fourth + word(stream, 8);
    v = word(glyph, 8);
    home_byte(h, 28);
    uv = base + word(stream, 0x28);
    home_byte(h, 31);
    put(uv, 0, u); put(uv, 4, v);
    offset = word(stream, 0x34);
    color(stream, fourth, offset, true, h, white);
    a.captured_fourth_vertex = fourth; // Native EBP through the membership gate.

    const Word q = h[6]; // Native reads the home ONCE, then ADD twice.
    const Word indices = h[3];
    const Word q4 = q + q;
    const Word index_base = q4 + q4;
    put_half(indices, 2, index_base + 1u);
    put_half(indices, 0, index_base);
    put_half(indices, 6, index_base);
    put_half(indices, 4, index_base + 2u);
    put_half(indices, 8, index_base + 2u);
    put_half(indices, 10, index_base + 3u);
}

void clear_uv(Word stream, Word vertex) {
    Word p = word(stream, 0x0c) * vertex;
    p += word(stream, 8);
    p += word(stream, 0x28);
    put(p, 0, 0); put(p, 4, 0);
}
void clear_uv_home(Word stream, const volatile Word* actual_home) {
    const Word stride = word(stream, 0x0c);
    const Word vertex = *actual_home; // Native loads this home AFTER stream+C.
    Word p = stride * vertex;
    p += word(stream, 8);
    p += word(stream, 0x28);
    put(p, 0, 0); put(p, 4, 0);
}
void boundary(Acquired& a, Word site, void* receiver) noexcept {
    a.active_call_site = site;
    a.captured_receiver = receiver;
}
void current(Acquired& a, Word receiver, Word slot) noexcept {
    a.captured_receiver = pointer(receiver);
    a.captured_profile = word(receiver, 0);
    a.captured_target = word(a.captured_profile, slot);
}
void normal_string_return(Acquired& a, NativeStringRawPoolContext& strings,
    Word getter_site, Word return_site, bool wide) {
    const Word data = static_cast<volatile Word*>(a.locals)[5]; // F28, capture before getter.
    a.native_eh_state = -1; // Native TEST data, then disarm before length/getter.
    if (data == 0) return;
    const Word length = static_cast<volatile Word*>(a.locals)[4];
    const Word bytes = wide ? length * 2u + 2u : length + 1u;
    boundary(a, getter_site, nullptr);
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    boundary(a, return_site, pool);
    return_native_string_pool_00bd1510(pool, pointer(data), bytes,
        strings.actual_small_returns_disabled_01090aa4);
}

void prepare_shadow(NativeGuiTextGlyphQuadStorageContext& c, Acquired& a, Word& offset) {
    volatile Word* const s = a.locals;
    const volatile float* const source = &c.shadow_00d5c5c0;
    const volatile float* const one = &c.one_00d7a24c;
    Word* const output = &offset;
    __asm {
        mov eax, source
        fld dword ptr [eax]
        xorps xmm0, xmm0
        mov edx, output
        fstp dword ptr [edx]
        mov ecx, s
        movss dword ptr [ecx + 1ch], xmm0
        movss dword ptr [ecx + 20h], xmm0
        movss dword ptr [ecx + 24h], xmm0
        mov eax, one
        movss xmm0, dword ptr [eax]
        movss dword ptr [ecx + 28h], xmm0
    }
}
void prepare_position(Word text, volatile Word* h,
    NativeGuiTextGlyphQuadStorageContext& c, Acquired& a, volatile Word*& output) {
    volatile Word* const s = a.locals;
    const volatile double* const x_sub = &c.x_subtract_00d7a358;
    const volatile double* const y_sub = &c.y_subtract_00d5c7a8;
    const volatile double* const z_sub = &c.z_subtract_00d7a220;
    const volatile double* const bias = &c.input_x_bias_00d06880;
    const volatile double* const width = &c.width_00cec380;
    if (*reinterpret_cast<const volatile unsigned char*>(static_cast<std::uintptr_t>(text + 0x1b4)) != 0) {
        __asm {
            mov eax, text
            mov edx, s
            fld dword ptr [eax + 1b8h]
            fstp dword ptr [edx + 10h]
            fld dword ptr [eax + 1bch]
            fstp dword ptr [edx + 14h]
            fld dword ptr [edx + 10h]
            fadd dword ptr [edx + 4]
            fstp dword ptr [edx + 1ch]
            fld dword ptr [edx + 14h]
            fadd dword ptr [edx + 8]
            fstp dword ptr [edx + 20h]
            fld dword ptr [edx + 0ch]
            mov eax, z_sub
            fsub qword ptr [eax]
            fstp dword ptr [edx + 24h]
        }
        output = s + 7; // Native F30.
    } else if (word(word(text, 0x1b0), 0xec) == 1u) {
        __asm {
            mov edx, s
            fld dword ptr [edx + 4]
            mov eax, x_sub
            fsub qword ptr [eax]
            fstp dword ptr [edx + 1ch]
            fld dword ptr [edx + 8]
            mov eax, y_sub
            fsub qword ptr [eax]
            fstp dword ptr [edx + 20h]
            fld dword ptr [edx + 0ch]
            mov eax, z_sub
            fsub qword ptr [eax]
            fstp dword ptr [edx + 24h]
        }
        output = s + 7;
    } else {
        __asm {
            mov eax, h
            mov ecx, dword ptr [eax + 4]
            fld dword ptr [ecx]
            mov eax, bias
            fsub qword ptr [eax]
            mov eax, width
            fdiv qword ptr [eax]
            mov edx, s
            fadd dword ptr [edx + 4]
            fstp dword ptr [edx + 2ch]
            fld dword ptr [edx + 2ch]
            mov eax, x_sub
            fsub qword ptr [eax]
            fstp dword ptr [edx + 2ch]
            fld dword ptr [edx + 8]
            mov eax, y_sub
            fsub qword ptr [eax]
            fstp dword ptr [edx + 30h]
            fld dword ptr [edx + 0ch]
            mov eax, z_sub
            fsub qword ptr [eax]
            fstp dword ptr [edx + 34h]
        }
        output = s + 11; // Native F40, including the mandatory F40 spill/reload.
    }
}
} // namespace

__declspec(noinline) void write_native_gui_text_glyph_quad_storage_00ab98f0(
    void* actual_text, volatile Word (&homes)[10], NativeGuiTextGlyphQuadStorageContext& c,
    NativeGuiTextGlyphQuadStorageAcquired& a) {
    const auto* const failure_runtime = c.failure_runtime;
    if (failure_runtime) {
        a.failure_origin_.begin(*failure_runtime, &a, 0x00ab98f0u);
        a.failure_origin_.host(*failure_runtime, {0, __COUNTER__}, &a, [&] {
            if (a.phase != Acquired::Phase::fresh)
                throw std::logic_error("Glyph quad requires a fresh retained invocation record");
        });
    } else if (a.phase != Acquired::Phase::fresh)
        throw std::logic_error("Glyph quad requires a fresh retained invocation record");
    a.actual_text = actual_text;
    a.home_identity = homes;
    a.phase = Acquired::Phase::entered;
    const Word text = bits(actual_text);
    try {
        a.active_call_site = 0x00ab9912;
        prefix(actual_text, homes, c, a);
        if (word(text, 0x1b0) == 0 || word(text, 0x1ac) == 0 ||
            word(text, 0xdc) != 0 || word(text, 0x1a4) == 0) {
            if (failure_runtime) a.failure_origin_.complete();
            a.phase = Acquired::Phase::complete; return;
        }
        Word codes = word(text, 0x1a8);
        if (codes == 0) codes = bits(c.actual_empty_utf16_00f8be50);
        Word code = half(codes, 0);
        if (code == 0) {
            if (failure_runtime) a.failure_origin_.complete();
            a.phase = Acquired::Phase::complete; return;
        }
        for (;;) {
            if (half(bits(homes), 36) == code) break;
            code = half(codes, 2);
            codes += 2u;
            if (code == 0) {
                if (failure_runtime) a.failure_origin_.complete();
                a.phase = Acquired::Phase::complete; return;
            }
        }
        clear_uv(a.captured_stream, a.captured_first_vertex);
        clear_uv_home(a.captured_stream, &homes[0]);
        clear_uv_home(a.captured_stream, &homes[2]);
        clear_uv(a.captured_stream, a.captured_fourth_vertex);

        boundary(a, 0x00ab9d38, c.actual_text_pool_00f8bdf0);
        void* const allocation = c.calls.allocate_00ab78a0(c.actual_text_pool_00f8bdf0, a);
        a.captured_allocation = allocation;
        homes[7] = bits(allocation);
        a.native_eh_state = 0;
        if (failure_runtime)
            a.constructor_slot_.arm(a.failure_origin_, *failure_runtime, &homes[7], allocation,
                &c.calls, {0x00ab9d4fu, __COUNTER__}, a.constructor_call_);
        void* child = nullptr;
        if (allocation) {
            boundary(a, 0x00ab9d4f, allocation);
            if (failure_runtime)
                child = a.constructor_call_.invoke([&] {
                    return c.calls.construct_00ab9650(allocation, a, a.constructor_call_);
                });
            else child = c.calls.construct_00ab9650(allocation, a);
        }
        a.captured_child = child;
        a.native_eh_state = -1;
        if (failure_runtime) a.constructor_slot_.normal_disarm(a.constructor_call_);
        homes[7] = bits(child);
        a.child_append_entered = true;
        boundary(a, 0x00ab9d70, pointer(text + 0x194));
        c.calls.append_005dad40(pointer(text + 0x194), &homes[7], a);
        a.child_append_returned = true;

        Word node = word(word(text, 0x1b0), 0x4c);
        current(a, node, 0x10);
        boundary(a, 0x00ab9d87, pointer(node));
        void* const cloned = c.calls.node_current10(pointer(node), a.captured_profile, a.captured_target, 0x26, 0, a);
        boundary(a, 0x00ab9d8c, child); c.calls.scene_node_00aa6720(child, cloned, a);
        boundary(a, 0x00ab9d93, child); c.calls.sections_00ab8530(child, a);
        current(a, bits(child), 0x34);
        boundary(a, 0x00ab9da5, child);
        c.calls.child_current34(child, a.captured_profile, a.captured_target, 1, a);
        void* const listener = pointer(word(text, 0x1ac));
        boundary(a, 0x00ab9db2, child); c.calls.listeners_00aa6bc0(child, listener, 0, a);
        *reinterpret_cast<volatile unsigned char*>(static_cast<std::uintptr_t>(bits(child) + 0x78)) = 1;
        const void* const font_name = pointer(word(text, 0x1b0) + 0x1c8);
        boundary(a, 0x00ab9dc9, child); c.calls.font_00ab8c30(child, font_name, a);

        Word shadow_offset;
        prepare_shadow(c, a, shadow_offset);
        boundary(a, 0x00ab9e05, child);
        c.calls.shadow_00ab6c30(child, 1, 0, shadow_offset, static_cast<volatile Word*>(a.locals) + 7, a);
        boundary(a, 0x00ab9e13, a.locals + 4);
        void* const shader_argument = construct_native_string_header_0041e870(
            a.locals + 4, c.strings, c.shader_name_00cefd78);
        a.native_eh_state = 1;
        boundary(a, 0x00ab9e1f, child); c.calls.shader_00ab8e70(child, shader_argument, a);
        normal_string_return(a, c.strings, 0x00ab9e39, 0x00ab9e40, false);
        const Word current_code_unit = homes[9];
        boundary(a, 0x00ab9e51, a.locals + 4);
        void* const content_argument = c.calls.code_unit_00ab81c0(a.locals + 4, current_code_unit, a);
        a.native_eh_state = 2;
        boundary(a, 0x00ab9e61, child); c.calls.content_00aba8d0(child, content_argument, a);
        current(a, bits(child), 0x50);
        boundary(a, 0x00ab9e71, child);
        c.calls.child_current50(child, a.captured_profile, a.captured_target, pointer(bits(child) + 0x50), a);
        normal_string_return(a, c.strings, 0x00ab9e8a, 0x00ab9e91, true);
        boundary(a, 0x00ab9e9b, actual_text); c.calls.attach_00aaa5a0(actual_text, child, 0, a);
        volatile Word* const scratch = a.locals;
        const volatile float* const pivot = &c.pivot_00ce3800;
        __asm {
            mov eax, pivot
            movss xmm0, dword ptr [eax]
            mov edx, scratch
            movss dword ptr [edx + 10h], xmm0
            movss dword ptr [edx + 14h], xmm0
        }
        current(a, bits(child), 0x30);
        boundary(a, 0x00ab9ec0, child);
        c.calls.child_current30(child, a.captured_profile, a.captured_target, static_cast<volatile Word*>(a.locals) + 4, a);
        void* const reference = pointer(word(text, 0x1b0));
        boundary(a, 0x00ab9ecd, reference);
        c.calls.position_00aa6750(reference, static_cast<volatile Word*>(a.locals) + 1, a);
        volatile Word* target;
        prepare_position(text, homes, c, a, target);
        boundary(a, 0x00ab9fa9, child); c.calls.position_00aa8240(child, target, a);
        if (failure_runtime) a.failure_origin_.complete();
        a.phase = Acquired::Phase::complete;
    } catch (...) {
        bool constructor_permit = false;
        if (failure_runtime) {
            a.failure_origin_.retain_current(*failure_runtime);
            if (a.native_eh_state == 0)
                constructor_permit = a.constructor_slot_.consume_proven_primary(
                    a.failure_origin_, *failure_runtime, a.constructor_call_, &homes[7]);
        }
        a.escaped_call_site = a.active_call_site;
        a.escaped_eh_state = a.native_eh_state;
        a.failure = std::current_exception();
        a.phase = Acquired::Phase::failed;
        const int state = a.native_eh_state;
        a.native_eh_state = -1;
        // Qualified state0 cannot reach cleanup for HOST/UNKNOWN or body-returned.
        // Legacy states1/2 retain their historical, separately gated projection.
        if (state >= 0 && (state != 0 || !failure_runtime || constructor_permit)) {
            a.cleanup_entered = true;
            try {
                if (state == 0) {
                    void* const current_slot = pointer(homes[7]);
                    boundary(a, 0x00cb80b3, current_slot);
                    if (failure_runtime)
                        a.failure_origin_.unknown(*failure_runtime, {0x00cb80b3u, __COUNTER__}, &c.calls,
                            [&] { c.calls.return_failed_00ab76f0(current_slot, a); },
                            NativeGuiTextFailureOrigin::Stage::cleanup_secondary);
                    else c.calls.return_failed_00ab76f0(current_slot, a);
                } else if (state == 1) {
                    boundary(a, 0x00cb80bb, a.locals + 4);
                    destroy_native_string_header_0041dd20(a.locals + 4, c.strings);
                } else {
                    boundary(a, 0x00cb80c3, a.locals + 4);
                    destroy_native_wide_string_header_00436430(a.locals + 4, c.strings);
                }
                a.cleanup_returned = true;
            } catch (...) { a.cleanup_failed = true; throw; }
        }
        throw;
    }
}
} // namespace bsp
