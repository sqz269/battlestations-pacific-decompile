#include "bsp/native_input_preset_map_index.hpp"
#include "bsp/native_input_preset_pair_construct.hpp"
#include "bsp/native_input_preset_temp_key_release.hpp"
#include "bsp/native_input_preset_tree_hint_insert.hpp"
#include "bsp/native_input_string_tree_lower_bounds.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_string_header_less_actual.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstddef>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
static_assert(sizeof(std::uintptr_t) == sizeof(Word));

Word word(const void* base, std::size_t offset) {
    return *reinterpret_cast<const volatile Word*>(
        reinterpret_cast<std::uintptr_t>(base) + offset);
}

void* pointer(Word bits) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

void* at(void* base, Word offset) {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}

struct CompletedKeyCleanup {
    void* actual_completed_key;
    NativeStringRawPoolContext& strings;
    bool armed;

    ~CompletedKeyCleanup() noexcept(false) {
        if (armed)
            cleanup_native_input_preset_map_completed_key_00c7f100(
                actual_completed_key, strings);
    }

    void disarm() noexcept { armed = false; }
};
} // namespace

void __fastcall cleanup_native_input_preset_map_completed_key_00c7f100(
    void* actual_completed_key, NativeStringRawPoolContext& strings) {
    release_native_input_preset_temp_key_0069e090(actual_completed_key, 0u, strings);
}

void* __fastcall index_native_input_preset_tree_006a1e70(
    void* actual_tree, std::uint32_t unused_edx, const void* actual_key_header,
    std::uint32_t default_flag_word_preimage,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& actual_callees) {
    static_cast<void>(unused_edx);
    auto* retained_owner = actual_tree;
    const auto* const retained_key = actual_key_header;
    auto* retained_node = lower_bound_native_input_preset_tree_0069cdb0(
        retained_owner, 0u, retained_key, actual_locale_changed_0109de1c,
        actual_callees);
    if (!retained_owner)
        invoke_native_invalid_parameter_00bf6713();

    if (retained_node == pointer(word(retained_owner, 4)) ||
        less_native_string_headers_with_callees_00443d00(
            retained_key, at(retained_node, 0x0c), actual_locale_changed_0109de1c,
            actual_callees)) {
        // Native initializes four DWORDs and only BYTE0 of the fifth. Supply
        // the copied upper24 representation explicitly, with no float value.
        alignas(4) const Word default_mapped[5]{
            0xffffffffu, 0u, 0u, 0u, default_flag_word_preimage & 0xffffff00u};
        alignas(4) Word temporary_pair[7];
        static_assert(sizeof(default_mapped) == 0x14);
        static_assert(sizeof(temporary_pair) == 0x1c);

        auto* const actual_pair_result = construct_native_input_preset_pair_0069e6e0(
            temporary_pair, 0u, retained_key, default_mapped, strings,
            actual_feature_word_0109eea4);
        const NativeInputPresetTreeIterator hint{retained_owner, retained_node};
        NativeInputPresetTreeIterator private_output;
        Word captured_data;
        {
            // Native state0 begins only after constructor success and before
            // whole hinted insertion. The cleanup borrows the physical temp,
            // independently of the constructor's actual returned pointer.
            CompletedKeyCleanup cleanup{temporary_pair, strings, true};
            auto* const actual_returned = insert_hint_native_input_preset_tree_006a15e0(
                retained_owner, 0u, &private_output, hint, actual_pair_result,
                strings, actual_feature_word_0109eea4,
                actual_locale_changed_0109de1c, actual_callees);
            retained_owner = pointer(word(actual_returned, 0));
            retained_node = pointer(word(actual_returned, 4));
            captured_data = word(temporary_pair, 4);
            cleanup.disarm();
        }

        // The completed-key guard/EH scope has ended. A getter failure on
        // this normal path must not retry69E090 or reread captured DATA.
        if (captured_data != 0) {
            const auto returned_size = word(temporary_pair, 0) + 1u;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                strings.actual_published_01090aa8,
                strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, pointer(captured_data), returned_size,
                strings.actual_small_returns_disabled_01090aa4);
        }
    }

    if (!retained_owner)
        invoke_native_invalid_parameter_00bf6713();
    if (retained_node == pointer(word(retained_owner, 4)))
        invoke_native_invalid_parameter_00bf6713();
    return at(retained_node, 0x14);
}
} // namespace bsp
