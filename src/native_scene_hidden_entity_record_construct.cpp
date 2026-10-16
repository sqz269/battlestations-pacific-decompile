#include "bsp/native_scene_hidden_entity_record_construct.hpp"

#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_duplicate.hpp"

#include <cstdint>
#include <cstring>

#pragma function(memmove)

namespace bsp {
namespace {

using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word address(const void* storage) noexcept {
    return reinterpret_cast<Word>(storage);
}

void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}

Word read_word(Word location) noexcept {
    return *reinterpret_cast<const volatile Word*>(location);
}

void write_word(Word location, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(location) = value;
}

// Source-only adapter to the real selected SDK symbol, with no Native credit.
// Keep zero-length/null-address calls raw rather than invoking a C++ builtin
// whose valid-object assumptions would narrow the reached native copy domain.
__declspec(naked) void* __cdecl copy_current_crt_bytes(
    void*, const void*, Word) {
    __asm { jmp memmove }
}

class ParentHeaderCleanup {
public:
    ParentHeaderCleanup(Word actual_saved_this4,
        NativeStringRawPoolContext& strings) noexcept
        : saved_this4_(actual_saved_this4), strings_(strings) {}

    ~ParentHeaderCleanup() noexcept(false) {
        if (armed_)
            unwind_native_scene_hidden_entity_record_00c61ad0(
                pointer(saved_this4_), strings_);
    }

    void arm() noexcept { armed_ = true; }
    void disarm() noexcept { armed_ = false; }

private:
    Word saved_this4_;
    NativeStringRawPoolContext& strings_;
    bool armed_ = false;
};

} // namespace

void unwind_native_scene_hidden_entity_record_00c61ad0(
    const void* actual_saved_this4, NativeStringRawPoolContext& strings) {
    const Word current_record = read_word(address(actual_saved_this4));
    destroy_native_string_header_0041dd20(pointer(current_record + 0x54u), strings);
}

void* construct_native_scene_hidden_entity_record_004693c0(void* actual_record,
    const NativeSceneHiddenRecordConstructFrame& frame,
    NativeStringRawPoolContext& strings) {
    const Word record = address(actual_record);
    const Word parameters = address(frame.actual_parameters24);
    const Word saved_this4 = address(frame.actual_saved_this4);
    const Word parent_header = record + 0x54u;
    ParentHeaderCleanup cleanup(saved_this4, strings);

    write_word(saved_this4, record); // Native PUSH ECX at004693D5.
    write_word(record, 0x00ce5640u); // Borrowed original image word, not Source vtable.
    write_word(saved_this4, record); //004693E4, before parent-header zeros.
    write_word(parent_header, 0u);
    write_word(parent_header + 4u, 0u);

    const Word bag = read_word(parameters);
    const Word name = read_word(parameters + 8u);
    cleanup.arm(); // Native state0 after these captures, before bag publication.
    write_word(record + 4u, bag);
    char* const name_copy = duplicate_native_string_00438e40(
        static_cast<const char*>(pointer(name)));

    const Word class_name = read_word(parameters + 4u);
    write_word(record + 8u, address(name_copy));
    char* const class_copy = duplicate_native_string_00438e40(
        static_cast<const char*>(pointer(class_name)));

    const Word party = read_word(parameters + 0x0cu);
    const Word matrix = read_word(parameters + 0x10u);
    write_word(record + 0x10u, party);
    write_word(record + 0x0cu, address(class_copy));
    (void)copy_native_camera_matrix_004134f0(pointer(record + 0x14u), nullptr,
        pointer(matrix));

    const Word source_header = read_word(parameters + 0x14u);
    if (source_header != parent_header) {
        const Word source_length = read_word(source_header);
        resize_native_string_header_0041dd40(pointer(parent_header), strings,
            source_length, true);
        if (read_word(source_header) != 0u) {
            const Word count = read_word(parent_header);
            const Word source_data = read_word(source_header + 4u);
            const Word destination_data = read_word(parent_header + 4u);
            (void)copy_current_crt_bytes(pointer(destination_data),
                pointer(source_data), count);
        }
    }

    cleanup.disarm();
    return pointer(record);
}

} // namespace bsp
