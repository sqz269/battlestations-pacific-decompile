#pragma once
#include "bsp/sound_system_update.hpp"
#include "bsp/sound_spatial_instance.hpp"

namespace bsp {
// Concrete active-entry/lifetime binding for D5ABF8 and optional D5B510 projections.
// All incoming void pointers must be the canonical SoundLevelEntry subobject,
// never a native bank/sample pointer. The context and services outlive entries.
// Does not own a parallel list. Factory returns one reference; tracked creation
// through A7E490 adds the manager reference using this same lifetime interface.
class SoundChannelRuntime final : public SoundActiveEntryHost {
public:
    explicit SoundChannelRuntime(SoundInstanceContext& context) noexcept : context_(context) {}
    explicit SoundChannelRuntime(SoundSpatialChannelContext& context) noexcept
        : context_(context.instance), spatial_(&context) {}
    SoundLevelEntry* create_nonspatial(SoundSystemOwner&, void* sample,
        std::int32_t class_index, std::int32_t type_index, bool flag);
    SoundLevelEntry* create_spatial_bank(SoundSystemOwner&, void* sample,
        std::int32_t class_index, std::int32_t type_index, bool flag);
    void retain_reference(void*) override;
    void release_reference(void*) noexcept override;
    SoundInstance& instance_fields(SoundLevelEntry*) override;
    void update_slot30(SoundLevelEntry*, float, SoundListenerOwnerState&) override;
    bool transition_slot24(SoundLevelEntry*) override;
    void refresh_slot10(SoundLevelEntry*) override;
    bool completed_slot0c(SoundLevelEntry*) override;
    bool nonvirtual_slot14(SoundLevelEntry*) override;
    void stop_slot08(SoundLevelEntry*, std::uint8_t) override;
private:
    SoundChannelInstance& channel(SoundLevelEntry*);
    SoundInstanceContext& context_;
    SoundSpatialChannelContext* spatial_{};
};
} // namespace bsp
