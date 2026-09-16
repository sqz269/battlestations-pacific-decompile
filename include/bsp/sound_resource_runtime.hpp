#pragma once
#include "bsp/fmod_configuration_library.hpp"
#include "bsp/frame_clock.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/resource_load_events.hpp"
#include "bsp/sound_resource_cache.hpp"
#include "bsp/sound_startup.hpp"
#include "bsp/vfs_file_date.hpp"
#include "bsp/vfs_lua_scripts.hpp"

namespace bsp {
struct NativeFrameClockPublicationContext;

// One borrowed clock representation, selected at construction. Implicit
// conversions preserve existing semantic service aggregate initializers.
// This source binding is not an original object/aggregate ABI.
class SoundClockBinding final {
public:
    SoundClockBinding(const FrameClock& clock) noexcept : semantic_(&clock), actual_(nullptr) {}
    SoundClockBinding(const NativeFrameClockPublicationContext& clock) noexcept
        : semantic_(nullptr), actual_(&clock) {}
    std::array<std::uint32_t, 4> startup_words_slot14() const;
    // Raw mode copies only the current 16-byte result into caller storage.
    // Semantic mode preserves its existing borrowed current-member pointer.
    const ClockTimestamp* current_slot14(ClockTimestamp& result) const;
private:
    const FrameClock* const semantic_;
    const NativeFrameClockPublicationContext* const actual_;
};

inline SoundResourceOwner* get_sound_resource_owner_00a79910(SoundSystemOwner& owner) noexcept {
    return owner.resource_owner_54.get();
}
// Composition of recovered game routines and actual VFS/FMOD services. All
// references outlive this adapter and all retained resources. Calls are
// serialized on the owning game thread; mount/provider callbacks are stable.
// The application's actual platform focus/XLive policy remains required.
class SoundResourceRuntime final : public SoundResourceLoadHost,
    public SoundResourceCacheHost, public SoundResourceAssetLifetimeHost,
    public SoundResourceCacheRemovalHost {
public:
    SoundResourceRuntime(SoundSystemState&, SoundSystemOwner* volatile& current_owner,
        volatile std::uint32_t& actual_resource_bytes, VfsMountContext&,
        const VfsCandidateRegistrations&, ResourceLoadEventHost&,
        FmodConfigurationLibrary&, NativeStringStorage& = crt_string_storage());
    SoundOwnedResource* load_00a84740(SoundResourceOwner&, const NativeString&,
        SoundResourceLoadOptions&, bool clone, bool load_if_missing) override;
    void destroy_failed_owner_00a85500(SoundResourceOwner&) noexcept override;
    void destroy_owner(SoundResourceOwner&);
    void release_resource(SoundOwnedResource&);

    void pump_load_events_00beccd0() override;
    NativeString resolve_00a82ea0(SoundResourceOwner&, const NativeString&,
        SoundResourceLoadOptions&, NativeStringStorage&) override;
    SoundOwnedResource* create_00a835b0(SoundResourceOwner&, const NativeString&,
        SoundResourceLoadOptions&) override;
    SoundOwnedResource* retain_00a854c0(SoundResourceOwner&, SoundOwnedResource&) override;
    std::uint32_t resource_size_vslot_0c(SoundOwnedResource&) override;
    VfsFileDate query_metadata_00bdd340(const NativeString&) override;
    void delete_resource_00a85ac0(SoundOwnedResource&, std::uint32_t flags) override;
    void remove_from_current_cache(const NativeString&) override;
private:
    SoundSystemState& system_;
    SoundSystemOwner* volatile& current_owner_;
    VfsMountContext& mounts_;
    const VfsCandidateRegistrations& registrations_;
    ResourceLoadEventHost& events_;
    FmodConfigurationLibrary& library_;
    NativeStringStorage& strings_;
    SoundResourceCleanupContext cleanup_;
};

// Raw startup +14 reads current (A887F5), without sampling. The legacy semantic
// path retains its earlier sampling behavior and is not native +14 proof.
// Borrowed clock/context and result lifetime remain the caller's responsibility.
class FrameClockSoundStartupHost final : public SoundStartupClockHost {
public:
    explicit FrameClockSoundStartupHost(const FrameClock& clock) : clock_(clock) {}
    explicit FrameClockSoundStartupHost(const NativeFrameClockPublicationContext& clock) : clock_(clock) {}
    explicit FrameClockSoundStartupHost(SoundClockBinding clock) : clock_(clock) {}
    std::array<std::uint32_t, 4> sample_time_14() override;
    const ClockTimestamp* current_timestamp_slot14();
private:
    SoundClockBinding clock_;
    ClockTimestamp current_result_;
};

// Uses real Lua 5.1.1, cached installed fundamentals, canonical VFS and the
// recovered chunk loader. Existing host protection raises C++ on native fatal
// Lua errors; no native panic/exception ABI is claimed.
class VfsSoundConfigurationLuaOwner final : public SoundConfigurationLuaOwner {
public:
    VfsSoundConfigurationLuaOwner(VfsLuaScriptFiles&, LuaScriptRuntime&,
        const LuaRuntimeGlobals&) noexcept;
    ~VfsSoundConfigurationLuaOwner() override;
    GuiLuaHost& construct_and_open(std::uint32_t library_mask) override;
    void load_and_call_chunk(const char* path, std::int32_t argument) override;
    void close() noexcept override;
private:
    VfsLuaScriptFiles& files_;
    LuaScriptRuntime& runtime_;
    const LuaRuntimeGlobals& globals_;
    std::unique_ptr<PcStorageLuaOwner> owner_;
    std::unique_ptr<GuiLua51Host> reader_;
};
}
