#include "bsp/native_string_vector_append.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_string_uninitialized_fill.hpp"
#include "bsp/native_string_vector_insert_one.hpp"

#include <cstdint>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word), "requires the Win32 address model");

Word address(const void* value) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}

void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}

Word word(Word base, Word offset) {
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer(base + offset), sizeof value);
    _ReadWriteBarrier();
    return value;
}

void put(Word base, Word offset, Word value) {
    _ReadWriteBarrier();
    std::memcpy(pointer(base + offset), &value, sizeof value);
    _ReadWriteBarrier();
}

Word sar_three(Word value) {
    return (value >> 3u) | ((0u - (value >> 31u)) << 29u);
}
} // namespace

void __fastcall append_native_string_header_vector_00450540(
    void* header, Word unused_edx, const void* value,
    NativeStringRawPoolContext& strings) {
    (void)unused_edx;
    const Word vector = address(header);
    const Word initial_begin = word(vector, 4u);
    const Word size = initial_begin == 0u
        ? 0u : sar_three(word(vector, 8u) - initial_begin);

    // The capacity calculation uses the original begin. A null begin skips
    // both this capacity read and the initial size's end read.
    if (initial_begin != 0u) {
        const Word capacity = sar_three(word(vector, 12u) - initial_begin);
        if (size < capacity) {
            const Word end = word(vector, 8u);
            // Native supplies four physical stack words to CE20: value,
            // owner, duplicate value, and a tag with only its low byte zeroed.
            // The accepted actual service consumes only value and owner.
            fill_native_string_uninitialized_0044ce20(
                pointer(end), 1u, value, header, strings);
            put(vector, 8u, end + 8u);
            return;
        }
    }

    const Word end = word(vector, 8u);
    if (initial_begin > end) {
        invoke_native_invalid_parameter_00bf6713();
    }
    // A returning handler keeps the captured end. InsertOne performs its
    // own actual validation/reloads and fills this discarded output pair.
    // Its native EDX is unconsumed; Source supplies a defined dead word.
    Word output[2];
    insert_native_string_header_vector_one_00450180(
        header, 0u, output, header, pointer(end), value, strings);
}
} // namespace bsp
