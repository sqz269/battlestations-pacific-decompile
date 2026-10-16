#include "bsp/native_scene_command_record_construct.hpp"

#include <cstdint>
#include <cstring>

#pragma function(memmove)

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read(Word base, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(base + offset);
}
void write(Word base, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(base + offset) = value;
}

Word measured_length(Word data) noexcept {
    if (data == 0) return 0;
    Word cursor = data;
    unsigned char byte;
    do {
        byte = *reinterpret_cast<const volatile unsigned char*>(cursor);
        cursor += 1u;
    } while (byte != 0);
    return cursor - (data + 1u);
}

// Actual current SDK boundary. Avoid a C++ builtin so reached zero-count/null
// calls retain the real service's behavior. No native BF7680/private ABI credit.
__declspec(naked) void* __cdecl copy_current_crt_bytes(void*, const void*, Word) {
    __asm { jmp memmove }
}

class RecordCleanup {
public:
    RecordCleanup(const void* saved_record, NativeStringRawPoolContext& strings)
        : saved_record_(saved_record), strings_(strings) {}
    ~RecordCleanup() noexcept(false) {
        if (state_ == 1) {
            state_ = 0;
            unwind_native_scene_command_record_target_00c61a7b(saved_record_, strings_);
        }
        if (state_ == 0) {
            state_ = -1;
            unwind_native_scene_command_record_command_00c61a70(saved_record_, strings_);
        }
    }
    void arm_command() noexcept { state_ = 0; }
    void arm_target() noexcept { state_ = 1; }
    void complete() noexcept { state_ = -1; }
private:
    const void* const saved_record_;
    NativeStringRawPoolContext& strings_;
    int state_ = -1;
};
} // namespace

void unwind_native_scene_command_record_command_00c61a70(
    const void* actual_saved_record4, NativeStringRawPoolContext& strings) {
    const Word current_record = read(address(actual_saved_record4));
    destroy_native_string_header_0041dd20(pointer(current_record + 4u), strings);
}

void unwind_native_scene_command_record_target_00c61a7b(
    const void* actual_saved_record4, NativeStringRawPoolContext& strings) {
    const Word current_record = read(address(actual_saved_record4));
    destroy_native_string_header_0041dd20(pointer(current_record + 0x0cu), strings);
}

void* construct_native_scene_command_record_004690d0(void* actual_record20,
    const NativeSceneCommandRecordConstructFrame& frame,
    NativeStringRawPoolContext& strings) {
    const Word record = address(actual_record20);
    const Word parameters = address(frame.actual_parameters12);
    const Word saved_record = address(frame.actual_saved_record4);
    const Word command_header = record + 4u;
    const Word target_header = record + 0x0cu;
    RecordCleanup cleanup(pointer(saved_record), strings);

    write(saved_record, 0, record); // Native PUSH ECX at 004690E5.
    write(saved_record, 0, record); // Explicit saved-record MOV at 004690F1.
    write(command_header, 0, 0);
    write(command_header, 4, 0);
    cleanup.arm_command(); // 004690FD state0, before target-header stores.
    write(target_header, 0, 0);
    write(target_header, 4, 0);

    const Word command = read(parameters, 4); // 00469106, BEFORE owner.
    const Word owner = read(parameters);
    cleanup.arm_target(); // 00469110 state1, before owner publication.
    write(record, 0, owner);
    const Word command_length = measured_length(command);
    resize_native_string_header_0041dd40(pointer(command_header), strings,
        command_length, false);
    const Word command_data = read(command_header, 4);
    if (command_data != 0) {
        const Word count = read(command_header);
        (void)copy_current_crt_bytes(pointer(command_data), pointer(command), count);
    }

    const Word target = read(parameters, 8); // 0046914E, after resize/copy.
    const Word target_length = measured_length(target);
    resize_native_string_header_0041dd40(pointer(target_header), strings,
        target_length, false);
    const Word target_data = read(target_header, 4);
    if (target_data != 0) {
        const Word count = read(target_header);
        (void)copy_current_crt_bytes(pointer(target_data), pointer(target), count);
    }
    cleanup.complete(); // Native returns with no member destruction.
    return pointer(record);
}

} // namespace bsp
