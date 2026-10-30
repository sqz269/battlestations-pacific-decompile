#include "bsp/native_lua_script_overrides_raw.hpp"

#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_vfs_application.hpp"
#include "bsp/native_lua_script_overrides.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_vector_raw_storage.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <intrin.h>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw BDEF90 binding requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(Word) == 4 && sizeof(std::size_t) == 4);

Word address(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
void* pointer(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
std::int32_t signed_word(Word bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
Word word(const void* header, Word byte_offset) noexcept {
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer(address(header) + byte_offset), sizeof(value));
    _ReadWriteBarrier();
    return value;
}
void put_word(void* header, Word byte_offset, Word value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(pointer(address(header) + byte_offset), &value, sizeof(value));
    _ReadWriteBarrier();
}
void zero_header(void* header) noexcept {
    put_word(header, 0u, 0u);
    put_word(header, 4u, 0u);
}
void copy_bytes(Word destination, Word source, Word count) {
    // Current raw-pool overlap convention; omit only a zero-byte call.
    if (count != 0u) std::memmove(pointer(destination), pointer(source), count);
}

struct TemporaryScope final {
    NativeStringRawPoolContext& strings;
    int state = -1;
    alignas(4) std::byte stem[8];
    alignas(4) std::byte extension[8];
    // Native reuses this same physical header for substring temp/candidate.
    alignas(4) std::byte candidate[8];
    alignas(4) std::byte underscore[8];
    alignas(4) std::byte prefix[8];
    alignas(4) std::byte with_suffix[8];

    explicit TemporaryScope(NativeStringRawPoolContext& raw) noexcept : strings(raw) {}
    TemporaryScope(const TemporaryScope&) = delete;
    TemporaryScope& operator=(const TemporaryScope&) = delete;

    void return_captured(Word data, Word bytes) {
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8,
            strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, pointer(data), bytes,
            strings.actual_small_returns_disabled_01090aa4);
    }
    void normal_return(void* header, int next_state) {
        const Word data = word(header, 4u); // BEFORE disarm
        state = next_state;
        if (data != 0u) {
            const Word bytes = word(header, 0u) + 1u; // CURRENT AFTER disarm
            return_captured(data, bytes); // never resample header.data
        }
    }
    void* selected_header(int action) noexcept {
        switch (action) {
        case 0: return stem;
        case 1: return extension;
        case 2: return candidate;
        case 3: return underscore;
        case 4: return prefix;
        case 5: return with_suffix;
        default: return candidate; // states6/7/8/9, SAME actual header
        }
    }
    ~TemporaryScope() noexcept {
        // Only true C++ unwind reaches an active state on supported paths.
        // A second cleanup failure terminates; no retry/healthy completion.
        static constexpr int previous[10] = {-1, 0, 1, 1, 3, 4, 5, 4, 3, 1};
        while (state >= 0) {
            const int action = state;
            state = previous[action]; // consume BEFORE real current-header cleanup
            destroy_native_string_header_0041dd20(selected_header(action), strings);
        }
    }
};
} // namespace

NativeLuaScriptOverridesRawContext::NativeLuaScriptOverridesRawContext(
    game::GameNativeVfsApplication& app, const game::GameNativeReadOnlyData& data)
    : strings_(app.raw_strings()), services_(app.borrow_raw_services()), data_(data),
      slash_00ce7898_(static_cast<const char*>(data.data_at(0x00ce7898u, 2u))),
      dot_00ce3a70_(static_cast<const char*>(data.data_at(0x00ce3a70u, 2u))),
      underscore_00ce7890_(static_cast<const char*>(data.data_at(0x00ce7890u, 2u))),
      empty_0109cef0_(services_.actual_empty_name_0109cef0) {
    if (!services_.strings.uses_raw_pool_cells(strings_))
        throw std::invalid_argument("raw BDEF90 requires the application's SAME pool cells");
    if (empty_0109cef0_ == nullptr)
        throw std::invalid_argument("raw BDEF90 requires its actual retained 0109CEF0 role");
    // SAME data-owner identity is a caller precondition; literal equality
    // does not validate an absent application data-identity accessor.
}

