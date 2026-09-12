#pragma once

#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/point_effect_children.hpp"

namespace bsp {

// Complete0086B100..0086B1B3. ECX actual24h definition; stack native8h name,
// component output slot, index output; AL boolean; RET0C. Captures row extent,
// compares actual component+08 length before CRT case-insensitive string data.
// Zero lengths match without reading either data pointer. Failure leaves BOTH
// outputs untouched. Success publishes/retains selected actual component,
// releases captured previous output, THEN writes index. Identity skips counts.
// Components are actual owners (Tracer's secondary component base is valid),
// and lifetime dispatch uses their sole actual+04. No companion count is made.
bool find_gameplay_effect_component_0086b100(GameplayEffectDefinition&,
    const void* actual_name_header, void*& actual_component_output,
    std::int32_t& index_output, GameplayEffectComponentLifetime&);
bool find_gameplay_effect_component_0086b100(GameplayEffectDefinition&,
    const NativeString&, void*& actual_component_output,
    std::int32_t& index_output, GameplayEffectComponentLifetime&);

class PointEffectLookupDefinitions {
public:
    virtual ~PointEffectLookupDefinitions() = default;
    // Pure nonthrowing identity projection of the exact current +84 companion
    // to its already-constructed actual definition. Never synthesize an owner,
    // copy its array, retain it, or start/reset any actual reference count.
    virtual GameplayEffectDefinition& definition(RenderCommandReference&) noexcept = 0;
};

struct PointEffectNameLookupBindings {
    PointEffectLookupDefinitions& definitions;
    GameplayEffectComponentLifetime& components;
    PointEffectRowRuntime& rows;
    EffectManager* volatile& global_00f87650;
    EffectManagerLifetimeAccess& lifetime;
};

// Complete00866CD0..00866E5F. ECX actual point; stack output slot, native name;
// EAX output slot; RET8. Calls canonical866440 and captures its actual+04 lock.
// Looks up current+84; creates via current row virtual18 only when the indexed
// child is null (no autostart/admission gate); reloads backing after factory and
// captured temporary release. Output is CONSTRUCTED: never releases its prior
// contents. Clears output BEFORE loading the selected current entry, preserving
// output/source alias behavior. Retains output before component release/unlock.
// A factory exception releases component before unlock; a normal component
// cleanup exception unlocks then releases/clears the already-constructed output.
// Known native owner/index/span preconditions are required. New C++ ABI only;
// borrowed event companions keep their existing nonthrowing terminal contract.
RenderCommandReference** find_or_create_point_effect_child_00866cd0(
    PointEffectInstanceStorage&, RenderCommandReference** output,
    const void* actual_name_header, PointEffectNameLookupBindings);
RenderCommandReference** find_or_create_point_effect_child_00866cd0(
    PointEffectInstanceStorage&, RenderCommandReference** output,
    const NativeString&, PointEffectNameLookupBindings);

// Complete00866E60..00866F41. ECX point; stack output slot,type DWORD; RET8,
// EAX output slot. Canonical manager lock; capture signed count/backing ONCE;
// first nonnull child with exact type+18 wins. Capture selected pointer BEFORE
// clearing output (unlike866CD0); retain it without releasing prior output.
// A nonpositive count returns null. No factories, active-byte or option gate.
RenderCommandReference** find_point_effect_child_by_type_00866e60(
    PointEffectInstanceStorage&, RenderCommandReference** output, std::uint32_t type,
    PointEffectChildEvents&, EffectManager* volatile& global_00f87650,
    EffectManagerLifetimeAccess&);

} // namespace bsp
