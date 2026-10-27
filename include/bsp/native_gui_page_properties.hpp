#pragma once
#include "bsp/native_gui_scene_acquisition_storage.hpp"
#include "bsp/native_gui_widget_property_children.hpp"
#include "bsp/native_gui_widget_position.hpp"

namespace bsp {
namespace game { class GameNativeReadOnlyData; }
struct NativeGuiPageReadPropertiesContext {
    game::GameNativeReadOnlyData& data; // SAME genuine mapped Original tables/literals
    NativeGuiSceneAcquisitionContext& scene;
    NativeGuiWidgetPropertyInteractionContext& widget;
    NativeGuiWidgetPropertyChildrenContext& children;
    const volatile double& half_00d7a280;
};
struct NativeGuiPageReadPropertiesFrame {
    // Initialized, caller-retained native preimage. S is ESP after AC4C54:
    // bytes represent [S-18,S+18), with pairs0/8/10, unwritten saved-register
    // gap18..1F, Priority20 and later xyz24/28/2C. No NativeString is here.
    alignas(4) std::byte bytes[0x30];
    const NativeGuiSceneAcquisitionFrame& scene;
    NativeGuiWidgetPropertyScratch& widget;
    GuiWidgetBounds& bounds;
};
struct NativeGuiPageReadPropertiesAcquired {
    enum class Phase { fresh, entered, complete, failed };
    Phase phase{Phase::fresh};
    std::uint32_t active_call_site{}, loaded_profile{}, loaded_target{};
    bool priority_published{}, render_order_read{}, scene_returned{};
    bool root_returned{}, position_returned{}, properties_returned{};
    void* requested_scene{};
    void* root_node{};
    NativeGuiSceneAcquisitionAcquired scene;
    NativeGuiWidgetPropertyChildrenAcquired children;
    NativeGuiPageReadPropertiesAcquired() = default;
    NativeGuiPageReadPropertiesAcquired(const NativeGuiPageReadPropertiesAcquired&) = delete;
    NativeGuiPageReadPropertiesAcquired& operator=(const NativeGuiPageReadPropertiesAcquired&) = delete;
    ~NativeGuiPageReadPropertiesAcquired() noexcept;
};
// Whole AC4C50 normal source schedule over the SAME actual124h Page/reader.
// Real current78 AC59A0 composes the existing raw scene/camera/store services;
// no other target receives a fallback. They require a warm genuine manager in
// the SAME platform/focus context and reread its actual publication natively.
// This helper never constructs manager A or another Page/tree/owner.
//
// Retain this complete graph/frame/acquisitions and all genuine derived factory
// providers through native Page retirement. Serialize calls/lifetimes and use
// distinct, initialized/disjoint scratch for reentry. Native callbacks may
// alter reached fields; later loads stay current. Entered failure retains all
// reached effects/credits/metadata, forbids replay and dependency drain. The
// caller must arrange process disposition if the graph cannot retire normally.
// Original __thiscall ECX Page / stack reader / RET4 is NOT this source ABI;
// FH3/SEH, private-stack coincidence and binary/game equivalence are unproved.
void read_native_gui_page_properties_00ac4c50(void* actual_page, void* actual_reader,
    NativeGuiPageReadPropertiesFrame&, NativeGuiPageReadPropertiesContext&,
    NativeGuiPageReadPropertiesAcquired&);
} // namespace bsp
