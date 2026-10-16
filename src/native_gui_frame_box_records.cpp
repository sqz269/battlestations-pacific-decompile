#include "bsp/native_gui_frame_box_records.hpp"
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI FrameBox record copy requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4);

namespace bsp {
namespace {
using Word = std::uint32_t;
volatile Word& word(void* raw, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(raw) + offset);
}
Word word(const void* raw, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::byte*>(raw) + offset);
}
} // namespace

void* copy_construct_native_gui_frame_box_record_00ad0c60(
    void* destination, const void* source, NativeStringRawPoolContext& strings) {
    const Word resource = word(source, 0);
    auto* const destination_name = static_cast<std::byte*>(destination) + 4;
    const auto* const source_name = static_cast<const std::byte*>(source) + 4;
    const bool same_header = destination_name == source_name;
    word(destination, 0) = resource;
    word(destination, 4) = 0;
    word(destination, 8) = 0;
    if (!same_header) {
        const Word requested = word(source, 4);
        resize_native_string_header_0041dd40(destination_name, strings, requested, true);
        if (word(source, 4) != 0) {
            const Word count = word(destination, 4);
            const auto* const source_data = reinterpret_cast<const void*>(
                static_cast<std::uintptr_t>(word(source, 8)));
            auto* const destination_data = reinterpret_cast<void*>(
                static_cast<std::uintptr_t>(word(destination, 8)));
            // Existing Source byte-copy boundary: overlap is admitted and a
            // zero count does not invoke a standard copy with null pointers.
            if (count != 0) std::memmove(destination_data, source_data, count);
        }
    }
    __asm {
        mov edi, source
        mov esi, destination
        fld dword ptr [edi+0ch]
        fstp dword ptr [esi+0ch]
        fld dword ptr [edi+10h]
        fstp dword ptr [esi+10h]
        fld dword ptr [edi+14h]
        fstp dword ptr [esi+14h]
        fld dword ptr [edi+18h]
        fstp dword ptr [esi+18h]
    }
    word(destination, 0x1c) = word(source, 0x1c);
    word(destination, 0x20) = word(source, 0x20);
    word(destination, 0x24) = word(source, 0x24);
    word(destination, 0x28) = word(source, 0x28);
    auto* const current_resource = reinterpret_cast<std::byte*>(
        static_cast<std::uintptr_t>(word(destination, 0)));
    if (current_resource != nullptr)
        ::InterlockedIncrement(reinterpret_cast<volatile LONG*>(current_resource + 4));
    return destination;
}
} // namespace bsp


#include "bsp/native_render_context.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
NativeGuiFrameBoxRecordReleaseOperation::~NativeGuiFrameBoxRecordReleaseOperation() noexcept {
    if (phase_ == NativeGuiFrameBoxRecordReleasePhase::entered ||
        phase_ == NativeGuiFrameBoxRecordReleasePhase::failed)
        std::terminate();
}

void release_native_gui_frame_box_record_00ad0860(void* record,
    NativeRenderActualOwners& owners, NativeStringRawPoolContext& strings,
    NativeGuiFrameBoxRecordReleaseOperation& operation) {
    if (operation.phase_ != NativeGuiFrameBoxRecordReleasePhase::fresh)
        throw std::logic_error("raw FrameBox record release is one-shot");
    operation.record_ = record;
    operation.phase_ = NativeGuiFrameBoxRecordReleasePhase::entered;
    auto* const resource = reinterpret_cast<std::byte*>(
        static_cast<std::uintptr_t>(word(record, 0)));
    operation.resource_ = resource;
    operation.name_action_armed_ = true;
    try {
        if (resource != nullptr) {
            operation.count_after_decrement_ = ::InterlockedDecrement(
                reinterpret_cast<volatile LONG*>(resource + 4));
            operation.resource_decremented_ = true;
            if (operation.count_after_decrement_ == 0) {
                operation.zero_dispatch_entered_ = true;
                dispatch_native_render_actual_owner_zero(owners, resource);
                operation.zero_dispatch_returned_ = true;
            }
            word(record, 0) = 0;
            operation.resource_cleared_ = true;
        }
        auto* const data = reinterpret_cast<void*>(
            static_cast<std::uintptr_t>(word(record, 8)));
        operation.name_data_ = data;
        operation.name_action_armed_ = false;
        operation.name_action_consumed_ = true;
        if (data != nullptr) {
            const Word size = word(record, 4) + 1u;
            operation.name_size_ = size;
            operation.normal_name_return_entered_ = true;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, data, size,
                strings.actual_small_returns_disabled_01090aa4);
            operation.normal_name_returned_ = true;
        }
        operation.phase_ = NativeGuiFrameBoxRecordReleasePhase::returned;
    } catch (...) {
        operation.phase_ = NativeGuiFrameBoxRecordReleasePhase::failed;
        if (operation.name_action_armed_) {
            operation.name_action_armed_ = false;
            operation.name_action_consumed_ = true;
            operation.failure_name_cleanup_entered_ = true;
            try {
                destroy_native_string_header_0041dd20(static_cast<std::byte*>(record) + 4, strings);
                operation.failure_name_cleanup_returned_ = true;
            } catch (...) {
                std::terminate();
            }
        }
        throw;
    }
}
} // namespace bsp
