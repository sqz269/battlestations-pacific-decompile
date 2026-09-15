#pragma once

#include "bsp/native_material_effect_owner.hpp"
#include "bsp/native_material_owner.hpp"
#include "bsp/native_post_effect_frame_binding.hpp"

#include <cstdint>

namespace bsp {

// Complete 00B17320 leaf: original ECX actual material, EAX DWORD at +104h,
// plain RET. This intentionally returns the stored bit pattern, including -1.
// It adds no validation, reference operation, or material construction.
std::uint32_t __fastcall get_native_material_word_104_00b17320(
    const NativeMaterialStorage* actual_material) noexcept;

// Complete 00B17D90 selection leaf over the SAME actual effect-owner storage.
// Original ECX owner and stack signed index remain ECX and stack/RET4; the new
// EDX input is a reference to the live InterlockedIncrement IAT cell 00CE221C.
// The reference is passed as that cell's address, never as a target snapshot.
// A nonnull direct slot is borrowed with no retain. Otherwise capture +98h,
// increment captured+4 through the CURRENT IAT target, and reload +98h.
// Negative indices below the signed +38h count make the original raw address
// calculation. Null owner/fallback and invalid slot addresses retain native
// fault behavior; no checks, substitute resource, or callback layer is added.
void* __fastcall select_native_material_effect_texture_00b17d90(
    const NativeMaterialEffectBaseStorage* actual_owner,
    NativePostEffectFrameAtomic const volatile& actual_increment_00ce221c,
    std::int32_t signed_index) noexcept;

// These are new source interfaces. Their additional IAT-cell argument means
// 00B17D90 is not a drop-in binary entry for its original one-argument ABI.

} // namespace bsp
