#pragma once

#include "bsp/camera_frame_state.hpp"
#include "bsp/system_fog_owner.hpp"
#include <cstdint>

namespace bsp {

// Borrow the ACTUAL +10 counted fog slot of the 40h D64518 receiver at
// game+19E8. No receiver allocation, defaults or copied fog state. This is
// distinct from the world object at game+19CC; no RTTI type name was recovered.
struct FogReceiverFields {
    const SystemFogState*& fog_10;
};

// References to actual live consumer-projection pointer slots of the game.
// Do not pass entry-time snapshots. The receiver wrapper references its real
// field, and camera_19fc names the SAME CameraFrameState used by consumers.
struct WorldFogFactoryGameFields {
    FogReceiverFields* const& receiver_19e8;
    CameraFrameState* const& camera_19fc;
    const void* const& scene_record_05fc;
};

// New host ABI for [004DF6A3,004DF7A5), inside004DE610. Native ESI=game,
// EBX=InterlockedDecrement, EDI=-1, plus parent stack/EH storage. Not a native
// entrypoint or the full constructor. Calls the existing real core allocator;
// host bad_alloc/new-handler behavior propagates, without a null-owner fallback.
//
// Allocate/initialize; optional receiver publication; camera publication; drop
// creator reference; then scalar68/scalar78/color and optional directionals.
// Retains the native x87 FLDZ/FSTP and zero/255 alpha operations. The caller's
// actual packed temporary receives00A5A3AB, consumed beyond this range by the
// native camera+190 setter. No owning result or private environment owner.
//
// The camera must be nonnull when loaded after receiver publication. Every
// nonnull existing fog slot must originate from a live SystemFogOwner. Borrowed
// objects/projection references must survive their native accesses. A nonnull
// scene guard requires every subsequent live reload to provide at leastA60h
// readable bytes; recordsA20..A5F are copied forward without float conversion.
// Null at the initial scene guard preserves raw directional allocation bytes.
// No source length/null recheck changes the native directional loop afterward.
void initialize_world_fog_004df6a3(const WorldFogFactoryGameFields& actual_game,
    std::uint32_t& actual_packed_color_temporary_18);

} // namespace bsp
