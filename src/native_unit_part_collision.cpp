#include "bsp/native_unit_part_collision.hpp"
#include "bsp/native_alias_count_growth.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(address(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(address(p) + offset) = value;
}
void invalid(NativeUnitPartCollisionCallbacks c) {
    if (!c.invalid_parameter)
        throw std::logic_error("Part collision requires the actual invalid-parameter provider");
    c.invalid_parameter(c.context);
}
Word group_count(void* part, Word begin) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(read(part, 0x170) - begin) >> 4);
}
bool above(const Word* a, const Word* b) noexcept {
    unsigned char result;
    __asm {
        mov eax, b
        fld dword ptr [eax]
        mov eax, a
        fld dword ptr [eax]
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
void* last_value(void* list, NativeUnitPartCollisionCallbacks c) {
    const Word head = read(list, 4);
    const Word last = read(pointer(head), 4);
    if (last == head) invalid(c);
    if (last == read(list, 4)) invalid(c);
    return pointer(last + 8);
}
} // namespace

// The assembly below keeps the original x87 copies, quieting, operand order
// and alias-visible reloads. Numeric native addresses stay data, never calls.
void copy_native_part_bounds_00723170(const void* source, void* minimum, void* maximum) noexcept {
    __asm {
        mov ecx, source
        fld dword ptr [ecx + 38h]
        mov eax, minimum
        fstp dword ptr [eax]
        fld dword ptr [ecx + 3ch]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 40h]
        fstp dword ptr [eax + 8]
        mov eax, maximum
        fld dword ptr [ecx + 44h]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 48h]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 4ch]
        fstp dword ptr [eax + 8]
    }
}
void set_native_part_shape_bounds_0098aab0(void* shape, const void* minimum,
    const void* maximum) noexcept {
    __asm {
        mov ecx, shape
        mov eax, minimum
        fld dword ptr [eax]
        fstp dword ptr [ecx + 4]
        fld dword ptr [eax + 4]
        fstp dword ptr [ecx + 8]
        fld dword ptr [eax + 8]
        mov eax, maximum
        fstp dword ptr [ecx + 0ch]
        fld dword ptr [eax]
        fstp dword ptr [ecx + 10h]
        fld dword ptr [eax + 4]
        fstp dword ptr [ecx + 14h]
        fld dword ptr [eax + 8]
        fstp dword ptr [ecx + 18h]
    }
}
void copy_native_unit_part_shape_00711020(void* destination, const void* source) noexcept {
    if (!destination) return;
    __asm {
        mov eax, destination
        mov ecx, source
        fld dword ptr [ecx + 4]
        mov dword ptr [eax], 0ce89dch
        mov edx, [ecx + 1ch]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        mov [eax + 1ch], edx
        fstp dword ptr [eax + 8]
        mov dword ptr [eax], 0cfd768h
        fld dword ptr [ecx + 0ch]
        mov edx, [ecx + 20h]
        fstp dword ptr [eax + 0ch]
        mov [eax + 20h], edx
        fld dword ptr [ecx + 10h]
        fstp dword ptr [eax + 10h]
        fld dword ptr [ecx + 14h]
        fstp dword ptr [eax + 14h]
        fld dword ptr [ecx + 18h]
        mov ecx, [ecx + 24h]
        fstp dword ptr [eax + 18h]
        mov [eax + 24h], ecx
    }
}
void* buy_native_unit_part_shape_node_00711460(void* next, void* previous, const void* source) {
    void* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x30, 0x30});
    // The canonical allocator either returns real storage or throws.
    write(node, 0, address(next));
    write(node, 4, address(previous));
    copy_native_unit_part_shape_00711020(pointer(address(node) + 8), source);
    return node;
}

