#pragma once
#include "bsp/game_native_gui_sampler_application.hpp"
#include <cstddef>
#include <memory>

namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameVfsHost;
class GameSingletonHost;
class GameNativeReadOnlyData;

// Retained owner of ONE unpublished private DEFAULT Text lifecycle, with an
// explicit optional primary-prefix Source path. No raw Text
// pointer/reference/credit is exposed. Source composition, not an AA1380 ABI,
// child factory, layout/content adapter or renderer/frontend activation.
class GameNativeRawDefaultTextOwner final {
public:
    GameNativeRawDefaultTextOwner(GameHostLog&, GameNativeRendererApplication&,
        GameVfsHost&, GameSingletonHost&, GameNativeReadOnlyData&,
        GameNativeGuiSamplerApplicationBindings);
    ~GameNativeRawDefaultTextOwner() noexcept;
    GameNativeRawDefaultTextOwner(const GameNativeRawDefaultTextOwner&) = delete;
    GameNativeRawDefaultTextOwner& operator=(const GameNativeRawDefaultTextOwner&) = delete;

    // Install this owner before calling. Requires completed canonical Text type
    // startup and the ready/open renderer scene domain, BEFORE mission/cache
    // entry. Allocates only C++ metadata; exports retained sampler bindings.
    // A partial export/failure poisons the owner; no cleanup/retry is inferred.
    // Capacity is an explicit finite prepared call budget, never recycled.
    void prepare_metadata_in_ready_renderer(std::size_t model_call_capacity);
    // Separate uncalled Source fixture: install an immutable primary-prefix
    // graph before factory effects. Existing preparation remains DEFAULT.
    // No Model authority is granted until the real producer issues its receipt.
    void prepare_primary_prefix_metadata_in_ready_renderer(std::size_t model_call_capacity);
    // Caller owns the REAL later canonical cache bracket. SAME Lua Activation
    // covers ba2 AA1380 and SEPARATE raw122 publication. False is native null
    // allocation, not a failed constructor. No private cache/warm substitute.
    bool construct_default_in_open_shader_cache();
    // Same real AA1380/raw93/separate122 first, then genuine raw pooled input
    // name, primary producer prefix and name return. No74/properties/loaded,
    // content, Font, parenting, rendering or whole-AAA710 completion is supplied.
    bool construct_primary_prefix_in_open_shader_cache();
    // Only this sole unpublished Text, actual count1, quiescent providers and
    // the genuine canonical direct scalar path are admitted. Retained flags1
    // cell is reread late by AB8EE0; no Text decrement/second cleanup is added.
    void retire_before_native_drain();
    // Metadata only, no native payload access. Failed/active calls and pending
    // Model credits/flags0 physical custody retain the WHOLE owner/providers.
    bool requires_process_retention() const noexcept;
    // Real renderer drain verification FIRST, then effect/sampler closure.
    // Retire Text/settle native custody before the parent drain; retain all
    // borrowed services/input/type/data/string/owner domains until this returns.
    void after_native_renderer_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
