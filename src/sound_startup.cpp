#include "bsp/sound_startup.hpp"

namespace bsp {

SoundSystemOwner& construct_sound_system_00a88770(SoundSystemOwner& owner,
    SoundOwnerLifetimeBindings& lifetime,
    std::unique_ptr<SoundAuxiliaryTreeOwner>& auxiliary_owner,
    bool sound_disabled, std::uint32_t, std::uint32_t,
    SoundStartupClockHost& clock, SoundResourceLoadHost& resources,
    FmodStartupHost& fmod_startup, SoundConfigurationFmodHost& fmod_configuration,
    SoundConfigurationLuaOwner& lua)
{
    construct_sound_system_owner_00a81480(owner, lifetime); // 00A88791
    owner.native_vtable_00 = 0x00d5b44c;
    owner.system.field_174 = 0;
    owner.system.sound_enabled = !sound_disabled;
    auxiliary_owner = create_sound_auxiliary_tree_owner_00a88650(lifetime);
    // Both constructors already registered their owners. This call changes the
    // destruction ordering; it does not register the sound manager a second time.
    auto* auxiliary = lifetime.global_00f8bbe8; // capture before getter,00A887D0
    lifetime.domain.get_manager_00415350()->move_object_after_00bd0d70(&owner, auxiliary);
    owner.time_118 = clock.sample_time_14(); // four literal word copies
    initialize_sound_library_00a8881e_fragment(fmod_startup, owner.system);
    owner.resource_owner_54 = create_sound_resource_owner_00a858f0(resources);
    initialize_sound_configuration_00a7ff80(owner.system, owner.configuration,
        owner.classes, fmod_configuration, lua);
    return owner;
}

} // namespace bsp
