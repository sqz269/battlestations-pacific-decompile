#include "bsp/native_scene_hidden_entity_record_destroy.hpp"

#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(Word p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(p + offset);
}
void write(Word p, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(p) = value;
}
using ActualBagDeleteSlot = void (__thiscall*)(void*, Word);

class HeaderCleanup {
public:
    HeaderCleanup(Word actual_saved_this4, NativeStringRawPoolContext& strings)
        noexcept : saved_(actual_saved_this4), strings_(strings) {}
    ~HeaderCleanup() noexcept(false) {
        if (armed_)
            unwind_native_scene_hidden_entity_record_00c61ab0(
                pointer(saved_), strings_);
    }
    void arm() noexcept { armed_ = true; }
    void disarm() noexcept { armed_ = false; }
private:
    Word saved_;
    NativeStringRawPoolContext& strings_;
    bool armed_ = false;
};
}

void unwind_native_scene_hidden_entity_record_00c61ab0(
    const void* actual_saved_this4, NativeStringRawPoolContext& strings) {
    const Word current_record = read(address(actual_saved_this4));
    destroy_native_string_header_0041dd20(pointer(current_record + 0x54u), strings);
}

void destroy_native_scene_hidden_entity_record_00469300(void* actual_record,
    void* actual_saved_this4, NativeStringRawPoolContext& strings) {
    const Word record = address(actual_record);
    const Word saved = address(actual_saved_this4);
    HeaderCleanup cleanup(saved, strings);
    write(saved, record); // Native PUSH ECX at469315.
    write(saved, record); //46931A, before the borrowed raw image word.
    write(record, 0x00ce5640u);
    const Word bag = read(record, 4);
    cleanup.arm(); // State0 follows the bag capture, even when it is null.
    if (bag != 0) {
        const Word actual_vtable = read(bag);
        const Word actual_slot = read(actual_vtable);
        reinterpret_cast<ActualBagDeleteSlot>(actual_slot)(pointer(bag), 1u);
        write(record + 4u, 0u);
    }
    const Word name = read(record, 8);
    if (name != 0) {
        singleton_lifetime_free(pointer(name));
        write(record + 8u, 0u);
    }
    const Word class_name = read(record, 0x0cu);
    if (class_name != 0) {
        singleton_lifetime_free(pointer(class_name));
        write(record + 0x0cu, 0u);
    }
    const Word data = read(record, 0x58u);
    cleanup.disarm(); // Data capture precedes disarm; length follows it.
    if (data != 0) {
        const Word bytes = read(record, 0x54u) + 1u;
        auto* const owner = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8,
            strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(owner, pointer(data), bytes,
            strings.actual_small_returns_disabled_01090aa4);
    }
}

std::uint32_t delete_native_scene_hidden_entity_record_004693a0(
    void* actual_record, const NativeSceneHiddenRecordDeleteFrame& frame,
    NativeStringRawPoolContext& strings) {
    const Word record = address(actual_record);
    const Word flags = address(frame.actual_flags4);
    void* const saved = frame.actual_child_saved_this4;
    destroy_native_scene_hidden_entity_record_00469300(pointer(record), saved,
        strings);
    const auto live_flags = *reinterpret_cast<const volatile unsigned char*>(flags);
    if ((live_flags & 1u) != 0)
        singleton_lifetime_free(pointer(record));
    return record;
}
}
