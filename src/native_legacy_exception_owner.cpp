#include "bsp/native_legacy_exception_owner.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native legacy exception owners require MSVC Win32 pointer widths.
#endif

namespace bsp {
namespace {
// Verified native table address words; never dispatched as host C++ vtables.
constexpr std::uint32_t exception_vtable = 0x00d69370;
constexpr std::uint32_t logic_error_vtable = 0x00d69248;
constexpr std::uint32_t length_error_vtable = 0x00d69260;

void construct_base(NativeLegacyExceptionStorage& owner) noexcept {
    // Complete library constructor 00BF632F..00BF6340.
    volatile auto& actual = owner;
    actual.base_message_04 = nullptr;
    actual.owns_base_message_08 = 0;
    actual.native_vtable_00 = exception_vtable;
}

void destroy_base(NativeLegacyExceptionStorage& owner) noexcept {
    // 00BF6454..00BF646A tests ownership BEFORE publishing the base table.
    // Message/ownership fields remain unchanged, including after a real free.
    volatile auto& actual = owner;
    const auto owned = actual.owns_base_message_08;
    actual.native_vtable_00 = exception_vtable;
    if (owned != 0) {
        std::free(actual.base_message_04);
    }
}

void copy_base(NativeLegacyExceptionStorage& owner,
    const NativeLegacyExceptionStorage& source) {
    // Complete library copy body 00BF63A6..00BF63FE. Preserve its nullable
    // _malloc contract rather than using the throwing singleton allocator.
    volatile auto& actual = owner;
    const volatile auto& actual_source = source;
    actual.native_vtable_00 = exception_vtable;
    const auto owned = actual_source.owns_base_message_08;
    actual.owns_base_message_08 = owned;
    auto* const captured_message = actual_source.base_message_04;
    if (owned == 0) {
        actual.base_message_04 = captured_message;
    } else if (!captured_message) {
        actual.base_message_04 = nullptr;
    } else {
        const auto bytes = static_cast<std::uint32_t>(std::strlen(captured_message)) + 1u;
        auto* const replacement = static_cast<char*>(std::malloc(bytes));
        actual.base_message_04 = replacement;
        if (replacement) {
            // 00BF63DE reloads source+4 after malloc and destination publication.
            auto* const current_source_message = actual_source.base_message_04;
            (void)strcpy_s(replacement, bytes, current_source_message);
        }
    }
}
}

NativeLegacyExceptionStorage& construct_native_legacy_logic_error_00411700(
    NativeLegacyExceptionStorage& owner, const NativeLegacySboStringStorage& source) {
    construct_base(owner);
    owner.native_vtable_00 = logic_error_vtable;
    owner.message_0c.length_14 = 0;
    owner.message_0c.capacity_18 = 15;
    try {
        // Native state 0 is armed before the first-byte store at 0041173E.
        owner.message_0c.buffer_04.inline_bytes[0] = '\0';
        native_legacy_sbo_string_assign_substring_00408120(
            owner.message_0c, source, 0, 0xffffffffu);
    } catch (...) {
        // State 0, FuncInfo 00D83F74 -> 00C5E010 -> base destructor only.
        destroy_base(owner);
        throw;
    }
    return owner;
}

void destroy_native_legacy_logic_error_00411780(NativeLegacyExceptionStorage& owner) noexcept {
    owner.native_vtable_00 = logic_error_vtable;
    // Includes 00411798 post-free continuation and member reset, then base tail.
    native_legacy_sbo_string_destroy_004072d0(owner.message_0c);
    destroy_base(owner);
}

NativeLegacyExceptionStorage* scalar_delete_native_legacy_logic_error_004117c0(
    NativeLegacyExceptionStorage& owner, std::uint32_t flags) noexcept {
    destroy_native_legacy_logic_error_00411780(owner);
    if ((flags & 1u) != 0) {
        singleton_lifetime_free(&owner);
    }
    // 004117FE..00411807 still returns this after the optional owner free.
    return &owner;
}

NativeLegacyExceptionStorage& copy_native_legacy_logic_error_004118d0(
    NativeLegacyExceptionStorage& owner, const NativeLegacyExceptionStorage& source) {
    copy_base(owner, source); // Native EH state is still -1 during base copying.
    owner.native_vtable_00 = logic_error_vtable;
    owner.message_0c.capacity_18 = 15;
    owner.message_0c.length_14 = 0;
    try {
        // Native state 0 is armed before the first-byte store at 00411918.
        owner.message_0c.buffer_04.inline_bytes[0] = '\0';
        native_legacy_sbo_string_assign_substring_00408120(
            owner.message_0c, source.message_0c, 0, 0xffffffffu);
    } catch (...) {
        // State 0, FuncInfo 00D84024 -> 00C5E050 -> base destructor only.
        destroy_base(owner);
        throw;
    }
    return owner;
}

NativeLegacyExceptionStorage& copy_native_legacy_length_error_00411940(
    NativeLegacyExceptionStorage& owner, const NativeLegacyExceptionStorage& source) {
    copy_native_legacy_logic_error_004118d0(owner, source);
    owner.native_vtable_00 = length_error_vtable;
    return owner;
}

}
