#include "bsp/game_native_shadow_target_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_renderer_scalars.hpp"
#include "bsp/game_native_settings_process.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

namespace bsp::game {
namespace {
using Stage = GameNativeShadowTargetObservation::Stage;
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
}
struct GameNativeShadowTargetApplication::Impl {
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameNativeSettingsProcess& settings;
    NativeShadowDepthTargetContext& context;
    void* volatile& publication;
    GameNativeShadowTargetObservation observed;

    Impl(GameHostLog& l, GameNativeRendererApplication& r, GameNativeSettingsProcess& s)
        : log(l), renderer(r), settings(s), context(r.shadow_depth_target_context()),
          publication(game_native_renderer_scalar_process().shadow_target_publication_00f8bbf0()) {
        require(&context.bindings.publication_00f8bbf0 == &publication,
            "shadow parent requires the canonical renderer F8BBF0 publication");
        require(!publication, "shadow parent requires an unpopulated canonical target cell");
        require(!renderer.requires_process_retention(),
            "shadow parent requires completed renderer acquisitions");
        // No settings capture, allocation, publication or native call here.
    }
};

GameNativeShadowTargetApplication::GameNativeShadowTargetApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameNativeSettingsProcess& settings)
    : impl_(std::make_unique<Impl>(log, renderer, settings)) {}

GameNativeShadowTargetApplication::~GameNativeShadowTargetApplication() noexcept {
    const auto stage = impl_->observed.stage;
    if (stage != Stage::prepared && stage != Stage::closed) std::terminate();
    // The native registered owner is never deleted by host metadata destruction.
}

void GameNativeShadowTargetApplication::construct() {
    auto& p = *impl_;
    auto& o = p.observed;
    require(o.stage == Stage::prepared, "shadow parent construction cannot be replayed");
    require(!p.publication, "shadow parent cannot replace an existing canonical target");
    require(!p.renderer.requires_process_retention(),
        "shadow parent cannot enter an interrupted renderer");
    try {
        // Caller qualification: 73DCCB reads F8899D once; NEG/SBB/AND800/ADD800
        // captures ESI before BF681B(2Ch), then 73DD04 pushes that extent twice.
        // settings() is the canonical static BCh raw owner, not its read-view copy.
        const auto* bytes = reinterpret_cast<const volatile std::uint8_t*>(p.settings.settings().data());
        o.hires_shadow = bytes[0x1d];
        const std::uint32_t extent = o.hires_shadow ? 4096u : 2048u;
        o.dimensions = {extent, extent};
        o.stage = Stage::settings_captured;
        o.stage = Stage::allocating;
        o.allocated = singleton_lifetime_allocate({SingletonAllocationKind::object,
            0x2c, sizeof(NativeShadowDepthTargetStorage)});
        o.stage = Stage::allocation_captured;
        // Existing BF681B Source allocation throws on exhaustion, but preserve
        // the caller's guarded null-return arm if that boundary ever returns null.
        if (o.allocated) {
            o.constructor_operation_index = p.context.operation_count();
            o.constructor_entered = true;
            o.stage = Stage::constructing;
            o.returned = construct_native_shadow_depth_target_00a8fe30(
                o.allocated, o.dimensions, p.context);
            require(o.returned == o.allocated && p.publication == o.allocated,
                "shadow constructor must publish its actual allocated owner");
            require(o.returned->profile_00 == 0x00d5b5e8 &&
                o.returned->width_04 == extent && o.returned->height_08 == extent,
                "shadow constructor must complete the genuine final owner");
        }
        o.stage = Stage::ready;
        p.log.notef("native shadow target: owner=%p extent=%u hires=%u constructor=%u",
            o.returned, extent, static_cast<unsigned>(o.hires_shadow),
            static_cast<unsigned>(o.constructor_entered));
    } catch (...) {
        o.failure = std::current_exception();
        o.failure_stage = o.stage;
        o.stage = Stage::failed;
        // Original entered locks/publication/storage and actual child frames
        // remain owned by the retained graph. No cleanup/retry/fallback occurs.
        throw;
    }
}

const GameNativeShadowTargetObservation&
GameNativeShadowTargetApplication::observation() const noexcept { return impl_->observed; }

const NativeShadowDepthTargetOperation*
GameNativeShadowTargetApplication::constructor_operation() const noexcept {
    const auto& p = *impl_;
    return p.observed.constructor_entered ?
        p.context.operation(p.observed.constructor_operation_index) : nullptr;
}

NativeShadowDepthTargetStorage* GameNativeShadowTargetApplication::target() const {
    const auto& p = *impl_;
    require(p.observed.stage == Stage::ready, "shadow target borrow requires completed construction");
    require(p.publication == p.observed.returned,
        "shadow target borrow requires the original still-published owner");
    return p.observed.returned;
}

bool GameNativeShadowTargetApplication::requires_process_retention() const noexcept {
    const auto stage = impl_->observed.stage;
    return (stage != Stage::prepared && stage != Stage::ready && stage != Stage::closed)
        || impl_->renderer.requires_process_retention();
}

void GameNativeShadowTargetApplication::after_native_renderer_drain() {
    auto& p = *impl_;
    if (p.observed.stage == Stage::closed) return;
    require(p.observed.stage == Stage::prepared || p.observed.stage == Stage::ready,
        "incomplete shadow parent requires process retention");
    require(!p.publication, "canonical shadow target survived native drain");
    p.renderer.after_native_drain();
    p.observed.stage = Stage::closed;
    // allocated/returned remain observation addresses; never dereference after drain.
}
} // namespace bsp::game
