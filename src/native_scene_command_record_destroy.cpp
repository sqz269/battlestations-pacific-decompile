#include "bsp/native_scene_command_record_destroy.hpp"

#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw storage reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const void* p) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(Word p) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(p));
}
void* add(const void* p, Word offset) noexcept { return pointer(bits(p) + offset); }
Word read(const void* p) noexcept { return *static_cast<const volatile Word*>(p); }
void write(void* p, Word value) noexcept { *static_cast<volatile Word*>(p) = value; }

class FirstHeaderCleanup {
public:
    FirstHeaderCleanup(const void* saved_this, NativeStringRawPoolContext& strings)
        : saved_this_(saved_this), strings_(strings) {}
    ~FirstHeaderCleanup() noexcept(false) {
        if (armed_) unwind_native_scene_command_record_00c61a50(saved_this_, strings_);
    }
    void arm() noexcept { armed_ = true; }
    void disarm() noexcept { armed_ = false; }
private:
    const void* const saved_this_;
    NativeStringRawPoolContext& strings_;
    bool armed_ = false;
};
} // namespace

void unwind_native_scene_command_record_00c61a50(const void* actual_saved_this4,
    NativeStringRawPoolContext& strings) {
    void* const current_record = pointer(read(actual_saved_this4));
    destroy_native_string_header_0041dd20(add(current_record, 4), strings);
}

void destroy_native_scene_command_record_00469010(void* actual_record,
    void* actual_saved_this4, NativeStringRawPoolContext& strings) {
    void* const record = actual_record;
    void* const saved_this = actual_saved_this4;
    FirstHeaderCleanup cleanup(saved_this, strings);
    write(saved_this, bits(record)); // Native PUSH ECX00469025.
    write(saved_this, bits(record)); // Native explicit store00469029.
    const Word target_data = read(add(record, 0x10));
    cleanup.arm(); // Native state0 follows target-data capture, including null.
    if (target_data != 0) {
        const Word bytes = read(add(record, 0x0c)) + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8,
            strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, pointer(target_data), bytes,
            strings.actual_small_returns_disabled_01090aa4);
    }
    const Word command_data = read(add(record, 8));
    cleanup.disarm(); // Native state-1 follows pointer capture, before length.
    if (command_data != 0) {
        const Word bytes = read(add(record, 4)) + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8,
            strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, pointer(command_data), bytes,
            strings.actual_small_returns_disabled_01090aa4);
    }
}

void* scalar_delete_native_scene_command_record_00469b00(void* actual_record,
    const NativeSceneCommandRecordScalarDeleteFrame& frame,
    NativeStringRawPoolContext& strings) {
    void* const record = actual_record;
    const void* const flags = frame.actual_flags4;
    void* const saved_this = frame.actual_destructor_saved_this4;
    destroy_native_scene_command_record_00469010(record, saved_this, strings);
    const auto live_flags = *static_cast<const volatile unsigned char*>(flags);
    if ((live_flags & 1u) != 0) singleton_lifetime_free(record);
    return record;
}

void delete_native_scene_command_record_pointer_0046a090(void* actual_slot4,
    void* actual_destructor_saved_this4, NativeStringRawPoolContext& strings) {
    void* const slot = actual_slot4;
    void* const record = pointer(read(slot));
    if (record != nullptr) {
        destroy_native_scene_command_record_00469010(record,
            actual_destructor_saved_this4, strings);
        singleton_lifetime_free(record);
        write(slot, 0);
    }
}
} // namespace bsp
