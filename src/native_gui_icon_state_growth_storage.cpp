#include "bsp/native_gui_icon_state_growth_storage.hpp"
#include "bsp/native_gui_icon_state_allocation_storage.hpp"
#include "bsp/native_gui_icon_state_range_lifetime_storage.hpp"
#include "bsp/native_gui_icon_state_repeat_wrapper_storage.hpp"
#include "bsp/native_gui_icon_state_range_copy_wrapper_storage.hpp"
#include "bsp/native_gui_icon_state_backward_assignment_storage.hpp"
#include "bsp/native_gui_icon_state_fill_storage.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <new>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This normal raw growth supplier requires MSVC Win32 DWORD/asm contracts.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
static_assert(std::is_trivially_default_constructible_v<NativeGuiIconStateStorage>);
static_assert(std::is_trivially_destructible_v<NativeGuiIconStateStorage>);
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word get(const Word& p) noexcept { return static_cast<const volatile Word&>(p); }
void set(Word& p, Word value) noexcept { static_cast<volatile Word&>(p) = value; }
void byte(Word& p, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(&p) = value;
}
Word read(void* object, Word byte_offset) noexcept {
    Word result;
    __asm {
        mov edx, object
        add edx, byte_offset
        mov eax, dword ptr [edx]
        mov result, eax
    }
    return result;
}
void write(void* object, Word byte_offset, Word value) noexcept {
    __asm {
        mov edx, object
        add edx, byte_offset
        mov eax, value
        mov dword ptr [edx], eax
    }
}
Word sar6(Word value) noexcept {
    Word result;
    __asm {
        mov eax, value
        sar eax, 6
        mov result, eax
    }
    return result;
}
} // namespace

