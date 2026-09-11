#pragma once

#include "bsp/sound_sample_cache.hpp"

namespace bsp {

// Complete game-specific normal/EH ownership projection of 00A886C0.
// Native ECX=this, RET; installs D5B460, clears weak keys and F8BBE4,
// destroys remaining pooled keys/tree storage, then unregisters A88180.
// If clear throws, tree/base cleanup still runs, without resetting F8BBE4.
// Pass the canonical lifetime bindings and SoundSampleRuntime's cache context.
// Requires a live constructed owner/tree. Does not release weak sample payloads.
// C++ map/RAII replace native STL/SEH; no original layout or ABI compatibility.
// Evidence and exceptional-path limits: docs/SOUND_SAMPLE_CACHE_SHUTDOWN.md.
void destroy_sound_sample_cache_00a886c0(SoundAuxiliaryTreeOwner&,
    SoundSampleCacheContext&, SoundOwnerLifetimeBindings&);

// Native ECX=this, stack flags DWORD (only low bit read), EAX=original this,
// RET4. Bit0 deletes the projection allocated by create_*_00a88650; its caller
// must release any unique_ptr ownership first. The returned pointer then dangles.
// A throwing destructor skips deletion, matching native control flow.
SoundAuxiliaryTreeOwner* scalar_delete_sound_sample_cache_00a88750(
    SoundAuxiliaryTreeOwner*, std::uint32_t flags,
    SoundSampleCacheContext&, SoundOwnerLifetimeBindings&);

} // namespace bsp