void append_native_lua_script_overrides_00bdef90(void* manager, const void* path,
    void* output, const NativeLuaScriptOverridesRawContext& context) {
    if (manager == nullptr || path == nullptr || output == nullptr)
        throw std::invalid_argument("raw BDEF90 requires actual manager/path/output backing");
    if (!context.services_.strings.uses_raw_pool_cells(context.strings_))
        throw std::invalid_argument("raw BDEF90 pool-cell affinity changed");

    const Word slash = reverse_find_native_string_bytes_004bcb80(
        path, context.slash_00ce7898_, 0x7fffffff);
    Word start = slash == 0xffffffffu ? 0u : slash; // slash itself, not +1
    Word dot = 0xffffffffu;
    const Word initial_data = word(path, 4u);
    if (initial_data != 0u) {
        const Word current_length = word(path, 0u);
        if (current_length != 0u) {
            if (signed_word(start) < 0) start = 0u;
            if (start <= current_length) {
                const Word span = static_cast<Word>(std::strcspn(
                    static_cast<const char*>(pointer(initial_data + start)), context.dot_00ce3a70_));
                const Word found = start + span;
                if (found != word(path, 0u)) dot = found; // CURRENT after CRT
            }
        }
    }

    TemporaryScope temporaries(context.strings_);
    zero_header(temporaries.stem);
    temporaries.state = 0;
    zero_header(temporaries.extension);
    temporaries.state = 1;
    if (dot != 0xffffffffu) {
        void* const returned = construct_native_string_substring_00469840(
            path, temporaries.candidate, 0u, dot, context.strings_);
        temporaries.state = 2; // ONLY after the actual callee returned normally
        assign_native_string_header_00425f40(temporaries.stem, returned, context.strings_);
        temporaries.normal_return(temporaries.candidate, 1);

        Word current_data = word(path, 4u); // AFTER temp getter/return
        if (current_data == 0u) current_data = address(context.empty_0109cef0_);
        const Word source = current_data + dot; // captured dot; native wrap/no clamp
        const Word length = source != 0u
            ? static_cast<Word>(std::strlen(static_cast<const char*>(pointer(source)))) : 0u;
        resize_native_string_header_0041dd40(temporaries.extension, context.strings_, length, false);
        const Word destination = word(temporaries.extension, 4u);
        if (destination != 0u) {
            const Word count = word(temporaries.extension, 0u);
            copy_bytes(destination, source, count);
        }
    } else {
        assign_native_string_header_00425f40(temporaries.stem, path, context.strings_);
    }

    Word cursor = word(manager, 0x48u);
    const Word suffix_count = word(manager, 0x4cu);
    const Word end = word(manager, 0x48u) + suffix_count * 8u; // genuine SECOND data read
    while (cursor != end) {
        zero_header(temporaries.underscore);
        resize_native_string_header_0041dd40(temporaries.underscore, context.strings_, 1u, true);
        const Word underscore_data = word(temporaries.underscore, 4u);
        const Word underscore_length = word(temporaries.underscore, 0u);
        if (underscore_data != 0u)
            copy_bytes(underscore_data, address(context.underscore_00ce7890_), underscore_length + 1u);

        temporaries.state = 3;
        void* const returned_prefix = concatenate_native_string_headers_004261a0(
            temporaries.stem, temporaries.prefix, temporaries.underscore, context.strings_);
        temporaries.state = 4;
        void* const returned_with_suffix = concatenate_native_string_headers_004261a0(
            returned_prefix, temporaries.with_suffix, pointer(cursor), context.strings_);
        temporaries.state = 5;
        concatenate_native_string_headers_004261a0(returned_with_suffix,
            temporaries.candidate, temporaries.extension, context.strings_);
        // State6 exists in the FH3 map; Native's normal provider boundaries
        // proceed directly from5 to7 without an invented state6 call/effect.
        temporaries.normal_return(temporaries.with_suffix, 7);
        temporaries.normal_return(temporaries.prefix, 8);
        temporaries.state = 9;
        if (underscore_data != 0u)
            temporaries.return_captured(underscore_data, underscore_length + 1u);

        const Word table = word(manager, 0u);
        const Word entry = word(pointer(table), 8u); // immediately before concrete call
        const auto exists = context.services_.bindings.exists_manager_entry(
            entry, manager, temporaries.candidate);
        if (exists != 0u)
            append_native_string_vector_004cdc20(output, temporaries.candidate, context.strings_);
        // The two original4254B0 diagnostics are verified bare RET and omitted,
        // as qualified by the existing producer; no fake provider is called.
        temporaries.normal_return(temporaries.candidate, 1);
        cursor += 8u;
    }
    temporaries.normal_return(temporaries.extension, 0);
    temporaries.normal_return(temporaries.stem, -1);
}
} // namespace bsp
