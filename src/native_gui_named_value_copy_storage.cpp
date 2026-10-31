#include "bsp/native_gui_named_value_copy_storage.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"

#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);

void* pointer_from_word(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
void* at(const void* base, Word offset) {
    const Word address = static_cast<Word>(reinterpret_cast<std::uintptr_t>(base));
    return pointer_from_word(address + offset);
}
Word read_word(const void* base, Word offset) {
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, at(base, offset), sizeof(value));
    _ReadWriteBarrier();
    return value;
}
void write_word(void* base, Word offset, Word value) {
    _ReadWriteBarrier();
    std::memcpy(at(base, offset), &value, sizeof(value));
    _ReadWriteBarrier();
}
} // namespace

void* copy_construct_native_gui_named_value_storage_00ab4690(
    void* actual_destination12, const void* actual_source12,
    NativeGuiNamedValueCopyStorageFrame& frame, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    void* const destination = actual_destination12;
    const void* const source = actual_source12;
    const bool same = destination == source; // AB4698, before the two stores.
    write_word(destination, 0u, 0u); // AB469A: ctor, not assignment.
    write_word(destination, 4u, 0u); // AB46A0, including self-copy.
    if (!same) {
        const Word request = read_word(source, 0u); // AB46A9.
        write_word(&frame.preserve_home, 0u, 1u); // Actual full PUSH1 HOME.
        resize_native_string_header_0041dd40_exact(destination, strings, request,
            &frame.preserve_home, actual_feature_word_0109eea4);
        if (read_word(source, 0u) != 0u) { // AB46B3, after resize callbacks.
            const Word count = read_word(destination, 0u); // AB46B8.
            const Word source_data = read_word(source, 4u); // AB46BA.
            const Word destination_data = read_word(destination, 4u); // AB46BD.
            // AB46C3 still calls BF when count==0. SAME live feature cell;
            // its genuine provider decides reached forward/overlap/CPU paths.
            (void)copy_native_crt_bytes_00bf7680(pointer_from_word(destination_data),
                pointer_from_word(source_data), count, actual_feature_word_0109eea4);
        }
    }
    const Word value = read_word(source, 8u); // AB46CB, after the real BF call.
    write_word(destination, 8u, value); // AB46CF, even for self-copy.
    return destination; // Captured ESI identity; new C++ ABI rather than RET4.
}
} // namespace bsp
