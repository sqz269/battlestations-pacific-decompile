#pragma once
#include "bsp/game_native_gui_page_application.hpp"
#include "bsp/native_texture_surface_getter.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>

namespace bsp {
struct GuiWidgetBounds;
struct GuiTextureCallbacks;
struct GuiIconStateTextureConstants;
struct NativeGuiWidgetTransformScratch;
struct NativeSingletonDeletionBindings;
struct NativePlatformFocusOwnersContext;
struct NativeParticleTextureNamesRawContext;
struct NativeMaterialEffectCacheContext;
struct NativeMaterialDestructionAccess;
struct NativeMaterialParameterAccess;
struct NativeGuiWidgetClipBindings;
struct NativeGuiIconDelayedDeliveryStorageContext;
class NativePlatformLoadPublication;
class NativeGuiIconDelayedLoads;
class NativeCrtStricmpCallees;
}

namespace bsp::game {
class GameNativeCanonicalDataOwner;
class GameNativeRendererApplication;
class GameNativeVfsRuntime;
class GameNativeGuiTextProcess;
class GameNativeGuiTextState;

// Mandatory initialized PRIVATE Source inputs. These are not default Native
// seeds, recovered Original stack words, capacity/Origin or lifetime proofs.
// State/find arrays remain CURRENT borrowed references. Each fresh74/raw80
// copies the initialized transform/bounds/key input it actually needs during
// cold preparation, preserving all transform holes. The raw key extent is
// physical backing, never a four-descriptor cap or an early count snapshot.
struct GameNativeGuiPageIconPrivateInputs {
    const volatile GameNativeGuiPageIconServices::StatePrivatePreimages& state;
    const volatile std::uint32_t (&find_guard)[2];
    const NativeGuiWidgetTransformScratch& initialized_transform;
    const GuiWidgetBounds& initialized_bounds;
    const volatile std::uint32_t* initialized_layout_key_words;
    std::size_t initialized_layout_key_word_extent;
};

// Genuine SAME process/renderer/Page domains, not newly constructed owners.
// The Root supplies the real CRT errno/invalid-parameter/current-locale family;
// this application supplies no ASCII-only, guessed errno or locale fallback.
// Retain every input and callback target through entered failures, Native
// retirement, actual pending/queued/worker last use and shared native drain.
struct GameNativeGuiPageIconServicesInputs {
    const GameNativeCanonicalDataOwner& canonical_v3;
    GameNativeRendererApplication& renderer;
    NativeStringRawPoolContext& strings;
    GameNativeVfsRuntime& runtime;
    NativeGuiIconDelayedLoads& delayed;
    const NativeSingletonDeletionBindings& singleton_deletion;
    NativePlatformFocusOwnersContext& gui;
    GameNativeGuiTextProcess& widget_types;
    GameNativeGuiTextState& widget_state;
    NativeParticleTextureNamesRawContext& atlas_names;
    NativeMaterialEffectCacheContext& effects;
    NativeMaterialDestructionAccess& material_lifetime;
    NativeMaterialParameterAccess& material_parameters;
    NativeGuiWidgetClipBindings& clip;
    const NativePlatformLoadPublication& platform;
    const GuiTextureCallbacks& texture_dimensions;
    const GuiIconStateTextureConstants& texture_constants;
    NativeTextureSurfaceReferenceIncrement const volatile& increment_00ce221c;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
    NativeCrtStricmpCallees& stricmp_callees;
    GameNativeGuiPageIconPrivateInputs private_inputs;
};

// Concrete borrower for the existing raw Page Type6 interface. It creates no
// Native Icon/Model/pool/cache/owner bank, registry entry or protective retain.
// Source records are nonmoving, unbounded history, not Native ownership.
// Cold bind/preparation performs no Native payload read/call or HOME seeding.
// Page retains each returned view BEFORE calling the existing whole body;
// this application retains the referenced frame/context/backing independently.
// Destroy only after genuine caller-proven last use; retain the whole graph on
// unknown escape. Normal observations and false retention are not permission.
class GameNativeGuiPageIconServicesApplication final : public GameNativeGuiPageIconServices {
public:
    explicit GameNativeGuiPageIconServicesApplication(const GameNativeGuiPageIconServicesInputs&);
    ~GameNativeGuiPageIconServicesApplication() noexcept override;
    GameNativeGuiPageIconServicesApplication(const GameNativeGuiPageIconServicesApplication&) = delete;
    GameNativeGuiPageIconServicesApplication& operator=(const GameNativeGuiPageIconServicesApplication&) = delete;

    NativeGuiIconStorageContext& bind_page_storage(NativeGuiWidgetLifetimeContext&,
        NativeRenderActualOwnerRegistry&, NativeGuiWidgetTransformBindings&,
        NativeGuiWidgetLoadedDispatch&) override;
    const NativeGuiIconPropertyStateServices& property_state_services() override;
    const volatile StatePrivatePreimages& property_private_preimages() override;
    std::unique_ptr<GameNativeGuiPageIconConstructedInvocation>
        prepare_constructed_74(NativeGuiIconReference&) override;
    std::unique_ptr<GameNativeGuiPageIconLoadedInvocation>
        prepare_loaded_78(NativeGuiIconReference&) override;
    bool class_query_00aa1cc0(NativeGuiIconReference&, std::uint32_t descriptor) override;
    void visibility_changed_00ab6120(NativeGuiIconReference&, std::uint8_t visible) override;

    // Address-only borrow after successful Page bind. Root separately performs
    // Runtime142's cold once-bind before any FileStore request. No callback,
    // native singleton getter, request/removal or owner authority occurs here.
    NativeGuiIconDelayedDeliveryStorageContext& completion_delivery_context();
    bool requires_process_retention() const noexcept;
    std::uint32_t failure_site() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
