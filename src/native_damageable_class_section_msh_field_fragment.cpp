#include "bsp/native_damageable_class_section_msh_field_fragment.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>

namespace bsp {
static_assert(sizeof(std::uintptr_t) == 4);

NativeDamageableClassSectionMshFieldFragment::NativeDamageableClassSectionMshFieldFragment(
    NativeDamageableClassSectionMshFieldFragmentScratch& scratch,
    NativeStringRawPoolContext& strings) noexcept : scratch_(scratch), strings_(strings) {}

NativeDamageableClassSectionMshFieldFragment::~NativeDamageableClassSectionMshFieldFragment() noexcept {
    if (state_ == 15) {
        state_ = 13;
        destroy_native_lua_object_00b67700(
            *static_cast<NativeLuaObjectStorage*>(scratch_.fresh_field_at_parent_e0));
    } else if (state_ == 17) {
        state_ = 13;
        destroy_native_string_header_0041dd20(scratch_.fresh_string_at_parent_18, strings_);
    }
}

void NativeDamageableClassSectionMshFieldFragment::open() {
    auto& value = *static_cast<NativeLuaObjectStorage*>(
        scratch_.same_iterator_scratch.fresh_value_at_parent_2c);
    // Borrow actual key storage; retained D0E190 metadata is not a name-byte proof.
    auto* const field = native_lua_get_by_name_protected(
        value, scratch_.fresh_field_at_parent_e0, scratch_.actual_msh_category_key_00d0e190);
    state_ = 15; // CED3, only after successful lookup
    const char* const text = native_lua_string_protected(*field);
    auto* const header = static_cast<std::byte*>(scratch_.fresh_string_at_parent_18);
    const std::uint32_t zero_length = 0;
    void* const null_data = nullptr;
    std::memcpy(header, &zero_length, sizeof zero_length);
    std::memcpy(header + 4, &null_data, sizeof null_data);
    const auto start = reinterpret_cast<std::uintptr_t>(text);
    auto cursor = start;
    unsigned char byte;
    do {
        byte = *reinterpret_cast<const unsigned char*>(cursor);
        ++cursor;
    } while (byte != 0);
    const auto length = static_cast<std::uint32_t>(cursor - start - 1u);
    resize_native_string_header_0041dd40(header, strings_, length, true);
    std::memcpy(&saved_data_, header + 4, sizeof saved_data_);
    const bool has_data = saved_data_ != nullptr;
    std::memcpy(&saved_length_, header, sizeof saved_length_); // even null
    if (has_data) std::memmove(saved_data_, text, saved_length_ + 1u);
    state_ = 17; // CF28, BEFORE field destruction; no state16 activation
    destroy_native_lua_object_00b67700(*field);
}

void NativeDamageableClassSectionMshFieldFragment::close() {
    const bool has_data = saved_data_ != nullptr;
    state_ = 13; // D183, before any potentially failing getter; no retry
    if (has_data) {
        const auto size = saved_length_ + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings_.actual_published_01090aa8, strings_.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(
            pool, saved_data_, size, strings_.actual_small_returns_disabled_01090aa4);
    }
}

bool NativeDamageableClassSectionMshFieldFragment::try_borrow_saved_category_data(
    const NativeDamageableClassSectionMshFieldFragmentScratch& expected,
    const char*& output) const noexcept {
    if (state_ != 17 || &expected != &scratch_) return false;
    output = static_cast<const char*>(saved_data_);
    return true;
}
} // namespace bsp
