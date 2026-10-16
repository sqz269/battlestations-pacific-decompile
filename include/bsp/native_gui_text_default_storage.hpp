#pragma once
#include "bsp/native_gui_text_storage_construct.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/game_native_gui_text_state.hpp"

namespace bsp {
struct NativeGuiTextDefaultStorageContext {
    NativeGuiTextSectionStorageContext& sections;
    NativeGuiWidgetLifetimeContext& lifetime;
    game::GameNativeGuiTextState& state; // SAME already-existing mutable quartet
    const volatile std::uint32_t& shadow_offset_00d5c5c0;
    const volatile std::uint32_t& normal_rgb_00ce3e18;
    const volatile std::uint32_t& one_00d7a24c;
    const volatile std::uint32_t& disabled_alpha_00ce3800;
};
struct NativeGuiTextDefaultStorageFrame {
    // Construct and retain the COMPLETE genuine 85 graph, backing and prepared
    // recursive frames before AA9390 can invoke its first allocation callback.
    NativeGuiTextSectionStorageFrame& sections;
};
struct NativeGuiTextDefaultStorageAcquired {
    bool started{}, complete{}, base_complete{}, derived_complete{}, sections_entered{};
    bool exception_cleanup_started{}, base_lifetime_ended{};
    std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{}, consumed_cleanup_actions{};
    void* destination{}; // diagnostic only after its base lifetime ends
    NativeGuiTextSectionStorageAcquired sections;
};

// Complete 5D6CF0[42B/13ins], native ECX actual16B vector/no args/RET.
// Preserve allocator+0, capture begin+4 once, free nonnull via the SAME Source
// CRT allocation domain, then ALWAYS clear +4/+8/+C. No element destruction,
// ownership adoption or semantic std::vector. Native CRT binary ABI excluded.
void tidy_native_gui_text_pointer_vector_005d6cf0(void* actual_vector) noexcept;

// Complete normal AB9650[671B/123ins]. Native ECX raw fresh1F4 destination,
// EAX SAME destination, RET. New C++ interface, not binary/native FH3 ABI.
// AA9390(type3) completes on SAME storage before exact derived stores and
// genuine raw85 AB8530. The actual constant cells are DWORD bits, not floats.
// GameNativeGuiTextState shadow channels are separately loaded/stored as bits.
// Preserve all unwritten bytes, baseE4..EB and pool trailer+1F4.
//
// One fresh caller-held acquisition graph and disjoint initialized frame
// backing are required. Text, outer/85 frames, locals, parent frame and this
// acquisition are separate from every borrowed context/provider/global/owner.
// Transitive valid storage/current-target/provenance contracts remain those
// of the genuine providers. No process/global/pool/renderer owner is created.
//
// Ordinary C++ failure after base completion projects all observed outer
// cleanup states, including reverse strings/vector/base destruction. An 85
// provider_call marker NEVER establishes Original/native exception origin.
// Even when outer cleanup ends the widget base lifetime, independent actual
// Model/mesh/material creators, provider metadata and complete 85 frame/context
// graph remain retained by the caller. Never rediscover them from dead widget
// fields, replay the operation, call AB8250, return the pool slot or add a
// Shadow rollback. Cleanup throws terminate; successful retirement is separate.
void* construct_native_gui_text_default_storage_00ab9650(void* actual_destination,
    NativeGuiTextDefaultStorageFrame&, NativeGuiTextDefaultStorageContext&,
    NativeGuiTextDefaultStorageAcquired&);
} // namespace bsp
