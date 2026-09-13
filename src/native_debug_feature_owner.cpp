#include "bsp/native_debug_feature_owner.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_vector.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstring>

namespace bsp {
namespace {
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
const void* at(const void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
std::uint32_t word(const void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(p, offset));
}
std::int32_t signed_word(const void* p, std::uint32_t offset) noexcept {
    return static_cast<std::int32_t>(word(p, offset));
}
void put(void* p, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(p, offset)) = value;
}
void* pointer(const void* p, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
void clear_byte(void* p, std::uint32_t offset) noexcept {
    *static_cast<volatile unsigned char*>(at(p, offset)) = 0;
}

struct NativeGuard {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, section_04) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(NativeStringVectorStorage) == 0x0c);

volatile std::uint32_t& tracked_counter(CRITICAL_SECTION* section) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(section, 0x18));
}

void destroy_names(void* actual_vector, NativeStringStorage& strings) {
    // Actual427880 cleanup schedule. Reuse full427110, rather than the old
    // mesh-only no-grow fragment, so a signed negative capacity still grows.
    resize_native_string_vector_00427110(
        *static_cast<NativeStringVectorStorage*>(actual_vector), 0, strings);
    singleton_lifetime_free(pointer(actual_vector));
}
} // namespace

void destroy_native_debug_feature_owner_base_00be8210(
    void* owner, NativeDebugFeatureOwnerContext& context) noexcept {
    context.actual_owner_publication_0109db70 = nullptr;
    put(owner, 0, 0x00ce3818u);
}

void* construct_native_debug_feature_owner_00be94f0(
    void* owner, NativeDebugFeatureOwnerContext& context) {
    put(owner, 0, 0x00d68b94u);
    put(owner, 4, 0);
    put(owner, 8, 0);
    put(owner, 0x0c, 0);
    try {
        // CE3A0C is a nonnull empty literal, not a null cstring contract.
        construct_native_string_cstring_0041e870(at(owner, 0x10), "", context.strings);
    } catch (...) {
        // E019F4 states1->0: CC7038(+04 array), CC7030(base). The string
        // constructor itself owns any cleanup; no +10 cleanup is armed here.
        destroy_names(at(owner, 4), context.strings);
        destroy_native_debug_feature_owner_base_00be8210(owner, context);
        throw;
    }
    put(owner, 0x18, 0);
    put(owner, 0x1c, 0);
    put(owner, 0x20, 0);
    clear_byte(owner, 0x24);
    put(owner, 0x28, 0);
    clear_byte(owner, 0x2c);
    put(owner, 0x30, 0);
    return owner;
}

void* get_native_debug_feature_owner_0051f460(NativeDebugFeatureOwnerContext& context) {
    void* const initial = context.actual_owner_publication_0109db70;
    if (initial) return initial;

    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_publication_01090aa0);
    auto* const captured_section = static_cast<CRITICAL_SECTION*>(pointer(first_manager, 0x10));
    NativeGuard guard{0x00ce37fcu, captured_section};
    if (captured_section) {
        EnterCriticalSection(captured_section);
        auto& depth = tracked_counter(captured_section);
        depth = depth + 1u;
    }
    // State0 starts only after successful Enter and the physical+18 increment.
    try {
        if (!context.actual_owner_publication_0109db70) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x34, 0x34});
            void* result;
            try {
                result = allocation
                    ? construct_native_debug_feature_owner_00be94f0(allocation, context)
                    : nullptr;
            } catch (...) {
                singleton_lifetime_free(allocation);
                throw;
            }
            // State1 ends before publication; registration failure keeps it.
            context.actual_owner_publication_0109db70 = result;
            void* const current_manager = get_native_singleton_manager_00415350(
                context.actual_manager_publication_01090aa0);
            void* const current_owner = context.actual_owner_publication_0109db70;
            register_native_singleton_object_00bd0c30(current_manager, nullptr, current_owner);
        }
        if (captured_section) {
            auto& depth = tracked_counter(captured_section);
            depth = depth - 1u;
            LeaveCriticalSection(captured_section);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(&guard);
        throw;
    }
    return context.actual_owner_publication_0109db70;
}

void native_debug_feature_shutdown_hook_00be8350(void*) noexcept {}

