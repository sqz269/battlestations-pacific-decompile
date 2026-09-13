#pragma once

#include "bsp/native_unit_observer_endpoint.hpp"
#include "bsp/recon_slot_lists.hpp"

#include <array>
#include <cstdint>

namespace bsp {

// Borrow actual fields of one existing detection record. Its established layout
// is unit + kReconUnitDetectionArrayOffset + index*kReconUnitDetectionStride.
// These references introduce no replacement record or native-layout cast.
// Preserve signed values and every nonzero force byte; do not normalize enums.
struct NativeUnitKilledDetectionView {
    const std::int32_t& level_04;
    const std::int32_t& forced_level_08;
    const std::uint8_t& force_10;
};

struct NativeUnitKilledTailView {
    NativeUnitObserverAlias unit;
    std::array<NativeUnitKilledDetectionView, kReconUnitDetectionCount> detection;
};

// Required actual providers. The accessor resolves the current world publication
// at the time called, with no side effects or copied world state. All borrowed
// owners must remain live until the final handler completes. No defaults exist.
struct NativeUnitKilledTailHost {
    virtual ~NativeUnitKilledTailHost() = default;

    // 00779AFA / 00779B0D: reload and dispatch primary +18h on this same unit.
    // The second call is conditional; neither result nor the dispatch is cached.
    virtual void* query_primary_18(void* canonical_unit) = 0;

    // 00779B15: 004BCA80, ECX=0. Publish the argument into actual E188DC,
    // then run 00B0D7B0 on current F8D39C, including its listener update.
    virtual void publish_controlled_listener_004bca80(void* handle) = 0;

    // 00779B1A..25: borrow [current E188A8]+193Ch after the optional call.
    virtual std::uint8_t& current_world_unit_lists_built_193c() = 0;

    // 00779B43: 00803BA0, ECX=index. Mark the present actual recon slot +25h,
    // then apply its current-player-context test and possible world+193Ch clear.
    virtual void mark_recon_slot_dirty_00803ba0(std::int32_t index) = 0;

    // 00779B58 JMP 00928C80, ECX=the original unit. Requires the COMPLETE base
    // Lua/log/detach handler. mission_entity_on_killed_00928c80 is only partial
    // and cannot supply this contract alone. No work follows this delegation.
    virtual void on_killed_00928c80(void* canonical_unit) = 0;
};

// Complete normal body 00779AF0..00779B5C (109 bytes), native ECX=unit, no stack
// arguments, final JMP to 00928C80. Borrow the actual E188DC publication cell.
// Capture it AFTER the first nonnull query and BEFORE the second query; later
// publication changes do not change that comparison. Read each detection record
// in order, after earlier callbacks, and mark only a signed effective level >0.
// New source ABI; no native exception/FH3/SEH, scene binding or gameplay claim.
void native_unit_on_killed_00779af0(const NativeUnitKilledTailView&,
    void* const& controlled_listener_00e188dc, NativeUnitKilledTailHost&);

} // namespace bsp
