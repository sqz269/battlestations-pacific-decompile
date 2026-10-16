#include "bsp/native_scene_hidden_entity_record_copy_construct.hpp"

#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_scene_property_clone.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_duplicate.hpp"

#include <cstdint>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw storage reconstruction requires MSVC Win32.
#endif

#pragma function(memmove)

namespace bsp {
namespace {
using Word = std::uint32_t;

Word word(const void* address) {
    return *static_cast<const volatile Word*>(address);
}

void store(void* address, Word value) {
    *static_cast<volatile Word*>(address) = value;
}

void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}

Word bits(const void* value) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}

void* add(const void* address, Word offset) {
    return pointer(bits(address) + offset);
}

// Source-only SDK boundary, with no Original helper-frame/entry credit.
// Retain overlap and the reached zero-count/null call without a C++ builtin.
__declspec(naked) void* __cdecl copy_current_crt_bytes(void*, const void*, Word) {
    __asm { jmp memmove }
}

class ParentHeaderCleanup {
public:
    ParentHeaderCleanup(const void* actual_saved_this4, NativeStringRawPoolContext& strings)
        : saved_this_(actual_saved_this4), strings_(strings) {}
    ~ParentHeaderCleanup() noexcept(false) {
        if (armed_) unwind_native_scene_hidden_entity_record_copy_00c61a90(saved_this_, strings_);
    }
    void arm() noexcept { armed_ = true; }
    void disarm() noexcept { armed_ = false; }
private:
    const void* const saved_this_;
    NativeStringRawPoolContext& strings_;
    bool armed_ = false;
};
} // namespace

void unwind_native_scene_hidden_entity_record_copy_00c61a90(
    const void* actual_saved_this4, NativeStringRawPoolContext& strings) {
    void* const current_record = pointer(word(actual_saved_this4));
    destroy_native_string_header_0041dd20(add(current_record, 0x54), strings);
}

void* copy_construct_native_scene_hidden_entity_record_00469240(
    void* actual_record, const NativeSceneHiddenRecordCopyConstructFrame& frame,
    NativeStringRawPoolContext& strings, NativeScenePropertyCloneContext& clone_context) {
    void* const record = actual_record;
    const void* const parameters = frame.actual_parameters4;
    void* const saved_this = frame.actual_saved_this4;
    void* const parent_header = add(record, 0x54);
    ParentHeaderCleanup cleanup(saved_this, strings);

    store(saved_this, bits(record)); // native PUSH ECX at 00469255
    store(record, 0x00ce5640);       // borrowed actual image word only
    store(saved_this, bits(record)); // explicit saved-this store at 00469264
    store(parent_header, 0);
    store(add(parent_header, 4), 0);

    const void* const source = pointer(word(parameters)); // 00469275, once
    const void* const source_bag = pointer(word(add(source, 4)));
    cleanup.arm(); // native state0 before the real bag clone
    void* const cloned_bag = clone_native_scene_property_bag_008f41f0(source_bag, clone_context);
    store(add(record, 4), bits(cloned_bag));

    const auto* const name = static_cast<const char*>(pointer(word(add(source, 8))));
    void* const name_copy = duplicate_native_string_00438e40(name);
    store(add(record, 8), bits(name_copy));
    const auto* const class_name = static_cast<const char*>(pointer(word(add(source, 0x0c))));
    void* const class_copy = duplicate_native_string_00438e40(class_name);
    const void* const matrix = add(source, 0x14);
    store(add(record, 0x0c), bits(class_copy)); // BEFORE current source party read
    const Word party = word(add(source, 0x10));
    store(add(record, 0x10), party);
    copy_native_camera_matrix_004134f0(add(record, 0x14), nullptr, matrix);

    const void* const source_parent = add(source, 0x54);
    if (parent_header != source_parent) {
        const Word source_length = word(source_parent);
        resize_native_string_header_0041dd40(parent_header, strings, source_length, true);
        if (word(source_parent) != 0) {
            const Word destination_length = word(parent_header);
            const void* const source_data = pointer(word(add(source_parent, 4)));
            void* const destination_data = pointer(word(add(parent_header, 4)));
            copy_current_crt_bytes(destination_data, source_data, destination_length);
        }
    }
    cleanup.disarm(); // ordinary guard only; no native state-minus-one write
    return record;
}
} // namespace bsp