void reserve_native_debug_feature_records_00be8df0(
    void* vector, std::int32_t requested, NativeStringStorage& strings) {
    if (requested < 1) requested = 1;
    if (signed_word(vector, 8) >= requested) return;
    const auto bytes = static_cast<std::uint32_t>(requested) * 0x88u;
    void* const fresh = singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes});

    // CC6F70 calls bare RET401130. Deliberately no rollback of fresh storage
    // or already copied strings if a later copy's source allocator throws.
    for (std::uint32_t index = 0;
         static_cast<std::int32_t>(index) < signed_word(vector, 4); ++index) {
        void* const destination = at(fresh, index * 0x88u);
        if (destination) {
            void* const source = at(pointer(vector), index * 0x88u);
            put(destination, 0, 0);
            put(destination, 4, 0);
            if (destination != source) {
                resize_native_string_header_0041dd40(destination, strings, word(source), true);
                if (word(source) != 0) {
                    const auto length = word(destination);
                    void* const input = pointer(source, 4);
                    void* const output = pointer(destination, 4);
                    // BF7680 also implements backward overlap. Match the
                    // existing raw string layer's zero-byte-copy boundary.
                    if (length != 0) std::memmove(output, input, length);
                }
            }
            // BE8EA9 is REP MOVSD, 32 ascending DWORD transfers. Preserve
            // propagation for overlapping record payloads; not memmove.
            for (std::uint32_t offset = 8; offset != 0x88; offset += 4)
                put(destination, offset, word(source, offset));
        }
    }
    for (std::uint32_t index = 0;
         static_cast<std::int32_t>(index) < signed_word(vector, 4); ++index) {
        destroy_native_string_header_0041dd20(at(pointer(vector), index * 0x88u), strings);
    }
    singleton_lifetime_free(pointer(vector));
    put(vector, 0, reinterpret_cast<std::uint32_t>(fresh));
    put(vector, 8, static_cast<std::uint32_t>(requested));
}

void resize_native_debug_feature_records_00be8f30(
    void* vector, std::int32_t requested, NativeStringStorage& strings) {
    if (requested > signed_word(vector, 8))
        reserve_native_debug_feature_records_00be8df0(vector, requested, strings);
    const auto initial = signed_word(vector, 4);
    if (initial < requested) {
        auto offset = static_cast<std::uint32_t>(initial) * 0x88u;
        auto remaining = static_cast<std::uint32_t>(requested) - static_cast<std::uint32_t>(initial);
        do {
            void* const entry = at(pointer(vector), offset);
            if (entry) {
                put(entry, 0, 0);
                put(entry, 4, 0);
                std::memset(at(entry, 8), 0, 0x80);
            }
            offset += 0x88u;
        } while (--remaining != 0);
    }
    while (requested < signed_word(vector, 4)) {
        put(vector, 4, word(vector, 4) - 1u);
        void* const entry = at(pointer(vector), word(vector, 4) * 0x88u);
        destroy_native_string_header_0041dd20(entry, strings);
    }
    put(vector, 4, static_cast<std::uint32_t>(requested));
}

void destroy_native_debug_feature_owner_00be9560(
    void* owner, NativeDebugFeatureOwnerContext& context) {
    int state = 2;
    try {
        void* const features = at(owner, 0x18);
        resize_native_debug_feature_records_00be8f30(features, 0, context.strings);
        singleton_lifetime_free(pointer(features));
        state = 1;
        destroy_native_string_header_0041dd20(at(owner, 0x10), context.strings);
        state = 0;
        destroy_names(at(owner, 4), context.strings);
    } catch (...) {
        // E01A30: state2 string, state1 names, state0 base. Neither record
        // destruction nor a backing free is retried by these unwind actions.
        if (state >= 2)
            destroy_native_string_header_0041dd20(at(owner, 0x10), context.strings);
        if (state >= 1) destroy_names(at(owner, 4), context.strings);
        destroy_native_debug_feature_owner_base_00be8210(owner, context);
        throw;
    }
    destroy_native_debug_feature_owner_base_00be8210(owner, context);
}

void* delete_native_debug_feature_owner_00be9600(
    void* owner, std::uint32_t flags, NativeDebugFeatureOwnerContext& context) {
    destroy_native_debug_feature_owner_00be9560(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
