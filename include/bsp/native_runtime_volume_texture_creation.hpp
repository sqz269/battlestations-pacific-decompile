#pragma once

#include "bsp/native_volume_texture_owner.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
struct NativeRendererDeviceRecreationContext;

// Original six stack DWORDs; this source block is mutable and caller-live.
// B2A710 overwrites height_04 with raw-slot bits before arming state1. Preserve
// current width/height reads on both COM calls and current flags before ctor.
// Format and pool preimage pass all32 bits through a recovered MSVC x86 raw
// DWORD call boundary; no portable SDK function-type compatibility is claimed.
struct NativeRuntimeVolumeTextureCreationArguments {
    std::uint32_t width_00;
    std::uint32_t height_04;
    std::uint32_t depth_08;
    std::uint32_t levels_0c;
    std::uint32_t format_10;
    std::uint32_t flags_14;
};
static_assert(sizeof(NativeRuntimeVolumeTextureCreationArguments) == 24);

// Actual 20-byte local schedule: pool/COM/saved renderer/8-byte guard. These
// are already-live C++ scalar/pointer objects, not an overlay on arbitrary raw
// bytes. Default initialization must preserve preimages. Pool must be defined
// if the low flag nibble exceeds3; a skipped guard is not later initialized.
struct NativeRuntimeVolumeTextureCreationLocals {
    std::uint32_t pool_00;
    IDirect3DVolumeTexture9* com_output_04;
    void* renderer_08;
    NativeRendererOptionalGuardStorage guard_0c;
};
static_assert(sizeof(NativeRuntimeVolumeTextureCreationLocals) == 20);
static_assert(offsetof(NativeRuntimeVolumeTextureCreationLocals, com_output_04) == 4);
static_assert(offsetof(NativeRuntimeVolumeTextureCreationLocals, renderer_08) == 8);
static_assert(offsetof(NativeRuntimeVolumeTextureCreationLocals, guard_0c) == 12);

struct NativeRuntimeVolumeTextureCreationContext {
    NativeVolumeTextureOwnerContext& owner;
    std::uint32_t& actual_shared_serial_0108d6e8;
    NativeRendererSynchronizationGlobals& synchronization;
    NativeRendererDeviceRecreationContext* recreation; // Required only on real retry.
};

// Persistent source diagnostics; no native owner/reference is released by
// destruction of this record. Keep it, the argument block and all domains
// alive after failure. Native locals above receive only the native writes.
// Phase, unwind state and diagnostics are implementation-owned through all
// callbacks: do not mutate them, move/destroy or reenter this acquired record.
// Mutable argument words and native local cells retain their separate contract.
struct NativeRuntimeVolumeTextureCreationAcquired {
    enum class Phase { fresh, running, complete, failed };
    Phase phase{Phase::fresh};
    NativeRuntimeVolumeTextureCreationLocals locals;
    std::uint32_t unwind_state;
    HRESULT last_create_result;
    void* owner{};
    void* returned_slot{};
    bool raw_slot_returned{};
};

// Complete B3DCF0..B3DCFB: ECX slot, RET. Select SAME actual0108DBA8 pool
// and call B3D860. Explicit source companion supplies the actual native pool;
// no pool initialization, slot destructor, count action or retirement proof.
void return_native_volume_texture_slot_00b3dcf0(
    void* current_slot, D3D9SurfacePool& actual_volume_pool_0108dba8);

// Complete [B2A5A0,B2A7AA), 522B/164 instructions, plus pool table B2A7AC.
// Native ECX renderer, six stack words, EAX owner or0, RET18h. Current real
// CreateVolumeTexture +60, exact optional B29670 retry, actual volume pool,
// genuine B3D720 and borrowed renderer+1B00 append. No COM retain/release,
// canonical bind, array retain, rollback, descriptor default or retry loop.
// B340A0/B3D720 backing and defined-descriptor-read contracts apply. SAME
// actual renderer/pool/serial/strings/retained/guard/recreation domains must
// remain valid. Current raw renderer/header/slot representation accesses need
// caller-proven compatible lifetimes/backing; no enclosing owner/header starts.
// Construction failure returns CURRENT height-word slot after inner cleanup,
// while count/COM obligations survive. Append failure leaves constructed owner.
// Pool return is before possible later canonical retirement; full outer
// quiescence through terminal/unbind/Entry/caller completion is required.
// Borrowed-array traversal stays quiescent through real backing disposal once
// owner addresses become dangling. New C++ API, not native FH3/ABI/game proof.
void* create_native_runtime_volume_texture_00b2a5a0(void* actual_renderer,
    volatile NativeRuntimeVolumeTextureCreationArguments&,
    NativeRuntimeVolumeTextureCreationContext&,
    NativeRuntimeVolumeTextureCreationAcquired&);
} // namespace bsp
