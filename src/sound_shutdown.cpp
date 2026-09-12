#include "bsp/sound_shutdown.hpp"
#include "bsp/sound_listener.hpp"
#include "bsp/sound_resource_runtime.hpp"
#include "bsp/sound_channel_runtime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdlib>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
void update_current_sound_virtual(SoundSystemOwner& owner, const CameraMatrix& matrix,
    SoundSystemUpdateContext& context) {
    // Reload the current profile at each native call; no assumed dt parameter.
    switch (owner.native_vtable_00) {
    case 0x00d5b44cu:
        update_sound_system_00a87bf0(owner, matrix, {0.0f, 0.0f, 0.0f}, context);
        break;
    case 0x00d5b000u:
        update_sound_system_base_00a7e630(owner, matrix, {0.0f, 0.0f, 0.0f}, context);
        break;
    default:
        throw std::logic_error("Unreconstructed sound manager update vtable");
    }
}

void destroy_member_stage(SoundSystemOwner& owner, SoundSystemShutdownContext& context,
    unsigned stage) {
    auto& config = owner.configuration;
    switch (stage) {
    case 0: // +134 pointer storage only; EventSystem owns the DSP objects.
        shrink_sound_system_dsps_00a7a980_fragment(config, 0);
        std::vector<void*>{}.swap(config.system_dsps_134);
        break;
    case 1:
        shrink_sound_configured_groups_00a7f240_fragment(config, 0);
        std::vector<SoundConfiguredChannelGroup>{}.swap(config.channel_groups_128);
        break;
    case 2:
        destroy_sound_listener_owner_00a7fe00(config,
            owner.listener.native_vtable_a4, context.strings);
        break;
    case 3:
        owner.classes.resize_00a7c2c0(0);
        std::vector<SoundClassLevel*>{}.swap(owner.levels.classes_98);
        break;
    case 4:
        resize_sound_entries_00a7c1c0(owner, 0, context.update.entries);
        std::vector<SoundLevelEntry*>{}.swap(owner.levels.entries_8c);
        break;
    case 5: // BF7C6E, stride4/count3, 004C3810: reverse array destruction.
        for (std::size_t index = 3; index != 0;) {
            auto& slot = owner.pointers_80[--index];
            if (auto* value = slot) {
                context.virtuals.release_retained_80_reference(value);
                slot = nullptr;
            }
        }
        break;
    case 6: // BF7C6E, stride4/count3, 00524180.
        for (std::size_t index = 3; index != 0;) {
            auto& slot = owner.pointers_74[--index];
            if (auto* value = slot) {
                context.virtuals.release_retained_74_reference(value);
                slot = nullptr;
            }
        }
        break;
    case 7:
        shrink_sound_configured_types_00a7fcc0_fragment(config, 0);
        std::vector<SoundConfiguredType>{}.swap(config.types_38);
        break;
    case 8:
        unregister_sound_system_owner_00a7b230(owner, context.lifetime);
        break;
    }
}

struct SoundBaseUnwind {
    SoundSystemOwner& owner;
    SoundSystemShutdownContext& context;
    bool armed{true};
    ~SoundBaseUnwind() noexcept {
        if (armed) destroy_sound_system_base_00a816b0(owner, context);
    }
};
} // namespace

SoundRetainedShutdownRuntime::SoundRetainedShutdownRuntime(SoundChannelRuntime& channels,
    GameplayEffectComponentLifetime& samples, SoundAlternateShutdownHost& alternate) noexcept
    : channels_(channels), samples_(samples), alternate_(alternate) {}
void SoundRetainedShutdownRuntime::stop_retained_74_slot08(void* entry, std::uint32_t flag) {
    channels_.stop_slot08(static_cast<SoundLevelEntry*>(entry), static_cast<std::uint8_t>(flag));
}
void SoundRetainedShutdownRuntime::release_retained_74_reference(void* entry) noexcept {
    channels_.release_reference(entry);
}
void SoundRetainedShutdownRuntime::release_retained_80_reference(void* sample) noexcept {
    try {
        if (sample && InterlockedDecrement(reinterpret_cast<volatile LONG*>(static_cast<unsigned char*>(sample) + 4)) == 0)
            samples_.zero_references_slot_00(sample);
    } catch (...) { std::terminate(); }
}
void SoundRetainedShutdownRuntime::delete_alternate_slot00(void* alternate, std::uint32_t flags) {
    alternate_.delete_alternate_slot00(alternate, flags);
}