void set_native_spatial_local_bounds_0098a920(void* node, const void* minimum,
    const void* maximum, const volatile double& half) noexcept {
    float scratch[9];
    const volatile double* half_pointer = &half;
    __asm {
        mov ecx, node
        mov edx, half_pointer
        lea edi, scratch
        mov eax, minimum
        fld dword ptr [eax]
        fstp dword ptr [ecx + 010ch]
        fld dword ptr [eax + 4]
        fstp dword ptr [ecx + 0110h]
        fld dword ptr [eax + 8]
        mov eax, maximum
        fstp dword ptr [ecx + 0114h]
        fld dword ptr [eax]
        fstp dword ptr [ecx + 0118h]
        fld dword ptr [eax + 4]
        fstp dword ptr [ecx + 011ch]
        fld dword ptr [eax + 8]
        fstp dword ptr [ecx + 0120h]
        fld dword ptr [ecx + 010ch]
        fld qword ptr [edx]
        fmul st(1), st(0)
        fxch st(1)
        fstp dword ptr [edi + 0ch]
        fld dword ptr [ecx + 0110h]
        fmul st(0), st(1)
        fstp dword ptr [edi + 010h]
        fld dword ptr [ecx + 0114h]
        fmul st(0), st(1)
        fstp dword ptr [edi + 014h]
        fld dword ptr [ecx + 0118h]
        fmul st(0), st(1)
        fstp dword ptr [edi]
        fld dword ptr [ecx + 011ch]
        fmul st(0), st(1)
        fstp dword ptr [edi + 4]
        fld dword ptr [ecx + 0120h]
        fmul st(0), st(1)
        fstp dword ptr [edi + 8]
        fld dword ptr [edi]
        fsub dword ptr [edi + 0ch]
        fstp dword ptr [edi + 018h]
        fld dword ptr [edi + 4]
        fsub dword ptr [edi + 010h]
        fstp dword ptr [edi + 01ch]
        fld dword ptr [edi + 8]
        fsub dword ptr [edi + 014h]
        fstp dword ptr [edi + 020h]
        fld dword ptr [edi + 018h]
        fstp dword ptr [ecx + 0130h]
        fld dword ptr [edi + 01ch]
        fstp dword ptr [ecx + 0134h]
        fld dword ptr [edi + 020h]
        fstp dword ptr [ecx + 0138h]
        fld dword ptr [ecx + 010ch]
        fmul st(0), st(1)
        fstp dword ptr [edi + 0ch]
        fld dword ptr [ecx + 0110h]
        fmul st(0), st(1)
        fstp dword ptr [edi + 010h]
        fld dword ptr [ecx + 0114h]
        fmul st(0), st(1)
        fstp dword ptr [edi + 014h]
        fld dword ptr [ecx + 0118h]
        fmul st(0), st(1)
        fstp dword ptr [edi + 018h]
        fld dword ptr [ecx + 011ch]
        fmul st(0), st(1)
        fstp dword ptr [edi + 01ch]
        fmul dword ptr [ecx + 0120h]
        fstp dword ptr [edi + 020h]
        fld dword ptr [edi + 018h]
        fadd dword ptr [edi + 0ch]
        fstp dword ptr [edi]
        fld dword ptr [edi + 01ch]
        fadd dword ptr [edi + 010h]
        fstp dword ptr [edi + 4]
        fld dword ptr [edi + 020h]
        fadd dword ptr [edi + 014h]
        fstp dword ptr [edi + 8]
        fld dword ptr [edi]
        fstp dword ptr [ecx + 0124h]
        fld dword ptr [edi + 4]
        fstp dword ptr [ecx + 0128h]
        fld dword ptr [edi + 8]
        fstp dword ptr [ecx + 012ch]
    }
}

