#pragma once

#include "bsp/camera_projection.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/sound_class_ownership.hpp"
#include "bsp/sound_configuration.hpp"
#include "bsp/panel_sequence_types.hpp"

#include <array>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bsp {

// Constructor behavior projections, not native layouts, vtables or ABIs.
// Native offsets and uncertainty: docs/SOUND_SYSTEM_OWNER.md.
struct SoundOwnedResource; // Concrete projection: sound_resource_asset.hpp.

struct SoundResourceCacheRecord {
    std::string name_00;
    std::uint32_t field_08{};
    std::list<std::string> aliases_0c;
    std::array<std::uint32_t, 5> metadata_14{};
    SoundOwnedResource* resource_28{};
};

struct SoundResourceOwner {
    std::uint32_t native_vtable_00{};
    std::vector<SoundResourceCacheRecord> records_04; // native 2Ch records
    std::int32_t capacity_0c{};
    std::uint32_t total_resource_size_10{};
    SoundOwnedResource* error_resource_14{};
};

// The temporary 48h native record at 00A85969..00A859A3. Unknown names stay
// offset names. Unwritten +28/+2C remain explicitly unknown, not zero defaults.
// Padding bytes are omitted; standard storage owns the +3C/+40/+44 vector.
struct SoundResourceLoadOptions {
    std::uint32_t field_00{};
    float field_04{1.0f};
    float field_08{1.0f};
    float field_0c{10.0f};
    float field_10{1000000000.0f};
    std::array<std::uint8_t, 3> flags_14{};
    float field_18{};
    float field_1c{};
    std::array<std::uint8_t, 3> flags_20{};
    std::int32_t field_24{-1};
    std::optional<std::uint32_t> unwritten_28;
    std::optional<std::uint32_t> unwritten_2c;
    float field_30{};
    float field_34{};
    std::uint8_t flag_38{};
    std::vector<std::array<std::uint32_t, 4>> records_3c;
    std::int32_t capacity_44{};
};

class SoundResourceLoadHost {
public:
    virtual ~SoundResourceLoadHost() = default;
    // Native ECX=18h (24-byte) owner; stack NativeString*, options*, clone byte,
    // load-if-missing byte; RET10. Both bytes are 1 at 00A859F6.
    // SoundResourceRuntime supplies the recovered loader using actual VFS/FMOD.
    // It normalizes/looks up aliases and reaches vslots +4/+8/+C to resolve,
    // create and clone resources; it can populate the same records_04 and +10.
    // Returned +14 retains its own native resource reference. The implementation
    // must provide the actual eventual 00A85A50/00A85500 teardown; the runtime
    // adapter exposes it explicitly. C++ container
    // destruction does not release opaque game assets or FMOD resources.
    virtual SoundOwnedResource* load_00a84740(SoundResourceOwner& owner,
        const NativeString& path, SoundResourceLoadOptions& options,
        bool clone, bool load_if_missing) = 0;
    // Required meaningful game cleanup on constructor failure. Called only
    // after temporary path/options destruction, while the acquired cache and
    // owner accounting still exist. This is native base-cache teardown A85500,
    // not derived A85A50: +14 has not received a successful loader result.
    // The host must release acquired resources; clearing C++ pointers is not
    // an implementation of native resource release. Must not throw in unwind.
    virtual void destroy_failed_owner_00a85500(SoundResourceOwner&) noexcept = 0;
};

inline constexpr char kSoundErrorResourcePath[] = "sound/gui/error.fsb";

struct SoundAuxiliaryTreeValue {
    void* sample_14{}; // Weak raw intrusive sample pointer; no tree retain.
};

// Native 10h owner with tree header +4, head pointer +8, count +C. The native
// 1Ch head is black/isnil and self-linked. Sound sample cache lookup/insertion
// uses native pooled keys and00443D00 ordering; see sound_sample_cache.hpp.
// Explicit clear releases keys. C++ map destruction alone is not that cleanup.
struct SoundAuxiliaryTreeOwner {
    using Tree = std::map<NativeString, SoundAuxiliaryTreeValue, PanelSequenceNameLess>;
    std::uint32_t native_vtable_00{};
    std::unique_ptr<Tree> tree_04; // allocated after singleton publication
    std::uint32_t count_0c{}; // Explicit native count, including cleanup callback timing.
};

