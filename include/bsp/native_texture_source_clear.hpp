#pragma once
#include "bsp/native_render_context.hpp"
#include <cstdint>

namespace bsp {
// Host-only one-shot diagnostic; no field/count/ownership credit in the owner.
struct NativeTextureSourceClearOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    void* owner{};
    void* child{};
    void** array{};
    void** captured_slot{};
    std::uint32_t index{}, native_site{};
    bool child_release_started{}, child_release_returned{}, slot_clear_returned{};
    bool resize_started{}, resize_returned{}, initialized_clear_returned{};
    NativeTextureSourceClearOperation() = default;
    ~NativeTextureSourceClearOperation();
    NativeTextureSourceClearOperation(const NativeTextureSourceClearOperation&) = delete;
    NativeTextureSourceClearOperation& operator=(const NativeTextureSourceClearOperation&) = delete;
    // Frees/releases nothing. Caller first resolves the retained partial state.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// C303B0..C30403, actual ECX receiver/no stack args/plain RET. Fresh array and
// unsigned count per forward iteration; null skip; actual+04 decrement and
// zero-only CURRENT slot0; clear SAME captured slot after normal return.
// Finally typed737390(header,0), then actual+1C=0 after its normal return.
// Requires the already-live separate payload placed by genuine C30470 and
// reached live void* slots. It cannot adopt raw images or construct objects.
// Caller must separately prove each nonnull child's live actual+04 atomic
// and SAME canonical companion. NativeTextureLoadOwners supports current
// D61948/D61870/D618B0 slot0 BD30E0 -> current scalar slot04(flags1), but this
// is zero-profile correspondence, not a C++ lifetime-start proof. Fresh
// B319B0/current2D B3F930 -> B34230 -> B34120 DWORD1 writes, raw slab backing,
// and Entry's atomic reference borrow do not start that atomic lifetime.
// This clear leaf neither constructs nor resets a count and admits no
// arbitrary terminal callback or separate count wrapper.
// Keep owner/header and each captured slot backing alive through reentrant
// terminal callbacks; retain all current profile/pool/renderer/registry context.
// No selected/rate reset, array free, base/profile store, or owner decrement.
// New source interface; no binary ABI/FH3/hardware-fault or runtime proof.
void clear_native_texture_source_children_00c303b0(void* actual_owner,
    NativeRenderActualOwners&, NativeTextureSourceClearOperation&);
} // namespace bsp