bool build_native_unit_part_collision_00712440(void* part,
    NativeUnitPartCollisionGlobals g, NativeUnitPartCollisionCallbacks c) {
    const Word minimum_seed = g.minimum_seed_00d7a248;
    const Word initial_set = read(part, 0x160);
    Word minimum[3]{minimum_seed, minimum_seed, minimum_seed};
    const Word maximum_seed = g.maximum_seed_00d7a244;
    Word maximum[3]{maximum_seed, maximum_seed, maximum_seed};
    void* const records = pointer(initial_set + 0x3c);
    Word cursor = read(records, 4);
    bool any = false;
    if (cursor > read(records, 8)) invalid(c);
    for (;;) {
        const Word current_set = read(part, 0x160);
        const Word end = read(pointer(current_set), 0x44);
        void* const current_records = pointer(current_set + 0x3c);
        if (read(current_records, 4) > end) invalid(c);
        if (!records || records != current_records) invalid(c);
        if (cursor == end) {
            if (!any) return false;
            set_native_spatial_local_bounds_0098a920(part, minimum, maximum, g.half_00d7a280);
            return true;
        }
        any = true;
        if (!records) invalid(c);
        if (cursor >= read(records, 8)) invalid(c);
        const Word saved_end = read(records, 8); // CMP precedes the token load.
        const Word token = read(pointer(cursor), 4);
        if (cursor >= saved_end) invalid(c);
        const Word bounds_identity = read(pointer(cursor));
        bool publish = true;
        Word begin = read(part, 0x16c);
        if (begin && group_count(part, begin) != 0) {
            Word index = 1, offset = 0x10;
            for (;;) {
                begin = read(part, 0x16c);
                if (!begin || index >= group_count(part, begin)) break;
                if (!begin || index >= group_count(part, begin)) invalid(c);
                void* const first_row = pointer(read(part, 0x16c) + offset);
                const Word first_end = read(first_row, 8);
                if (read(first_row, 4) > first_end) invalid(c);
                begin = read(part, 0x16c);
                if (!begin || index >= group_count(part, begin)) invalid(c);
                void* const end_row = pointer(read(part, 0x16c) + offset);
                const Word search_end = read(end_row, 8);
                if (read(end_row, 4) > search_end) invalid(c);
                begin = read(part, 0x16c);
                if (!begin || index >= group_count(part, begin)) invalid(c);
                void* const search_row = pointer(read(part, 0x16c) + offset);
                Word search = read(search_row, 4);
                if (search > read(search_row, 8)) invalid(c);
                while (search != search_end && read(pointer(search)) != token) search += 4;
                if (search_row != first_row) invalid(c);
                if (search != first_end) { publish = false; break; }
                ++index;
                offset += 0x10;
            }
        }
        if (cursor >= read(records, 8)) invalid(c);
        Word local_min[3], local_max[3];
        copy_native_part_bounds_00723170(pointer(read(pointer(cursor))), local_min, local_max);
        const Word x = g.initial_point_00f87574[0];
        const Word y = g.initial_point_00f87574[1];
        const Word z = g.initial_point_00f87574[2];
        Word shape[10]{0x00cfd768, x, y, z, x, y, z, address(part), token, bounds_identity};
        void* const list = pointer(address(part) + (publish ? 0x188u : 0x194u));
        void* const head = pointer(read(list, 4));
        void* const previous = pointer(read(head, 4));
        void* const node = buy_native_unit_part_shape_node_00711460(head, previous, shape);
        // Original allocation precedes count growth; no caller unwind frees an
        // unlinked allocation if length-error construction/throw interrupts it.
        grow_native_unit_part_shape_count_00711ed0(list, 1);
        write(head, 4, address(node));
        write(pointer(read(node, 4)), 0, address(node));
        set_native_part_shape_bounds_0098aab0(last_value(list, c), local_min, local_max);
        if (publish) {
            void* const value = last_value(list, c);
            const Word count = read(part, 0xf8);
            write(part, 0xd0 + count * 4, address(value));
            write(part, 0xf8, read(part, 0xf8) + 1);
        }
        for (unsigned i = 0; i < 3; ++i)
            if (above(minimum + i, local_min + i)) minimum[i] = local_min[i];
        for (unsigned i = 0; i < 3; ++i)
            if (above(local_max + i, maximum + i)) maximum[i] = local_max[i];
        if (cursor >= read(records, 8)) invalid(c);
        cursor += 8;
    }
}
} // namespace bsp
