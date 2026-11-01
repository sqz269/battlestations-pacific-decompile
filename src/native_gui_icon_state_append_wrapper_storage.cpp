#include "bsp/native_gui_icon_state_append_wrapper_storage.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw append wrapper requires MSVC Win32 DWORD storage contracts.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
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
void set(Word& home, Word value) noexcept { static_cast<volatile Word&>(home) = value; }
} // namespace

void append_native_gui_icon_state_storage_00ab63a0(void* actual_vector,
    const volatile Word& prototype_home,
    NativeGuiIconStateAppendWrapperStorageFrame& frame,
    NativeGuiIconStateGrowthStorageContext& context) {
    const Word vector = bits(actual_vector);
    const Word first = read(pointer(vector), 4);
    const Word size = first == 0 ? 0 : sar6(read(pointer(vector), 8) - first);
    if (first != 0 && size < sar6(read(pointer(vector), 0x0c) - first)) {
        const Word first_prototype = prototype_home;
        const Word second_prototype = prototype_home;
        Word last = read(pointer(vector), 8);
        *reinterpret_cast<volatile std::uint8_t*>(&frame.local_08[0]) = 0;
        const Word tag = static_cast<volatile Word&>(frame.local_08[0]);
        set(frame.repeat_homes[3], tag);
        set(frame.repeat_homes[2], first_prototype);
        set(frame.repeat_homes[1], vector);
        set(frame.repeat_homes[0], second_prototype);
        repeat_construct_native_gui_icon_states_storage_00ab5400(pointer(last), 1,
            frame.repeat_homes, frame.repeat_frame, context.storage);
        last += 0x40u;
        write(pointer(vector), 8, last);
        return;
    }
    const Word last = read(pointer(vector), 8);
    if (first > last) invoke_native_invalid_parameter_00bf6713();
    const Word prototype = prototype_home;
    set(frame.iterator_homes[3], prototype);
    set(frame.iterator_homes[2], last);
    set(frame.iterator_homes[1], vector);
    set(frame.iterator_homes[0], bits(frame.local_08));
    (void)insert_native_gui_icon_state_iterator_storage_00ab6070(pointer(vector),
        frame.iterator_homes, frame.growth_homes, frame.growth_frame, context);
}
} // namespace bsp
