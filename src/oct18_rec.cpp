#include "bsp/oct18_rec.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These original x87 sequences require MSVC Win32.
#endif

namespace bsp::oct18_rec {
__declspec(naked) void __fastcall radix_noop_00403c40(void*, void*, std::uint32_t) {
    __asm { ret 4 }
}
__declspec(naked) void __fastcall terrain_noop_00404540(
    void*, void*, void*, const void*, const void*) {
    __asm { ret 0ch }
}
__declspec(naked) void __fastcall terrain_noop_00404550(
    void*, void*, void*, const void*, const void*) {
    __asm { ret 0ch }
}
__declspec(naked) void __fastcall triangle_noop_00405aa0(void*) {
    __asm { ret }
}

namespace {
template<std::uint32_t Depth, std::uint32_t Trailer, std::uint32_t Stride>
void reclaim_empty_pages(Pool& pool, Free release) {
    std::uint32_t i = 0;
    while (i < pool.page_count_2c) {
        auto* const page = pool.pages_28[i];
        if (*reinterpret_cast<const std::uint16_t*>(page + Depth) == 0x80) {
            release(page);
            // Re-read the table/count after the callback, in original order.
            pool.pages_28[i] = pool.pages_28[pool.page_count_2c - 1];
            --pool.page_count_2c;
            if (i < pool.page_count_2c) {
                auto* slot = pool.pages_28[i] + Trailer;
                for (std::uint32_t remaining = 0x80; remaining; --remaining) {
                    *reinterpret_cast<std::uint32_t*>(slot) = i;
                    slot += Stride;
                }
            }
            --i; // Original unsigned SUB/ADD retries the moved page.
        }
        ++i;
    }
    pool.first_free_page_34 = 0xffffffffu;
    for (std::uint32_t j = 0; j < pool.page_count_2c; ++j) {
        if (*reinterpret_cast<const std::uint16_t*>(pool.pages_28[j] + Depth) != 0) {
            pool.first_free_page_34 = j;
            return;
        }
    }
}
} // namespace

void pool36_reclaim_empty_pages_00407750(Pool& pool, Free release) {
    reclaim_empty_pages<0x1500, 0x24, 0x28>(pool, release);
}
void pool20_reclaim_empty_pages_00407bb0(Pool& pool, Free release) {
    reclaim_empty_pages<0xd00, 0x14, 0x18>(pool, release);
}
void store_word_00408300(std::uint32_t* destination, std::uint32_t value) noexcept {
    *destination = value;
}
std::uint32_t load_word_00409d00(const std::uint32_t* source) noexcept {
    return *source;
}
void vector_free_00408e00(void* const* header, Free release) {
    void* const data = *header;
    if (data) release(data);
}
void* vector_construct_empty_0040ab80(std::uint32_t* header) noexcept {
    header[0] = 0;
    header[1] = 0;
    header[2] = 0;
    return header;
}
void vector_free_0040ab90(void* const* header, Free release) {
    void* const data = *header;
    if (data) release(data);
}
} // namespace bsp::oct18_rec

namespace bsp::oct18_rec {
namespace {
template<class T> T raw_read(const void* p, std::uint32_t offset) {
    return *reinterpret_cast<const volatile T*>(
        static_cast<const std::byte*>(p) + offset);
}
template<class T> void raw_write(void* p, std::uint32_t offset, T value) {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(p) + offset) = value;
}
void unwind_cleanup(void (*cleanup)(void*), void* p) noexcept {
    // A destructor throwing while the original CRT unwinds terminates.
    cleanup(p);
}
} // namespace

void set_xbox_mode_008d44c0(void* settings, std::uint8_t enabled,
    volatile std::uint8_t& mirror, void* volatile& game_publication,
    RunLuaString run, const char* command_true, const char* command_false) {
    raw_write(settings, 0xb0, enabled);
    mirror = enabled;
    void* const game = game_publication;
    if (!game || !raw_read<void*>(game, 0x1a08)) return;
    // The original reloads +1A08h after the null test.
    void* const owner = raw_read<void*>(game, 0x1a08);
    void* const machine = raw_read<void*>(owner, 4);
    if (!machine) return;
    const bool current = raw_read<std::uint8_t>(settings, 0xb0) != 0;
    run(machine, current ? command_true : command_false, 0, 0, 2);
}

bool mission_completed_0090c560(void* table, const void* key,
    FindMission find, InvalidParameter invalid) {
    MissionIterator result; // The original find initializes both stack words.
    find(table, &result, key);
    void* const owner = result.owner;
    void* const end = raw_read<void*>(table, 4);
    if (!owner || owner != table) invalid();
    void* const node = result.node;
    if (node == end) return false;
    if (!owner) invalid();
    if (node == raw_read<void*>(owner, 4)) invalid();
    return raw_read<std::int32_t>(node, 0x14) != 0;
}

