#pragma once
#include "bsp/ship_ai_goal_vector.hpp"
#include "bsp/gameplay_settings.hpp"
#include "bsp/native_particle_unit_random.hpp"

namespace bsp {
// Borrow canonical cells; this view neither allocates a brain nor initializes
// padding, omitted subobjects or previous ownership. Cells must not alias.
struct ShipAiBrainProducerView {
    ShipAiGoalVectorState& goal;
    float& period_b3c;
    float& countdown_b40;
    float& torpedo_period_b44;
    float& torpedo_countdown_b48;
    float& ship_period_b4c;
    float& ship_countdown_b50;
    TrackedCriticalSection*& lock_04;
};

struct ShipAiBrainProducerContext {
    // Existing registered/fallback thread domain, including its live constants.
    const NativeParticleUnitRandomAccess& random;
    const volatile float& one_00d7a24c;
    const volatile float& two_00ce3958;
    const volatile float* zero_vector_00f87574; // three actual adjacent cells
    void* settings_context;
    const GameplayTuningSettings& (*settings_00424c40)(void*);
};

// PARTIAL 009F1160: complete normal producer tail009F126B..009F1401,
// excluding the private SEH-state writes/restoration loads. New C++ ABI.
// Must follow the original navigation/owner/helper/inline-target construction;
// the navigation constructor's earlier random draw must already have occurred.
// Executes seven actual stream1 range draws, four fresh settings returns, and
// actual raw1Ch critical-section construction last. Does not seed/register RNG.
// Existing lock_04 is not read/released: use fresh constructor ownership only.
// Null allocation is published; a thrown allocation/OS init leaves preceding
// stores/draws intact and lock_04 untouched. No parent SEH rollback is supplied.
// Release the produced section with release_native_tracked_critical_section_
// 0041cc80 on its owning/quiescent thread, not the older new/delete interface.
void ship_ai_brain_produce_tail_009f126b(ShipAiBrainProducerView,
    const ShipAiBrainProducerContext&);
} // namespace bsp
