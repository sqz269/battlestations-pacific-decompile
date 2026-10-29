#pragma once
#include "bsp/game_native_gui_sampler_application.hpp"
#include <cstddef>
#include <memory>

namespace bsp { class NativeFontResourceOwners; }
namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameVfsHost;
class GameSingletonHost;
class GameNativeReadOnlyData;
class GameNativeFontApplication;

// Retained owner of ONE unpublished private DEFAULT Text lifecycle, with an
// explicit optional primary-prefix and private geometry Source paths. No raw Text
// pointer/reference/credit is exposed. Source composition, not an AA1380 ABI,
// child factory, layout/content adapter or renderer/frontend activation.
class GameNativeRawDefaultTextOwner final {
public:
    // Closed C++ choices, fixed before preparation. No public environment or
    // activation switch. Every choice first sets Viper19 and runs real AB8CE0.
    enum class FontWork { shader_then_retirement, invalidate_shader, equal_name };
    GameNativeRawDefaultTextOwner(GameHostLog&, GameNativeRendererApplication&,
        GameVfsHost&, GameSingletonHost&, GameNativeReadOnlyData&,
        GameNativeGuiSamplerApplicationBindings);
    ~GameNativeRawDefaultTextOwner() noexcept;
    GameNativeRawDefaultTextOwner(const GameNativeRawDefaultTextOwner&) = delete;
    GameNativeRawDefaultTextOwner& operator=(const GameNativeRawDefaultTextOwner&) = delete;

    // Combined Font Host/App metadata handshake only. Begin on this fresh,
    // stably installed owner BEFORE the producer's irreversible export;
    // complete with its SAME ready actual collection/image before preparation.
    // No Font lookup/retain, native string, shader or pool effect is performed.
    void begin_raw_font_link(const GameNativeFontApplication& producer,
        GameNativeRendererApplication&, GameVfsHost&);
    void complete_raw_font_link(const GameNativeFontApplication& producer,
        NativeFontResourceOwners&, const void* admitted_registry_image);
    // Latches even an admitted producer rejection before begin. Never replaces
    // a different existing producer or permits cancellation/rebind/replay.
    void fail_raw_font_link(const GameNativeFontApplication& producer) noexcept;
    // Called directly by the SAME producer BEFORE native Font retirement.
    // Requires terminal retired owner and all actual custody/invocations;
    // !requires_process_retention() while bound grants no permission.
    void require_font_retirement(const GameNativeFontApplication& producer,
        const NativeFontResourceOwners&, const void* admitted_registry_image) const;
    // Explicit BOTH-layer unlink AFTER after_native_renderer_drain and BEFORE
    // resetting this owner. Saved image is identity-only after Font retirement;
    // this operation never follows retired Font/header/list/texture payloads.
    void detach_raw_font_link_after_parent_drain(const GameNativeFontApplication& producer,
        const NativeFontResourceOwners&, const void* admitted_registry_image);

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
    // Separate immutable geometry-only graph, installed before factory/raw122.
    // Constructor Text+74 must remain zero through the closed real-provider
    // operation; no outer guard, flag repair or element fallback supplies that.
    void prepare_primary_prefix_geometry_metadata_in_ready_renderer(std::size_t model_call_capacity);
    // Separate closed geometry+created-main-section graph. Install its exact
    // lifetime/frame/cache backing before pools/raw122; this first domain has
    // no FontWork or Font export requirement. No native effects during prepare.
    void prepare_primary_prefix_geometry_main_section_metadata_in_ready_renderer(
        std::size_t model_call_capacity);
    // Requires the completed actual Font Host/App link. Installs ALL input,
    // selector/shader/lifetime backing before pools/raw122; no native Font work.
    // Optional second distinct invocation uses Arial18 to invalidate or Viper19
    // for the real equal-name path. Existing preparation modes remain separate.
    void prepare_primary_prefix_geometry_font_work_in_ready_renderer(
        std::size_t model_call_capacity, FontWork);
    // Separate closed Font-selection then created-main-section fragment graph.
    // Requires the SAME completed actual Font Host/App link. Installs every
    // fixed input/selector/shader/helper backing before pools/raw122; no native
    // Font or shader effects during preparation. Existing modes stay separate.
    void prepare_primary_prefix_geometry_font_main_section_in_ready_renderer(
        std::size_t model_call_capacity);
    // Caller owns the REAL later canonical cache bracket. SAME Lua Activation
    // covers ba2 AA1380 and SEPARATE raw122 publication. False is native null
    // allocation, not a failed constructor. No private cache/warm substitute.
    bool construct_default_in_open_shader_cache();
    // Same real AA1380/raw93/separate122 first, then genuine raw pooled input
    // name, primary producer prefix and name return. No74/properties/loaded,
    // content, Font, parenting, rendering or whole-AAA710 completion is supplied.
    bool construct_primary_prefix_in_open_shader_cache();
    // Same genuine prefix/name sequence, then exactly one existing379/AB7700
    // body. This is explicit private entry, not AAA710/current74 scheduling.
    // No main-section, Font/content, parenting, rendering or host gate is added.
    bool construct_primary_prefix_geometry_in_open_shader_cache();
    // Same factory93/separate122/primary/name/ONE379, then the genuine later
    // AB8530 main arm and its private created-normal witness/lifetime seal.
    // No Font/content/layout/stream/current74 dispatch or renderer activation.
    bool construct_primary_prefix_geometry_main_section_in_open_shader_cache();
    // Same genuine primary+geometry, then seal helper, construct/return genuine
    // input headers around raw setter calls and run the real shader producer.
    // Shader caller credit is consumed by second setter OR later raw118 once.
    bool construct_primary_prefix_geometry_font_work_in_open_shader_cache();
    // Same factory93/separate122/primary/name/ONE379, then real Viper19 Font
    // selection/input return BEFORE real AB8530 Main creation and lifetime seal.
    // No AB8CE0 producer, content, properties, rendering or full ABA8D0 claim.
    bool construct_primary_prefix_geometry_font_main_section_in_open_shader_cache();
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