void insert_native_gui_icon_states_storage_00ab59e0(void* actual_vector,
    volatile Word (&homes)[4], NativeGuiIconStateGrowthStorageFrame& f,
    NativeGuiIconStateGrowthStorageContext& c) {
    // Ordinary private Source captures, not a register/stack ABI replica.
    Word eax, ecx, edx, ebx, edi, vector;
    const Word temp = bits(&f.temporary_58);
    set(f.state_04, 0xffffffffu);
    eax = homes[3];
    vector = bits(actual_vector);
    set(f.receiver_14, vector);
    ::new (static_cast<void*>(&f.temporary_58)) NativeGuiIconStateStorage;
    copy_native_gui_icon_state_00ab3a70(&f.temporary_58, pointer(eax), c.storage);
    ecx = read(pointer(vector), 4);
    eax = 0;
    set(f.state_04, 0);
    if (ecx != 0) ebx = sar6(read(pointer(vector), 0x0c) - ecx);
    else ebx = 0;
    edi = homes[2];
    if (edi == 0) goto normal_end;
    if (ecx != 0) eax = sar6(read(pointer(vector), 8) - ecx);
    edx = 0x03ffffffu - eax;
    if (edx < edi) native_singleton_length_error_00bd0590(); // AB4A70 equivalent accepted service.
    if (ecx != 0) eax = sar6(read(pointer(vector), 8) - ecx);
    else eax = 0;
    eax += edi;
    if (ebx >= eax) goto inplace;

    eax = ebx >> 1u;
    edx = 0x03ffffffu - eax;
    if (edx < ebx) ebx = 0;
    else ebx += eax;
    if (ecx != 0) eax = sar6(read(pointer(vector), 8) - ecx);
    else eax = 0;
    eax += edi;
    if (ebx < eax) {
        if (ecx != 0) eax = sar6(read(pointer(vector), 8) - ecx);
        else eax = 0;
        ebx = eax + edi;
    }
    eax = bits(allocate_native_gui_icon_state_backing_00ab1520(ebx));
    ecx = read(pointer(vector), 4); // FRESH after allocator callbacks.
    byte(f.scratch_18, 0);
    edx = get(f.scratch_18);
    set(f.direct_range_homes[3], edx);
    homes[2] = eax;
    edx = homes[2];
    set(f.direct_range_homes[2], edx);
    edx = homes[1];
    set(f.direct_range_homes[1], vector);
    set(f.direct_range_homes[0], eax);
    homes[3] = eax;
    byte(f.state_04, 1);
    eax = construct_native_gui_icon_state_range_storage_00ab4c50(
        pointer(ecx), pointer(edx), f.direct_range_homes, f.direct_range_frame, c.storage);
    set(f.repeat_outer_homes[2], temp);
    set(f.repeat_outer_homes[1], edi);
    set(f.repeat_outer_homes[0], eax);
    homes[2] = eax;
    eax = repeat_native_gui_icon_states_wrapper_storage_00ab56c0(vector,
        f.repeat_outer_homes, f.repeat_tag_home, f.repeat_callee_homes, f.repeat_frame, c.storage);
    edx = read(pointer(vector), 8); // CURRENT last, not original last.
    byte(f.scratch_18, 0);
    ecx = get(f.scratch_18);
    set(f.direct_range_homes[3], ecx);
    homes[2] = eax;
    ecx = homes[2];
    set(f.direct_range_homes[2], ecx);
    ecx = homes[1];
    set(f.direct_range_homes[1], vector);
    set(f.direct_range_homes[0], eax);
    eax = construct_native_gui_icon_state_range_storage_00ab4c50(
        pointer(ecx), pointer(edx), f.direct_range_homes, f.direct_range_frame, c.storage);
    ecx = read(pointer(vector), 4);
    set(f.state_04, 0);
    if (ecx != 0) eax = sar6(read(pointer(vector), 8) - ecx);
    else eax = 0;
    edi += eax;
    if (ecx != 0) {
        edx = read(pointer(vector), 8);
        set(f.destroy_homes[1], edx);
        set(f.destroy_homes[0], ecx);
        destroy_native_gui_icon_state_range_storage_00ab57e0(f.destroy_homes, c.storage);
        eax = read(pointer(vector), 4); // CURRENT first after all destructor callbacks.
        singleton_lifetime_free(pointer(eax));
    }
    eax = homes[3];
    ebx = (ebx << 6u) + eax;
    edi = (edi << 6u) + eax;
    write(pointer(vector), 0x0c, ebx);
    write(pointer(vector), 8, edi);
    write(pointer(vector), 4, eax);
    goto normal_end;

inplace:
    eax = read(pointer(vector), 8);
    ebx = homes[1];
    ecx = sar6(eax - ebx);
    set(f.scratch_18, eax);
    if (ecx >= edi) goto tail_ge_count;
    ecx = edi << 6u;
    homes[3] = ecx;
    ecx += ebx;
    set(f.range_outer_homes[2], ecx);
    set(f.range_outer_homes[1], eax);
    set(f.range_outer_homes[0], ebx);
    eax = copy_native_gui_icon_state_range_wrapper_storage_00ab58a0(vector,
        f.range_outer_homes, f.range_tag_home, f.range_callee_homes, f.wrapped_range_frame, c.storage);
    eax = read(pointer(vector), 8); // CURRENT after suffix construction/callbacks.
    ecx = sar6(eax - ebx);
    set(f.repeat_outer_homes[2], temp);
    edi -= ecx;
    set(f.repeat_outer_homes[1], edi);
    set(f.repeat_outer_homes[0], eax);
    byte(f.state_04, 3);
    eax = repeat_native_gui_icon_states_wrapper_storage_00ab56c0(vector,
        f.repeat_outer_homes, f.repeat_tag_home, f.repeat_callee_homes, f.repeat_frame, c.storage);
    eax = homes[3];
    write(pointer(vector), 8, read(pointer(vector), 8) + eax);
    edi = read(pointer(vector), 8); // Source carrier for Native ESI's late reload.
    set(f.fill_homes[2], temp);
    edi -= eax;
    set(f.fill_homes[1], edi);
    set(f.fill_homes[0], ebx);
    set(f.state_04, 0);
    fill_native_gui_icon_state_range_storage_00ab4af0(f.fill_homes,
        f.assignment_preserve_home, c.storage.strings, c.feature_word_0109eea4);
    goto normal_end;

tail_ge_count:
    edi <<= 6u;
    ecx = edi;
    set(f.range_outer_homes[2], eax);
    edi = eax - ecx;
    set(f.range_outer_homes[1], eax);
    homes[3] = ecx;
    set(f.range_outer_homes[0], edi);
    eax = copy_native_gui_icon_state_range_wrapper_storage_00ab58a0(vector,
        f.range_outer_homes, f.range_tag_home, f.range_callee_homes, f.wrapped_range_frame, c.storage);
    write(pointer(vector), 8, eax);
    eax = get(f.scratch_18);
    set(f.backward_homes[2], eax);
    set(f.backward_homes[1], edi);
    set(f.backward_homes[0], ebx);
    (void)assign_native_gui_icon_state_range_backward_storage_00ab5650(f.backward_homes,
        f.assignment_preserve_home, c.storage.strings, c.feature_word_0109eea4);
    edx = homes[3];
    set(f.fill_homes[2], temp);
    edx += ebx;
    set(f.fill_homes[1], edx);
    set(f.fill_homes[0], ebx);
    fill_native_gui_icon_state_range_storage_00ab4af0(f.fill_homes,
        f.assignment_preserve_home, c.storage.strings, c.feature_word_0109eea4);

normal_end:
    set(f.state_04, 0xffffffffu);
    destroy_native_gui_icon_state_00ab3280(&f.temporary_58, c.storage);
}
} // namespace bsp
