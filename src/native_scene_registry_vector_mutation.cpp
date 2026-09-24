#include "bsp/native_scene_registry_vector_mutation.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* ptr(Word v) noexcept { return reinterpret_cast<void*>(v); }
Word read(Word p, Word n = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(p + n);
}
void write(Word p, Word n, Word v) noexcept {
    *reinterpret_cast<volatile Word*>(p + n) = v;
}
Word distance(Word last, Word first) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(last - first) >> 3);
}
Word size(Word vector, Word captured_begin) noexcept {
    return captured_begin ? distance(read(vector, 8), captured_begin) : 0;
}
void low_zero(volatile Word& word) noexcept {
    *reinterpret_cast<volatile unsigned char*>(&word) = 0;
}
}

void* erase_native_scene_registry_pairs_00b82bf0(void* actual_vector,
    NativeSceneRegistryVectorEraseArguments& args,
    NativeSceneRegistryVectorMutationBindings& bindings) {
    Word owner = args.first_owner_04;
    const Word vector = bits(actual_vector);
    if (owner == 0 || owner != args.last_owner_0c)
        bindings.actual_00bf6713();
    const Word first = args.first_position_08;
    Word last = args.last_position_10;
    if (first != last) {
        const Word end = read(vector, 8);
        const Word new_end = first + distance(end, last) * 8u;
        if (last != end) {
            const Word delta = first - last;
            do {
                const Word a = read(last);
                write(last + delta, 0, a);
                const Word b = read(last, 4);
                write(last + delta, 4, b);
                last += 8u;
            } while (last != end);
            owner = args.first_owner_04; // B82C43 only after the loop
        }
        write(vector, 8, new_end);
    }
    const Word result = args.result_00; // after current vector end publication
    write(result, 4, first);
    write(result, 0, owner);
    return ptr(result);
}

void insert_native_scene_registry_pairs_00b82fd0(void* actual_vector,
    NativeSceneRegistryInsertFrame& frame,
    NativeSceneRegistryStorageBindings& bindings) {
    const Word pair = frame.pair_or_work_18;
    const Word second = read(pair, 4); // B82FEE before the first-word read
    const Word vector = bits(actual_vector);
    const Word first = read(pair);
    const Word begin = read(vector, 4); // before the local snapshot stores
    frame.pair_00[0] = first;
    frame.pair_00[1] = second;
    Word capacity = begin ? distance(read(vector, 0x0c), begin) : 0;
    const Word count = frame.count_14;
    if (count == 0) return;
    if (0x1fffffffu - size(vector, begin) < count)
        bindings.actual_00b82dd0(); // required genuine nonreturning failure
    const void* const snapshot = ptr(bits(frame.pair_00));
    if (capacity < size(vector, begin) + count) {
        if (0x1fffffffu - (capacity >> 1) < capacity) capacity = 0;
        else capacity += capacity >> 1;
        if (capacity < size(vector, begin) + count)
            capacity = size(vector, begin) + count;
        const Word allocated = bits(allocate_native_scene_registry_pairs_00b81b90(
            capacity, bindings));
        const Word position = frame.position_10;
        const Word copy_begin = read(vector, 4);
        low_zero(frame.copy_unused_08);
        const Word unused4 = frame.copy_unused_08;
        const Word unused3 = frame.count_14;
        frame.pair_or_work_18 = allocated; // B830B6, then native state0
        try {
            void* end = copy_native_scene_registry_iterators_00b821c0(
                ptr(copy_begin), ptr(position), ptr(allocated), vector,
                unused3, unused4);
            const Word fill_count = frame.count_14;
            end = fill_native_scene_registry_iterators_end_00b82b80(
                vector, nullptr, end, fill_count, snapshot);
            const Word tail_end = read(vector, 8);
            low_zero(frame.position_10);
            const Word tail_unused4 = frame.position_10;
            const Word tail_unused3 = frame.count_14;
            copy_native_scene_registry_iterators_00b821c0(ptr(position),
                ptr(tail_end), end, vector, tail_unused3, tail_unused4);
            const Word old_begin = read(vector, 4);
            const Word old_size = size(vector, old_begin);
            const Word new_size = frame.count_14 + old_size;
            if (old_begin) bindings.actual_00bf65ac(ptr(old_begin));
            const Word current_buffer = frame.pair_or_work_18;
            const Word new_capacity = current_buffer + capacity * 8u;
            const Word new_end = current_buffer + new_size * 8u;
            write(vector, 0x0c, new_capacity);
            write(vector, 8, new_end);
            write(vector, 4, current_buffer);
        } catch (...) {
            // B83137 frees the CURRENT argument, not the earlier EAX result.
            // Source C++ exception transport; no claim for native FH3 faults.
            bindings.actual_00bf65ac(ptr(frame.pair_or_work_18));
            throw;
        }
        return;
    }
    const Word old_end = read(vector, 8);
    const Word position = frame.position_10;
    if (distance(old_end, position) < count) {
        frame.pair_or_work_18 = count * 8u;
        copy_native_scene_registry_pairs_00b82c90(vector, nullptr,
            ptr(position), ptr(old_end), ptr(position + count * 8u));
        const Word current_end = read(vector, 8);
        const Word current_count = frame.count_14;
        const Word fill_count = current_count - distance(current_end, position);
        // Native state2 covers the fill/remaining stores; catch B83143 merely
        // rethrows. These genuine raw helpers are noexcept for valid memory.
        fill_native_scene_registry_iterators_end_00b82b80(vector, nullptr,
            ptr(current_end), fill_count, snapshot);
        const Word bytes = frame.pair_or_work_18;
        write(vector, 8, read(vector, 8) + bytes);
        const Word fill_end = read(vector, 8) - bytes;
        assign_native_scene_registry_pairs_00b824f0(ptr(position), ptr(fill_end),
            snapshot);
        return;
    }
    const Word bytes = count * 8u;
    const Word copy_begin = old_end - bytes;
    frame.pair_or_work_18 = bytes;
    frame.count_14 = copy_begin;
    void* const end = copy_native_scene_registry_pairs_00b82c90(vector, nullptr,
        ptr(copy_begin), ptr(old_end), ptr(old_end));
    const Word current_copy_begin = frame.count_14;
    write(vector, 8, bits(end));
    move_native_scene_registry_pairs_backward_00b82a90(ptr(position),
        ptr(current_copy_begin), ptr(old_end));
    const Word current_bytes = frame.pair_or_work_18;
    assign_native_scene_registry_pairs_00b824f0(ptr(position),
        ptr(position + current_bytes), snapshot);
}

