#pragma once

#include <cstdint>

namespace bsp {

// Borrow the application's immutable original profile words through each
// reached slot. These are native address tokens, never host function pointers.
struct NativeRendererResourceRestoreProfiles {
    const volatile std::uint32_t* texture_2d_00d61948; // through +27
    const volatile std::uint32_t* cube_00d61870;       // through +27
    const volatile std::uint32_t* volume_00d618b0;     // through +27
    const volatile std::uint32_t* surface_00d619a0;    // through +43
    const volatile std::uint32_t* query_00d62ad0;      // through +1F
};
struct NativeRendererResourceRestoreContext {
    const NativeRendererResourceRestoreProfiles& actual_profiles;
    // Address of the actual publication cell, read by the full query provider.
    const void* actual_renderer_publication_00f8d394;
};

// Complete B23B10..B23C4F: native ECX actual renderer, plain RET, no own EH
// or stable semantic EAX result. Reached owners use the profiles above and
// remain valid at the original accesses. No owner/registry/device is copied.
//
// Gate on current +1D8B==0 and +1D8A==0, capture first device, publish ready,
// reacquire/bind color zero then depth, and visit texture/surface/query lists.
// Texture and surface lists retain their old raw cursor while rereading count
// then base after callbacks. Query traversal reloads its table and signed count.
// HRESULTs are ignored; callback exceptions propagate with all earlier writes.
//
// The new C++ interface uses real local COM output cells. Default acquisitions
// reuse one cell, zeroed before each call. Texture restoration reuses its own
// three-DWORD scratch area, keeping surface/output and device/argument cells
// eight bytes apart as required by the native texture callback. It does not
// reproduce the original caller's return-address or unrelated stack contents.
// This is not a drop-in binary ABI replacement or a game-validation claim.
void restore_native_renderer_resources_00b23b10(
    void* actual_renderer, NativeRendererResourceRestoreContext&);

} // namespace bsp
