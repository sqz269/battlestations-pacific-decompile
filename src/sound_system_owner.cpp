#include "bsp/sound_system_owner.hpp"

#include <cassert>
#include <cstring>

namespace bsp {
namespace {

// Capture the first manager's section, while native registration gets the
// manager again. Adjust the native recursion word once around the OS lock.
using CapturedSoundSection = CapturedSoundLifetimeSection;

class TemporarySoundPath final {
public:
    explicit TemporarySoundPath(NativeStringStorage& storage) : storage_(storage) {
        value.resize_0041dd40(storage_, 0x13u, true);
        std::memcpy(value.data(), kSoundErrorResourcePath, value.length() + 1u);
    }
    ~TemporarySoundPath() {
        destroy_native_string_header_0041dd20(&value, storage_);
    }
    NativeString value;
private:
    NativeStringStorage& storage_;
};

} // namespace

SoundSystemOwner::SoundSystemOwner(SoundSystemState& system_state,
    SoundConfigurationState& configuration_state, SoundManagerLevels& level_state,
    SoundClassOwnership& class_owner) noexcept
    : system(system_state), configuration(configuration_state), levels(level_state),
      classes(class_owner) {}

void register_sound_system_owner_00a7b190(SoundSystemOwner& owner,
    SoundOwnerLifetimeBindings& lifetime) {
    owner.native_vtable_00 = 0x00d5abacu;
    try {
        CapturedSoundSection section(lifetime.domain);
        lifetime.global_00f8bbd8 = &owner;
        lifetime.domain.get_manager_00415350()->register_object(lifetime.global_00f8bbd8);
    } catch (...) {
        // DEAEDC unwinds the captured guard, then CB4FD0 -> 00412430.
        // Neither action clears the published global or unregisters it.
        owner.native_vtable_00 = 0x00ce3818u;
        throw;
    }
}

void register_sound_auxiliary_tree_owner_00a880e0(SoundAuxiliaryTreeOwner& owner,
    SoundOwnerLifetimeBindings& lifetime) {
    owner.native_vtable_00 = 0x00d5b448u;
    try {
        CapturedSoundSection section(lifetime.domain);
        lifetime.global_00f8bbe8 = &owner;
        lifetime.domain.get_manager_00415350()->register_object(lifetime.global_00f8bbe8);
    } catch (...) {
        // DEC36C has the same guard/root-base unwind via CB6208/CB6200.
        owner.native_vtable_00 = 0x00ce3818u;
        throw;
    }
}

void construct_sound_listener_owner_00a7fd40(SoundListenerOwnerState& owner,
    SoundConfigurationState& configuration) {
    owner.native_vtable_a4 = 0x00ceb130u;
    owner.references_a8 = 1;
    owner.native_vtable_a4 = 0x00d5aec4u;
    configuration.listeners_ac.clear();
    configuration.listener_capacity_b4 = 0;
    const CameraMatrix identity{
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    copy_camera_matrix_004134f0(owner.transform_c4, identity);
    owner.position_b8.fill(0.0f);
    configuration.selected_listener_104 = -1; // native null pointer
    configuration.selected_listener_108 = 0;
}

void construct_sound_system_owner_00a81480(SoundSystemOwner& owner,
    SoundOwnerLifetimeBindings& lifetime) {
    assert(&owner.classes.manager() == &owner.levels);
    assert(owner.classes.capacity() == 0 && owner.levels.classes_98.empty());
    assert(owner.levels.entries_8c.empty());
    assert(owner.configuration.types_38.empty());
    assert(owner.configuration.listeners_ac.empty());
    assert(owner.configuration.channel_groups_128.empty());
    assert(owner.configuration.system_dsps_134.empty());
    register_sound_system_owner_00a7b190(owner, lifetime);
    owner.configuration.scalars = SoundConfigurationScalars{};
    owner.native_vtable_00 = 0x00d5b000u;
    owner.configuration.type_capacity_40 = 0;
    owner.levels.global_4c = 1.0f;
    owner.flag_50 = 0;
    owner.words_58.fill(0);
    owner.flag_68 = 0;
    owner.flag_69 = 0;
    owner.level_6c = 1.0f;
    owner.system.sound_enabled = true;
    // 0054D440 and 004BA0D0 each store one null dword; EH array counts are 3.
    owner.pointers_74.fill(nullptr);
    owner.pointers_80.fill(nullptr);
    owner.entry_capacity_94 = 0;
    construct_sound_listener_owner_00a7fd40(owner.listener, owner.configuration);
    owner.configuration.current_listener_10c = -1;
    owner.time_118 = {0, 0, 1, 0};
    owner.configuration.channel_group_capacity_130 = 0;
    owner.configuration.system_dsp_capacity_13c = 0;
    owner.configuration.master_channel_group_140 = nullptr;
    owner.words_144.fill(0);
    owner.words_160.fill(0);
    // Same raw words later used as queried frequency outputs by 00A88770.
    owner.system.min_frequency = 100;
    owner.system.max_frequency = 0x00432380;
    owner.pointers_74.fill(nullptr); // repeated final stores 00A8168D..92
}

std::unique_ptr<SoundAuxiliaryTreeOwner>
create_sound_auxiliary_tree_owner_00a88650(SoundOwnerLifetimeBindings& lifetime) {
    auto owner = std::make_unique<SoundAuxiliaryTreeOwner>();
    register_sound_auxiliary_tree_owner_00a880e0(*owner, lifetime);
    owner->native_vtable_00 = 0x00d5b460u;
    // 00A88270 allocates a black node (isnil=0); the caller turns it into the
    // self-linked isnil=1 head, then writes count=0. Project its empty invariant.
    try {
        owner->tree_04 = std::make_unique<SoundAuxiliaryTreeOwner::Tree>();
    } catch (...) {
        // FuncInfo DEC408 -> unwind map DEC400 -> CB6260 calls A88180
        // when construction of the tree head throws after base publication.
        unregister_sound_auxiliary_tree_owner_00a88180(*owner, lifetime);
        throw;
    }
    return owner;
}

std::unique_ptr<SoundResourceOwner> create_sound_resource_owner_00a858f0(
    SoundResourceLoadHost& loader, NativeStringStorage& strings) {
    auto owner = std::make_unique<SoundResourceOwner>();
    owner->native_vtable_00 = 0x00d5b210u;
    try {
        SoundResourceLoadOptions options;
        {
            TemporarySoundPath path(strings);
            owner->error_resource_14 = loader.load_00a84740(
                *owner, path.value, options, true, true);
        } // Native releases the path before resizing/freeing options.records_3c.
        options.records_3c.clear(); // 0093F950(0): reverse count shrink, trivial records
    } catch (...) {
        // CB5F98 -> DEC080/DEC068: CB5F90 destroys the path, CB5F88 destroys
        // options, then CB5F80 calls base-cache teardown A85500. The nested
        // scopes above have already unwound; opaque resources are still alive.
        loader.destroy_failed_owner_00a85500(*owner);
        throw;
    }
    return owner; // vector allocation released by C++ storage destruction
}

void unregister_sound_system_owner_00a7b230(SoundSystemOwner& owner,
    SoundOwnerLifetimeBindings& lifetime) {
    owner.native_vtable_00 = 0x00d5abacu;
    {
        CapturedSoundSection section(lifetime.domain);
        lifetime.domain.get_manager_00415350()->unregister_object(lifetime.global_00f8bbd8);
        lifetime.global_00f8bbd8 = nullptr;
    }
    owner.native_vtable_00 = 0x00ce3818u;
}

void unregister_sound_auxiliary_tree_owner_00a88180(SoundAuxiliaryTreeOwner& owner,
    SoundOwnerLifetimeBindings& lifetime) {
    owner.native_vtable_00 = 0x00d5b448u;
    {
        CapturedSoundSection section(lifetime.domain);
        lifetime.domain.get_manager_00415350()->unregister_object(lifetime.global_00f8bbe8);
        lifetime.global_00f8bbe8 = nullptr;
    }
    owner.native_vtable_00 = 0x00ce3818u;
}

} // namespace bsp
