#pragma once

#include "bsp/game_native_string_process.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native property library readers require MSVC Win32.
#endif

namespace bsp {

// Retain the SAME concrete process supplying raw key allocation/return and
// the distinct E18560 library-empty/E17654 query-empty cells. Starts nothing;
// no arbitrary fallback, model host or callback is accepted here.
class NativePropertyLibraryReaderContext final {
public:
    explicit NativePropertyLibraryReaderContext(
        game::GameNativeStringProcess& actual_strings) noexcept
        : strings_(actual_strings) {}

    game::GameNativeStringProcess& strings() const noexcept { return strings_; }

private:
    game::GameNativeStringProcess& strings_;
};

// Complete ordinary bodies with new C++ interfaces, not Original ECX/stack/
// RET4/FS/EH transport. Borrow an actual coherent10Ch owner, map at owner+4,
// genuine live14h nodes/canonical pooled keys and a nonnull closed ASCII C
// string in the current C locale. The owner, chains, raw-string generation and
// SAME process providers must stay live/stable through lookup AND key release.
// No physical-pool startup is required merely to read an already genuine map.
// Admit successful key construction/cleanup, no overflow/alias/mutation/reentry,
// invalid storage, provider throw/fault or exceptional unwind. There is no
// invented RAII cleanup or Original exception equivalence.

// Whole [008F2DB0,008F2E35),133B/41 instructions. Construct actual aligned8h
// raw local key, find on owner+4, capture node!=null BEFORE key cleanup. Null
// node returns false normally. Never read mapped+8. Original only AL matters.
bool contains_native_property_library_name_008f2db0(const void* actual_owner,
    const char* key, NativePropertyLibraryReaderContext& context);

// Whole [00469B60,00469BE1),129B/38 instructions. This ordinary domain REQUIRES
// the key to be found. Physically read opaque DWORD node+8 BEFORE key cleanup,
// retain it through cleanup, then return full word (which may itself be zero).
// No null-node guard/default is supplied; absence is outside this API's domain.
std::uint32_t read_native_property_library_word_00469b60(
    const void* actual_owner, const char* key,
    NativePropertyLibraryReaderContext& context);

} // namespace bsp