__declspec(naked) void* __fastcall copy_native_scene_registry_pairs_00b82c90(
    Word, void*, const void*, const void*, void*) noexcept {
    __asm {
        push ecx
        mov edx, dword ptr [esp + 10h]
        mov byte ptr [esp], 0
        mov eax, dword ptr [esp]
        push eax
        mov eax, dword ptr [esp + 14h]
        push edx
        mov edx, dword ptr [esp + 14h]
        push ecx
        mov ecx, dword ptr [esp + 14h]
        push eax
        call copy_native_scene_registry_iterators_00b821c0
        pop ecx
        ret 0ch
    }
}
__declspec(naked) void* __cdecl move_native_scene_registry_pairs_backward_00b82a90(
    const void*, const void*, void*) noexcept {
    __asm {
        mov ecx, dword ptr [esp + 8]
        mov edx, dword ptr [esp + 0ch]
        push esi
        mov esi, dword ptr [esp + 8]
        mov eax, ecx
        sub eax, esi
        sar eax, 3
        add eax, eax
        add eax, eax
        add eax, eax
        push edi
        mov edi, eax
        mov eax, edx
        sub eax, edi
        cmp esi, ecx
        jz done
        sub edx, ecx
    next_pair:
        mov edi, dword ptr [ecx - 8]
        sub ecx, 8
        cmp ecx, esi
        mov dword ptr [edx + ecx], edi
        mov edi, dword ptr [ecx + 4]
        mov dword ptr [edx + ecx + 4], edi
        jnz next_pair
    done:
        pop edi
        pop esi
        ret
    }
}
__declspec(naked) void __cdecl assign_native_scene_registry_pairs_00b824f0(
    void*, void*, const void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov edx, dword ptr [esp + 8]
        cmp eax, edx
        jz done
        mov ecx, dword ptr [esp + 0ch]
        push esi
    next_pair:
        mov esi, dword ptr [ecx]
        mov dword ptr [eax], esi
        mov esi, dword ptr [ecx + 4]
        mov dword ptr [eax + 4], esi
        add eax, 8
        cmp eax, edx
        jnz next_pair
        pop esi
    done:
        ret
    }
}
} // namespace bsp
