#include "bsp/native_gui_icon_state_iterator_insert_storage.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw iterator-insertion supplier requires MSVC Win32 DWORD contracts.
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
} // namespace

void* insert_native_gui_icon_state_iterator_storage_00ab6070(void* actual_vector,
    volatile Word (&homes)[4], volatile Word (&growth_homes)[4],
    NativeGuiIconStateGrowthStorageFrame& growth_frame,
    NativeGuiIconStateGrowthStorageContext& context) {
    const Word owner = homes[1];
    const Word vector = bits(actual_vector);
    Word first = read(pointer(vector), 4);
    Word ordinal;
    if (first != 0) {
        const Word last = read(pointer(vector), 8);
        if (sar6(last - first) != 0) {
            if (first > last) invoke_native_invalid_parameter_00bf6713();
            if (owner == 0 || owner != vector) invoke_native_invalid_parameter_00bf6713();
            ordinal = sar6(homes[2] - first);
        } else ordinal = 0;
    } else ordinal = 0;
    const Word prototype = homes[3];
    const Word position = homes[2];
    growth_homes[3] = prototype;
    growth_homes[2] = 1;
    growth_homes[1] = position;
    growth_homes[0] = owner;
    insert_native_gui_icon_states_storage_00ab59e0(pointer(vector),
        growth_homes, growth_frame, context);
    first = read(pointer(vector), 4);
    if (first > read(pointer(vector), 8)) invoke_native_invalid_parameter_00bf6713();
    ordinal = (ordinal << 6u) + first;
    const Word last = read(pointer(vector), 8);
    homes[2] = first;
    if (ordinal > last || ordinal < read(pointer(vector), 4))
        invoke_native_invalid_parameter_00bf6713();
    const Word output = homes[0];
    write(pointer(output), 4, ordinal);
    write(pointer(output), 0, vector);
    return pointer(output);
}
} // namespace bsp
