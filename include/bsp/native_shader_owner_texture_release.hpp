#pragma once

#include "bsp/native_renderer_texture_binding.hpp"

namespace bsp {

// Full B188A0: native ECX actual shader owner, no stack arguments, plain RET;
// no semantic EAX result. Release each current texture slot and zero it only
// after returned destruction. Preserve texture high-water short +38h, all
// registered names, fallback texture and secondary objects. Current short
// bounds are sign-extended then compared unsigned, including the growth store.
void unload_native_shader_owner_textures_00b188a0(
    void* actual_owner, NativeRendererTextureBindingContext&);

// Full B24E20: native ECX actual renderer, no stack arguments, plain RET.
// Walk actual +1A9Ch records with +1AA0h count and 2Ch stride; call full B188A0
// on current record+28h. After the call reread count then base, advance the OLD
// cursor and compare equality with the current end using DWORD arithmetic.
void unload_native_renderer_shader_owner_textures_00b24e20(
    void* actual_renderer, NativeRendererTextureBindingContext&);

// Borrow the existing context's actual profile words, canonical owner pools,
// string allocation and publication/lifetime domains. Admitted current texture
// profiles D61948/D61870/D618B0 select full BD30E0 semantics and their actual
// deleting providers B3F590/B3F410/B3F430, with a fresh table read for slot+04.
// No guard, renderer/owner construction, null repair, count clear, callback
// substitution, rollback or cleanup is added. Reached raw storage must remain
// valid at native reads/writes; these are not mutation-safe container walks.
// Terminal exceptions propagate before slot zero. New MSVC Win32 interfaces,
// not drop-in native ABI entries or proof of game/device-reset validation.

} // namespace bsp