char* read_quoted_runs_008efb00(void* tokenizer, const QuotedRunCalls& calls) {
    alignas(4) std::byte writer[0x20]; // Preserve unspecified stack bytes.
    raw_write<std::uint32_t>(writer, 8, 0);
    raw_write<std::uint32_t>(writer, 0xc, 2);
    raw_write<std::uint8_t>(writer, 0x10, 0x20);
    raw_write<std::uint8_t>(writer, 4, 0);
    raw_write<std::uint32_t>(writer, 0, 0x00d162cc);
    raw_write<std::uint32_t>(writer, 0x18, 0);
    raw_write<std::uint32_t>(writer, 0x14, 0);
    raw_write<std::uint32_t>(writer, 0x1c, 0);
    char* joined;
    try { // FuncInfo 00DD72B4 state 0 -> unwind funclet 00CA4700.
        calls.reset_009502e0(writer);
        calls.peek_008d8a70(tokenizer);
        while (raw_read<std::uint8_t>(tokenizer, 4) != 0) {
            const char* token = calls.read_008d9980(tokenizer);
            const char* decoded = calls.decode_008ee670(token);
            calls.append_0094ff10(writer, decoded);
            calls.peek_008d8a70(tokenizer);
        }
        joined = calls.join_009500a0(writer);
    } catch (...) {
        unwind_cleanup(calls.destroy_reader_008ef030, writer);
        throw;
    }
    raw_write<std::uint32_t>(writer, 0, 0x00d162cc);
    try { // State 1 -> 00CA4708: base only if normal clear throws.
        calls.clear_00950320(writer);
    } catch (...) {
        unwind_cleanup(calls.destroy_base_0045c4c0, writer);
        throw;
    }
    return joined;
}
} // namespace bsp::oct18_rec

