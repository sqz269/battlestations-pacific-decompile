#pragma once

// Oct18b nC low: the 30 newly-defined rows in segments 60..64.
// Names are hypotheses. These interfaces preserve the existing raw/class families;
// additional calls are explicit contracts, not native entry-point wiring.
// Original ABIs and evidence: docs/OCT18B_NC_LOW.md and reports/oct18b_nc_low.json.

#include "bsp/cc13_left61.hpp"
#include "bsp/cc13_seg49.hpp"
#include "bsp/engine_seg49_64_78_cc13.hpp"
#include "bsp/support_seg61_cc13.hpp"

namespace bsp::support_seg61_cc13 {
// All four: __thiscall(ECX=self, flags), RET 4, EAX=self.
void* pending_init_lock_scalar_delete_00924150(Raw self, std::uint8_t flags, Seg61Host& host);
void* pending_entity_lock_scalar_delete_009241d0(Raw self, std::uint8_t flags, Seg61Host& host);
void* deferred_event_lock_scalar_delete_00924250(Raw self, std::uint8_t flags, Seg61Host& host);
void* sound_request_queue_scalar_delete_00940da0(Raw self, std::uint8_t flags, Seg61Host& host);
// __thiscall(ECX=buffer, text), RET 4, EAX=buffer. Appends no terminator.
Raw block_buffer_append_text_0094f700(Raw buffer, const char* text, Seg61Host& host);
}

namespace bsp::cc13_seg49 {
// Additional image calls needed by the existing tickable entity/Seg60Host family.
struct SinkingWreckHost : Seg60Host {
    virtual void physics_transform_expand_00c33650(const void* transform, float* matrix) = 0;
    virtual float physics_body_mass_00c31fc0(void* body) = 0;
    virtual void* physics_first_shape_00c31dc0(void* body) = 0;
    virtual void physics_shape_clear_flags_00c47f60(void* shape, std::uint32_t flags) = 0;
    // ECX=entity+390h owner, EDX=matrix, (mass, vector-by-value), RET 10h.
    virtual void spawn_sinking_wreck_debris_00935540(void* owner, const float* matrix,
                                                    float mass, const float* span) = 0;
};
// __thiscall(ECX=entity, float delta), RET 4. Normal flow only; no native ABI thunk.
void sinking_wreck_update_0092b920(Raw entity, float delta, SinkingWreckHost& host);
}

namespace bsp::cc13_left61 {
struct SingleEffectUpdateHost : Left61Host {
    virtual void point_effect_set_matrix_0053d9c0(void* effect, const float* matrix) = 0;
    virtual bool point_effect_finished_00865fe0(void* effect) = 0;
    virtual void effect_reference_adopt_00484620(void** slot, void* value) = 0;
    virtual void mission_entity_kill_00926d90(void* entity, std::int32_t reason) = 0;
};
// ECX=entity, one unused stack argument (the update delta), RET 4. SEH unwind
// metadata is not reproduced; counted ownership and ordinary control flow are.
void single_effect_entity_update_0094f9e0(Raw entity, float unused_delta, SingleEffectUpdateHost& host);
}

namespace bsp::cc13_seg49_64_78 {
// Descriptor virtual methods: no stack arguments, RET, result in EAX.
std::uint32_t event_param_kind_00968090() noexcept;
std::uint32_t event_param_kind_009680f0() noexcept;
std::uint32_t event_param_kind_00968130() noexcept;
std::uint32_t event_param_kind_00968170() noexcept;
// __thiscall(ECX=parameter, flags), RET 4, EAX=parameter. Writes base vptr.
void* event_param_scalar_delete_009682b0(void* parameter, std::uint8_t flags,
                                       support_seg61_cc13::Seg61Host& host);
void* event_param_scalar_delete_009682d0(void* parameter, std::uint8_t flags,
                                       support_seg61_cc13::Seg61Host& host);
void* event_param_scalar_delete_009682f0(void* parameter, std::uint8_t flags,
                                       support_seg61_cc13::Seg61Host& host);
// __thiscall(ECX=filter, EventParam*), RET 4, EAX=0/1.
bool range_filter_matches_00968440(const RangeFilter& filter, const EventParam& parameter) noexcept;
struct KillWarningHost : WarningHost {
    // ECX=[entity+538h], AL result; the class must be a torpedo boat or small landing ship.
    virtual bool torpedo_boat_or_small_landing_ship_00827f70(void* vehicle_class) = 0;
};
// __thiscall(ECX=self, other), RET 4, AL result. Uses existing WarningObject family.
bool kill_warning_is_duplicate_0096ac60(const WarningObject& self, const WarningObject& other,
                                       KillWarningHost& host);
// No stack arguments, RET, ST0=5.0f loaded from 00CE3850.
float warning_default_lifetime_0096f230() noexcept;
// __thiscall(ECX=self, ignored other), RET 4; AL=0 (upper EAX unspecified).
bool confirm_warning_is_duplicate_00973ed0(const WarningObject& self, const WarningObject& other) noexcept;
bool string_list_warning_is_duplicate_00976290(const WarningObject& self, const WarningObject& other) noexcept;
}

namespace bsp {
// MCruiser descriptor 00D1AD38 slot +0Ch, RET, EAX=literal at 00D1AD68.
const char* cruiser_class_name_00963be0() noexcept;
// MSubmarine descriptor 00D1AE38 slot +18h; __thiscall(kind), RET 4, EAX=0/1.
bool submarine_class_is_kind_of_00963e10(std::uint32_t kind) noexcept;
}