void stop_retained_sound_pointers_00a7b9c0(SoundSystemOwner& owner,
    SoundShutdownVirtualHost& host) {
    for (std::size_t index = 0; index != 3; ++index) {
        if (auto* value = owner.pointers_74[index]) {
            host.stop_retained_74_slot08(value, 0);
            // Native reloads +74 after stop; the stopped receiver can differ.
            if (auto* current = owner.pointers_74[index])
                host.release_retained_74_reference(current);
            owner.pointers_74[index] = nullptr;
        }
        if (auto* current = owner.pointers_80[index])
            host.release_retained_80_reference(current);
        owner.pointers_80[index] = nullptr;
    }
}

SoundResourceOwner* scalar_delete_sound_resource_owner_00a85ae0(
    SoundResourceOwnerSlot& slot, std::uint8_t flags, SoundResourceRuntime& resources) {
    auto* const owner = slot.get();
    resources.destroy_owner(*owner);
    if (flags & 1u) slot.destroy_storage_preserving_word();
    return owner;
}

void close_sound_system_00a7e240(SoundSystemOwner& owner,
    SoundSystemShutdownContext& context) {
    stop_retained_sound_pointers_00a7b9c0(owner, context.virtuals);
    while (!owner.levels.entries_8c.empty())
        pop_sound_entry_00a7d640(owner, context.update.entries);
    for (unsigned pass = 0; pass != 2; ++pass) {
        const CameraMatrix identity{1, 0, 0, 0, 0, 1, 0, 0,
            0, 0, 1, 0, 0, 0, 0, 1};
        update_current_sound_virtual(owner, identity, context.update);
    }
    if (owner.resource_owner_54) {
        if (owner.resource_owner_54->native_vtable_00 != 0x00d5b210u)
            throw std::logic_error("Unreconstructed sound resource deleting vtable");
        scalar_delete_sound_resource_owner_00a85ae0(owner.resource_owner_54, 1,
            context.resources);
    }
    // +48 is reloaded after resource destruction, even if callbacks changed it.
    if (context.fmod.release_event_system(owner.system.event_system) == FmodResult::err_memory) {
        std::int32_t current{}, maximum{};
        context.fmod.memory_get_stats(&current, &maximum);
    }
}

void destroy_sound_system_base_00a816b0(SoundSystemOwner& owner,
    SoundSystemShutdownContext& context) {
    owner.native_vtable_00 = 0x00d5b000u;
    for (unsigned stage = 0; stage != 9; ++stage) {
        try {
            destroy_member_stage(owner, context, stage);
        } catch (...) {
            // DEBB18 states 7..0 skip the member whose destructor has started,
            // then destroy every still-live member and singleton base in order.
            // A second exception during unwind terminates the host process.
            try {
                for (unsigned remaining = stage + 1; remaining != 9; ++remaining)
                    destroy_member_stage(owner, context, remaining);
            } catch (...) { std::terminate(); }
            throw;
        }
    }
}

void destroy_sound_system_00a882c0(SoundSystemOwner& owner,
    SoundSystemShutdownContext& context) {
    owner.native_vtable_00 = 0x00d5b44cu;
    SoundBaseUnwind unwind{owner, context};
    auto* manager = context.lifetime.domain.get_manager_00415350();
    manager->unregister_object(context.alternate_00f8bbcc);
    if (auto* alternate = context.alternate_00f8bbcc) {
        context.virtuals.delete_alternate_slot00(alternate, 1);
        context.alternate_00f8bbcc = nullptr;
    }
    shutdown_sound_configuration_effects_00a7f560(owner.configuration,
        owner.words_144, context.fmod);
    close_sound_system_00a7e240(owner, context);
    if (owner.system.field_174) {
        std::free(reinterpret_cast<void*>(owner.system.field_174));
        owner.system.field_174 = 0;
    }
    unwind.armed = false;
    destroy_sound_system_base_00a816b0(owner, context);
}

SoundSystemOwner* scalar_delete_sound_system_00a883b0(SoundSystemOwner* owner,
    std::uint8_t flags, SoundSystemShutdownContext& context) {
    destroy_sound_system_00a882c0(*owner, context);
    if (flags & 1u) delete owner;
    return owner;
}
SoundSystemOwner* scalar_delete_sound_system_base_00a817d0(SoundSystemOwner* owner,
    std::uint8_t flags, SoundSystemShutdownContext& context) {
    destroy_sound_system_base_00a816b0(*owner, context);
    if (flags & 1u) {
        // A816B0 never touches +54. Do not let the C++ ownership convenience
        // implicitly free that unrelated allocation when freeing the manager.
        (void)owner->resource_owner_54.release_storage_preserving_word();
        delete owner;
    }
    return owner;
}
} // namespace bsp
