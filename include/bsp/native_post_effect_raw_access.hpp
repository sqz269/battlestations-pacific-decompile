#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual post-effect raw getters require MSVC Win32.
#endif

namespace bsp {

static_assert(sizeof(void*) == 4, "Native raw getters require 32-bit pointers.");

// Complete original ECX-input/EAX-result/plain-RET leaves. Each returns the
// unchanged DWORD at the named offset. "Pass" and "owner" are descriptive
// storage-role hypotheses; no original class layout or pointer type is imposed.
// The receiver must already designate readable native storage. There is no
// null check, retain/release, validation, conversion, or stacked argument.
std::uint32_t __fastcall native_post_effect_pass_raw_08_00b4cb90(
    const void* actual_pass) noexcept;
std::uint32_t __fastcall native_post_effect_pass_raw_18_00b4ec50(
    const void* actual_pass) noexcept;
std::uint32_t __fastcall native_post_effect_owner_raw_23c_00b4ffe0(
    const void* actual_owner) noexcept;

} // namespace bsp
