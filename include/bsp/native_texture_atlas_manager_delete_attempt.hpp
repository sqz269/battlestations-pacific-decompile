#pragma once
#include <cstdint>

namespace bsp {
struct NativeSingletonDeletionBindings;
class NativeTextureAtlasDeletionLifetime;
class NativeGuiSelectedManagerDeleteAttempt;
namespace game { class GameSingletonHost; class GameNativeTextureAtlasApplication; }

// Negative-only ordinary Source record. The actual singleton Host privately
// constructs/adopts it before atlas Native birth; the actual application supplies
// its receiver only after whole737EE0 and the SAME base registration return.
// No public constructor, setter, profile/count/flag or copied binding mints it.
// It retains metadata, not the external graph: the actual heap StartupHost and
// canonical process owner must survive both catches and the full source fence.
class NativeTextureAtlasManagerDeleteAttempt final {
public:
    NativeTextureAtlasManagerDeleteAttempt(const NativeTextureAtlasManagerDeleteAttempt&) = delete;
    NativeTextureAtlasManagerDeleteAttempt& operator=(const NativeTextureAtlasManagerDeleteAttempt&) = delete;
    NativeTextureAtlasManagerDeleteAttempt(NativeTextureAtlasManagerDeleteAttempt&&) = delete;
    NativeTextureAtlasManagerDeleteAttempt& operator=(NativeTextureAtlasManagerDeleteAttempt&&) = delete;
    bool requires_source_retention() const noexcept {
        return unknown_ || (host_entered_ && !source_fence_returned_);
    }
private:
    friend class game::GameSingletonHost;
    friend class game::GameNativeTextureAtlasApplication;
    friend void __fastcall destroy_native_singleton_manager_00bd0400(void*,
        const NativeSingletonDeletionBindings&, NativeTextureAtlasManagerDeleteAttempt&);
    friend void __fastcall destroy_native_singleton_manager_00bd0400(void*,
        const NativeSingletonDeletionBindings&, NativeGuiSelectedManagerDeleteAttempt&,
        NativeTextureAtlasManagerDeleteAttempt&);
    friend void* dispatch_native_texture_atlas_selected_scalar(void*, std::uint32_t,
        const NativeSingletonDeletionBindings&, NativeTextureAtlasManagerDeleteAttempt&);
    NativeTextureAtlasManagerDeleteAttempt(game::GameSingletonHost& host,
        NativeTextureAtlasDeletionLifetime& atlas, const NativeSingletonDeletionBindings& bindings)
        : host_(&host), atlas_(&atlas), bindings_(&bindings) {}
    game::GameSingletonHost* const host_;
    NativeTextureAtlasDeletionLifetime* const atlas_;
    const NativeSingletonDeletionBindings* const bindings_;
    void* outer_receiver_{}; // actual manager capture in real Host shutdown
    void* registered_receiver_{}; // actual normal birth, never CURRENT F8C26C
    void* scalar_receiver_{}; // actual popped receiver, retained before admission
    std::uint32_t scalar_flags_{}; // full real flags; never a proof of origin
    bool cold_bound_{}, registered_receiver_bound_{};
    bool host_entered_{}, receiver_bound_{}, core_entered_{}, core_returned_{};
    bool scalar_entered_{}, scalar_returned_{}, host_returned_{};
    bool source_fence_returned_{}, unknown_{};
};

// Source adapter only. Capture/arm precedes all throwing admission; a failure
// retains this SAME record and does not invoke cleanup/unlock/free/retry.
void* dispatch_native_texture_atlas_selected_scalar(void* actual_popped_receiver,
    std::uint32_t actual_full_flags, const NativeSingletonDeletionBindings&,
    NativeTextureAtlasManagerDeleteAttempt&);
} // namespace bsp
