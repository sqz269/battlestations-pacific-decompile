#pragma once
#include "bsp/sound_configuration_shutdown.hpp"
#include "bsp/sound_system_update.hpp"

namespace bsp {
class SoundResourceRuntime;
class SoundChannelRuntime;

class SoundShutdownFmodHost : public SoundConfigurationShutdownFmodHost {
public:
    // Native EventSystem vslot4 takes explicit self on the stack (A7E3DC).
    virtual FmodResult release_event_system(void*) = 0;
};

// Required virtual/reference operations on the two retained pointer arrays.
// A7F2F0 produces +80 from actual sample references and +74 from the distinct
// D5AD58/D5ADA0/D5ADE8 channel variants, including their additional +5C records.
// Release implements atomic decrement of the receiver's native +4 ownership
// and zero-reference vslot0. Adapters map that operation to their actual type.
// There is no default no-op, alternate engine destructor or synthetic reference
// store. Release callbacks must not throw, matching the existing reference host.
class SoundShutdownVirtualHost {
public:
    virtual ~SoundShutdownVirtualHost() = default;
    virtual void stop_retained_74_slot08(void*, std::uint32_t flag) = 0;
    virtual void release_retained_74_reference(void*) noexcept = 0;
    virtual void release_retained_80_reference(void*) noexcept = 0;
    virtual void delete_alternate_slot00(void*, std::uint32_t flags) = 0;
};

// Alternate engine destruction remains a required independently bound service.
class SoundAlternateShutdownHost {
public:
    virtual ~SoundAlternateShutdownHost() = default;
    virtual void delete_alternate_slot00(void*, std::uint32_t flags) = 0;
};
// Concrete +74 channel and +80 actual-sample reference operations. All services
// outlive shutdown. Actual sample zero-reference dispatch is the same lifetime
// used by SoundInstanceContext, normally SoundSampleRuntime.
class SoundRetainedShutdownRuntime final : public SoundShutdownVirtualHost {
public:
    SoundRetainedShutdownRuntime(SoundChannelRuntime&, GameplayEffectComponentLifetime&,
        SoundAlternateShutdownHost&) noexcept;
    void stop_retained_74_slot08(void*, std::uint32_t) override;
    void release_retained_74_reference(void*) noexcept override;
    void release_retained_80_reference(void*) noexcept override;
    void delete_alternate_slot00(void*, std::uint32_t) override;
private:
    SoundChannelRuntime& channels_;
    GameplayEffectComponentLifetime& samples_;
    SoundAlternateShutdownHost& alternate_;
};

struct SoundSystemShutdownContext {
    SoundOwnerLifetimeBindings& lifetime;
    SoundShutdownVirtualHost& virtuals;
    SoundSystemUpdateContext& update;
    SoundResourceRuntime& resources;
    SoundShutdownFmodHost& fmod;
    void* volatile& alternate_00f8bbcc;
    NativeStringStorage& strings = crt_string_storage();
};

// Full normal control flow, new C++ interfaces, not original layouts/ABIs.
// All four destructor/cleanup bodies take native ECX=self, RET, no stack args.
// Calls are serialized; captured array storage must survive release callbacks.
void stop_retained_sound_pointers_00a7b9c0(SoundSystemOwner&, SoundShutdownVirtualHost&);
// Drain entries, two fresh identity/zero listener updates, delete resources,
// release current EventSystem. Leaves +44/+48/+54 native pointer words unchanged.
// Supports the reconstructed D5B000/D5B44C manager and D5B210 resource profiles.
void close_sound_system_00a7e240(SoundSystemOwner&, SoundSystemShutdownContext&);
// Storage/base cleanup only. Does not release live FMOD handles/resources.
// Includes the DEBB18 reverse-member unwind sequence, not native SEH ABI.
void destroy_sound_system_base_00a816b0(SoundSystemOwner&, SoundSystemShutdownContext&);
void destroy_sound_system_00a882c0(SoundSystemOwner&, SoundSystemShutdownContext&);

// Native scalar wrappers: ECX=self, stack flags byte, RET4; return original
// pointer even after freeing. Flag bit0 frees a separately new-allocated owner.
SoundSystemOwner* scalar_delete_sound_system_00a883b0(SoundSystemOwner*,
    std::uint8_t flags, SoundSystemShutdownContext&);
SoundSystemOwner* scalar_delete_sound_system_base_00a817d0(SoundSystemOwner*,
    std::uint8_t flags, SoundSystemShutdownContext&);
// C++ ownership-slot interface for the A85AE0 wrapper. Runs A85A50 first, then
// frees storage on bit0 while preserving the calling manager's dead +54 word.
SoundResourceOwner* scalar_delete_sound_resource_owner_00a85ae0(
    SoundResourceOwnerSlot&, std::uint8_t flags, SoundResourceRuntime&);
} // namespace bsp