struct SoundListenerOwnerState {
    std::uint32_t native_vtable_a4{};
    std::uint32_t references_a8{};
    // Historical name: A7E630 submits this as FMOD listener velocity. Listener
    // position is transform_c4[12..14]; see docs/SOUND_SYSTEM_UPDATE.md.
    std::array<float, 3> position_b8{};
    CameraMatrix transform_c4{};
};

struct SoundSystemOwner {
    SoundSystemOwner(SoundSystemState&, SoundConfigurationState&,
        SoundManagerLevels&, SoundClassOwnership&) noexcept;
    SoundSystemOwner(const SoundSystemOwner&) = delete;
    SoundSystemOwner& operator=(const SoundSystemOwner&) = delete;

    // Bindings refer to the canonical state, including scalar defaults, FMOD
    // handles, enabled byte, global level, listeners, entries and class table.
    SoundSystemState& system;
    SoundConfigurationState& configuration;
    SoundManagerLevels& levels;
    SoundClassOwnership& classes;

    std::uint32_t native_vtable_00{};
    std::uint8_t flag_50{};
    std::unique_ptr<SoundResourceOwner> resource_owner_54;
    std::array<std::uint32_t, 4> words_58{};
    std::uint8_t flag_68{};
    std::uint8_t flag_69{};
    float level_6c{};
    std::array<void*, 3> pointers_74{};
    std::array<void*, 3> pointers_80{};
    std::int32_t entry_capacity_94{};
    SoundListenerOwnerState listener;
    // +114 is not written; no invented semantic field. +118 is four words,
    // not four floats: its constructor default is {0,0,1,0}.
    std::array<std::uint32_t, 4> time_118{};
    std::array<std::uint32_t, 5> words_144{}; // +144..+154; +158 is system.min_frequency
    std::array<std::uint32_t, 4> words_160{};
};

struct SoundOwnerLifetimeBindings {
    SingletonLifetimeDomain& domain;
    SoundSystemOwner* volatile& global_00f8bbd8;
    SoundAuxiliaryTreeOwner* volatile& global_00f8bbe8;
};

// All native constructors take ECX=this, no stack args, return this in EAX.
// Factories' C++ allocation is a convenience; 00A88770 allocates separately.
void register_sound_system_owner_00a7b190(SoundSystemOwner&,
    SoundOwnerLifetimeBindings&);
void register_sound_auxiliary_tree_owner_00a880e0(SoundAuxiliaryTreeOwner&,
    SoundOwnerLifetimeBindings&);
void construct_sound_listener_owner_00a7fd40(SoundListenerOwnerState&,
    SoundConfigurationState&);
// Fresh construction: canonical containers/classes must be empty and the
// class owner must bind levels. Unwritten native fields retain caller state.
void construct_sound_system_owner_00a81480(SoundSystemOwner&,
    SoundOwnerLifetimeBindings&);
std::unique_ptr<SoundAuxiliaryTreeOwner>
create_sound_auxiliary_tree_owner_00a88650(SoundOwnerLifetimeBindings&);
std::unique_ptr<SoundResourceOwner> create_sound_resource_owner_00a858f0(
    SoundResourceLoadHost&, NativeStringStorage& = crt_string_storage());

// ONLY singleton-base teardown, not whole owner/resource/FMOD destruction.
// Native unregisters the current global slot (which can differ from &owner),
// clears it unconditionally, then changes owner's vtable to CE3818.
// Call appropriate full teardown before these; remove registrations before
// discarding projection storage or shutting down its lifetime domain.
void unregister_sound_system_owner_00a7b230(SoundSystemOwner&,
    SoundOwnerLifetimeBindings&);
void unregister_sound_auxiliary_tree_owner_00a88180(SoundAuxiliaryTreeOwner&,
    SoundOwnerLifetimeBindings&);

} // namespace bsp