namespace bsp::oct18_rec {
// Select the furthest of three vertices after transforming the direction.
// Keep every original x87 register operation, binary32 spill and JBE tie/NaN
// branch. The double overload narrows its direction and final coordinates
// through binary32 before widening outputs; it is not double-precision math.
__declspec(naked) void __fastcall triangle_support_float_00405bf0(
    const void*, void*, float*, const float*, const float*) {
    __asm {
        sub esp, 0x14 // 00405bf0
        mov edx, dword ptr [esp + 0x20] // 00405bf3
        mov eax, dword ptr [esp + 0x1c] // 00405bf7
        fld dword ptr [eax + 4] // 00405bfb
        push esi // 00405bfe
        fstp dword ptr [esp + 0x24] // 00405bff
        fld dword ptr [eax] // 00405c03
        fstp dword ptr [esp + 4] // 00405c05
        fld dword ptr [edx] // 00405c09
        fstp dword ptr [esp + 0x20] // 00405c0b
        fld dword ptr [eax + 8] // 00405c0f
        fstp dword ptr [esp + 8] // 00405c12
        fld dword ptr [edx + 4] // 00405c16
        fld dword ptr [esp + 0x24] // 00405c19
        fld st(0) // 00405c1d
        fmulp st(2), st(0) // 00405c1f
        fld dword ptr [esp + 0x20] // 00405c21
        fld st(0) // 00405c25
        fld dword ptr [esp + 4] // 00405c27
        fld st(0) // 00405c2b
        fmulp st(2), st(0) // 00405c2d
        fxch st(4) // 00405c2f
        faddp st(1), st(0) // 00405c31
        fld dword ptr [edx + 8] // 00405c33
        fld dword ptr [esp + 8] // 00405c36
        fld st(0) // 00405c3a
        fmulp st(2), st(0) // 00405c3c
        fxch st(2) // 00405c3e
        faddp st(1), st(0) // 00405c40
        fstp dword ptr [esp + 0xc] // 00405c42
        fld dword ptr [edx + 0x10] // 00405c46
        fmul st(0), st(3) // 00405c49
        fld dword ptr [edx + 0xc] // 00405c4b
        fmul st(0), st(5) // 00405c4e
        faddp st(1), st(0) // 00405c50
        fld dword ptr [edx + 0x14] // 00405c52
        fmul st(0), st(2) // 00405c55
        faddp st(1), st(0) // 00405c57
        fstp dword ptr [esp + 0x10] // 00405c59
        fld dword ptr [edx + 0x1c] // 00405c5d
        fmulp st(3), st(0) // 00405c60
        fld dword ptr [edx + 0x18] // 00405c62
        fmulp st(4), st(0) // 00405c65
        fxch st(2) // 00405c67
        faddp st(3), st(0) // 00405c69
        fld dword ptr [edx + 0x20] // 00405c6b
        fmulp st(2), st(0) // 00405c6e
        fxch st(2) // 00405c70
        faddp st(1), st(0) // 00405c72
        fstp dword ptr [esp + 0x14] // 00405c74
        fld dword ptr [ecx + 0x214] // 00405c78
        fld dword ptr [esp + 0x10] // 00405c7e
        fld st(0) // 00405c82
        fmulp st(2), st(0) // 00405c84
        fld dword ptr [ecx + 0x210] // 00405c86
        fld dword ptr [esp + 0xc] // 00405c8c
        fld st(0) // 00405c90
        fmulp st(2), st(0) // 00405c92
        fxch st(3) // 00405c94
        faddp st(1), st(0) // 00405c96
        fld dword ptr [ecx + 0x218] // 00405c98
        fld dword ptr [esp + 0x14] // 00405c9e
        fld st(0) // 00405ca2
        fmulp st(2), st(0) // 00405ca4
        fxch st(2) // 00405ca6
        faddp st(1), st(0) // 00405ca8
        fstp dword ptr [esp + 0xc] // 00405caa
        fld dword ptr [ecx + 0x220] // 00405cae
        fmul st(0), st(2) // 00405cb4
        fld dword ptr [ecx + 0x21c] // 00405cb6
        fmul st(0), st(4) // 00405cbc
        faddp st(1), st(0) // 00405cbe
        fld dword ptr [ecx + 0x224] // 00405cc0
        fmul st(0), st(2) // 00405cc6
        faddp st(1), st(0) // 00405cc8
        fstp dword ptr [esp + 0x10] // 00405cca
        fld dword ptr [ecx + 0x22c] // 00405cce
        fmulp st(2), st(0) // 00405cd4
        fld dword ptr [ecx + 0x228] // 00405cd6
        fmulp st(3), st(0) // 00405cdc
        fxch st(1) // 00405cde
        faddp st(2), st(0) // 00405ce0
        fmul dword ptr [ecx + 0x230] // 00405ce2
        xor esi, esi // 00405ce8
        faddp st(1), st(0) // 00405cea
        fstp dword ptr [esp + 0x14] // 00405cec
        fld dword ptr [esp + 0xc] // 00405cf0
        fld dword ptr [esp + 0x10] // 00405cf4
        fcomip st(0), st(1) // 00405cf8
        fstp st(0) // 00405cfa
        jbe tri_00405d03 // 00405cfc
        mov esi, 1 // 00405cfe
    tri_00405d03:
        fld dword ptr [esp + esi*4 + 0xc] // 00405d03
        fld dword ptr [esp + 0x14] // 00405d07
        fcomip st(0), st(1) // 00405d0b
        fstp st(0) // 00405d0d
        jbe tri_00405d16 // 00405d0f
        mov esi, 2 // 00405d11
    tri_00405d16:
        lea eax, [esi + esi*2 + 0x84] // 00405d16
        fmul dword ptr [ecx + eax*4] // 00405d1d
        lea eax, [ecx + eax*4] // 00405d20
        fld dword ptr [eax + 4] // 00405d23
        mov ecx, dword ptr [esp + 0x1c] // 00405d26
        fmul dword ptr [edx + 0xc] // 00405d2a
        pop esi // 00405d2d
        faddp st(1), st(0) // 00405d2e
        fld dword ptr [eax + 8] // 00405d30
        fmul dword ptr [edx + 0x18] // 00405d33
        faddp st(1), st(0) // 00405d36
        fadd dword ptr [edx + 0x24] // 00405d38
        fstp dword ptr [ecx] // 00405d3b
        fld dword ptr [edx + 0x10] // 00405d3d
        fmul dword ptr [eax + 4] // 00405d40
        fld dword ptr [eax] // 00405d43
        fmul dword ptr [edx + 4] // 00405d45
        faddp st(1), st(0) // 00405d48
        fld dword ptr [edx + 0x1c] // 00405d4a
        fmul dword ptr [eax + 8] // 00405d4d
        faddp st(1), st(0) // 00405d50
        fadd dword ptr [edx + 0x28] // 00405d52
        fstp dword ptr [ecx + 4] // 00405d55
        fld dword ptr [edx + 0x14] // 00405d58
        fmul dword ptr [eax + 4] // 00405d5b
        fld dword ptr [edx + 8] // 00405d5e
        fmul dword ptr [eax] // 00405d61
        faddp st(1), st(0) // 00405d63
        fld dword ptr [edx + 0x20] // 00405d65
        fmul dword ptr [eax + 8] // 00405d68
        faddp st(1), st(0) // 00405d6b
        fadd dword ptr [edx + 0x2c] // 00405d6d
        fstp dword ptr [ecx + 8] // 00405d70
        add esp, 0x14 // 00405d73
        ret 0xc // 00405d76
    }
}
__declspec(naked) void __fastcall triangle_support_double_00405d80(
    const void*, void*, double*, const double*, const float*) {
    __asm {
        sub esp, 0x14 // 00405d80
        mov eax, dword ptr [esp + 0x1c] // 00405d83
        mov edx, dword ptr [esp + 0x20] // 00405d87
        fld qword ptr [eax] // 00405d8b
        fstp dword ptr [esp + 8] // 00405d8d
        push esi // 00405d91
        fld qword ptr [eax + 8] // 00405d92
        fstp dword ptr [esp + 0x10] // 00405d95
        fld qword ptr [eax + 0x10] // 00405d99
        fstp dword ptr [esp + 0x14] // 00405d9c
        fld dword ptr [edx + 4] // 00405da0
        fstp dword ptr [esp + 0x24] // 00405da3
        fld dword ptr [edx] // 00405da7
        fstp dword ptr [esp + 4] // 00405da9
        fld dword ptr [edx + 8] // 00405dad
        fstp dword ptr [esp + 0x20] // 00405db0
        fld dword ptr [esp + 0x24] // 00405db4
        fld st(0) // 00405db8
        fld dword ptr [esp + 0x10] // 00405dba
        fld st(0) // 00405dbe
        fmulp st(2), st(0) // 00405dc0
        fld dword ptr [esp + 4] // 00405dc2
        fld st(0) // 00405dc6
        fld dword ptr [esp + 0xc] // 00405dc8
        fld st(0) // 00405dcc
        fmulp st(2), st(0) // 00405dce
        fxch st(4) // 00405dd0
        faddp st(1), st(0) // 00405dd2
        fld dword ptr [esp + 0x20] // 00405dd4
        fld dword ptr [esp + 0x14] // 00405dd8
        fld st(0) // 00405ddc
        fmulp st(2), st(0) // 00405dde
        fxch st(2) // 00405de0
        faddp st(1), st(0) // 00405de2
        fstp dword ptr [esp + 0xc] // 00405de4
        fld dword ptr [edx + 0x10] // 00405de8
        fmul st(0), st(3) // 00405deb
        fld dword ptr [edx + 0xc] // 00405ded
        fmul st(0), st(5) // 00405df0
        faddp st(1), st(0) // 00405df2
        fld dword ptr [edx + 0x14] // 00405df4
        fmul st(0), st(2) // 00405df7
        faddp st(1), st(0) // 00405df9
        fstp dword ptr [esp + 0x10] // 00405dfb
        fld dword ptr [edx + 0x1c] // 00405dff
        fmulp st(3), st(0) // 00405e02
        fld dword ptr [edx + 0x18] // 00405e04
        fmulp st(4), st(0) // 00405e07
        fxch st(2) // 00405e09
        faddp st(3), st(0) // 00405e0b
        fld dword ptr [edx + 0x20] // 00405e0d
        fmulp st(2), st(0) // 00405e10
        fxch st(2) // 00405e12
        faddp st(1), st(0) // 00405e14
        fstp dword ptr [esp + 0x14] // 00405e16
        fld dword ptr [ecx + 0x214] // 00405e1a
        fld dword ptr [esp + 0x10] // 00405e20
        fld st(0) // 00405e24
        fmulp st(2), st(0) // 00405e26
        fld dword ptr [ecx + 0x210] // 00405e28
        fld dword ptr [esp + 0xc] // 00405e2e
        fld st(0) // 00405e32
        fmulp st(2), st(0) // 00405e34
        fxch st(3) // 00405e36
        faddp st(1), st(0) // 00405e38
        fld dword ptr [ecx + 0x218] // 00405e3a
        fld dword ptr [esp + 0x14] // 00405e40
        fld st(0) // 00405e44
        fmulp st(2), st(0) // 00405e46
        fxch st(2) // 00405e48
        faddp st(1), st(0) // 00405e4a
        fstp dword ptr [esp + 0xc] // 00405e4c
        fld dword ptr [ecx + 0x220] // 00405e50
        fmul st(0), st(2) // 00405e56
        fld dword ptr [ecx + 0x21c] // 00405e58
        fmul st(0), st(4) // 00405e5e
        faddp st(1), st(0) // 00405e60
        fld dword ptr [ecx + 0x224] // 00405e62
        fmul st(0), st(2) // 00405e68
        faddp st(1), st(0) // 00405e6a
        fstp dword ptr [esp + 0x10] // 00405e6c
        fld dword ptr [ecx + 0x22c] // 00405e70
        fmulp st(2), st(0) // 00405e76
        xor esi, esi // 00405e78
        fld dword ptr [ecx + 0x228] // 00405e7a
        fmulp st(3), st(0) // 00405e80
        fxch st(1) // 00405e82
        faddp st(2), st(0) // 00405e84
        fmul dword ptr [ecx + 0x230] // 00405e86
        faddp st(1), st(0) // 00405e8c
        fstp dword ptr [esp + 0x14] // 00405e8e
        fld dword ptr [esp + 0xc] // 00405e92
        fld dword ptr [esp + 0x10] // 00405e96
        fcomip st(0), st(1) // 00405e9a
        fstp st(0) // 00405e9c
        jbe tri_00405ea5 // 00405e9e
        mov esi, 1 // 00405ea0
    tri_00405ea5:
        fld dword ptr [esp + esi*4 + 0xc] // 00405ea5
        fld dword ptr [esp + 0x14] // 00405ea9
        fcomip st(0), st(1) // 00405ead
        fstp st(0) // 00405eaf
        jbe tri_00405eb8 // 00405eb1
        mov esi, 2 // 00405eb3
    tri_00405eb8:
        lea eax, [esi + esi*2 + 0x84] // 00405eb8
        fld dword ptr [ecx + eax*4 + 4] // 00405ebf
        lea eax, [ecx + eax*4] // 00405ec3
        fstp dword ptr [esp + 0x24] // 00405ec6
        pop esi // 00405eca
        fld dword ptr [eax] // 00405ecb
        fstp dword ptr [esp] // 00405ecd
        fld dword ptr [eax + 8] // 00405ed0
        mov eax, dword ptr [esp + 0x18] // 00405ed3
        fstp dword ptr [esp + 4] // 00405ed7
        fld dword ptr [esp + 0x20] // 00405edb
        fld st(0) // 00405edf
        fmul dword ptr [edx + 0xc] // 00405ee1
        fld dword ptr [esp] // 00405ee4
        fld st(0) // 00405ee7
        fmulp st(4), st(0) // 00405ee9
        fxch st(1) // 00405eeb
        faddp st(3), st(0) // 00405eed
        fld dword ptr [edx + 0x18] // 00405eef
        fld dword ptr [esp + 4] // 00405ef2
        fld st(0) // 00405ef6
        fmulp st(2), st(0) // 00405ef8
        fxch st(4) // 00405efa
        faddp st(1), st(0) // 00405efc
        fadd dword ptr [edx + 0x24] // 00405efe
        fstp dword ptr [esp + 8] // 00405f01
        fld dword ptr [edx + 0x10] // 00405f05
        fmul st(0), st(2) // 00405f08
        fld st(1) // 00405f0a
        fmulp st(5), st(0) // 00405f0c
        faddp st(4), st(0) // 00405f0e
        fld dword ptr [edx + 0x1c] // 00405f10
        fmul st(0), st(3) // 00405f13
        faddp st(4), st(0) // 00405f15
        fld dword ptr [edx + 0x28] // 00405f17
        faddp st(4), st(0) // 00405f1a
        fxch st(3) // 00405f1c
        fstp dword ptr [esp + 0xc] // 00405f1e
        fmul dword ptr [edx + 0x14] // 00405f22
        fld dword ptr [esp + 0x1c] // 00405f25
        fmulp st(3), st(0) // 00405f29
        faddp st(2), st(0) // 00405f2b
        fmul dword ptr [edx + 0x20] // 00405f2d
        faddp st(1), st(0) // 00405f30
        fadd dword ptr [edx + 0x2c] // 00405f32
        fstp dword ptr [esp + 0x10] // 00405f35
        fld dword ptr [esp + 8] // 00405f39
        fld dword ptr [esp + 0xc] // 00405f3d
        fld dword ptr [esp + 0x10] // 00405f41
        fxch st(2) // 00405f45
        fstp qword ptr [eax] // 00405f47
        fstp qword ptr [eax + 8] // 00405f49
        fstp qword ptr [eax + 0x10] // 00405f4c
        add esp, 0x14 // 00405f4f
        ret 0xc // 00405f52
    }
}
} // namespace bsp::oct18_rec
