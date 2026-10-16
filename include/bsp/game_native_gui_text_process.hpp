#pragma once

#include "bsp/gui_text_factory.hpp"
#include "bsp/gui_widgets_cc13.hpp"
#include <mutex>

namespace bsp::game {

// One process owns the F8BDF0 Text pool and the distinct Element/Text/Layer guard
// and descriptor cells. These are Source backing objects, not fixed-address ABI
// replacements. Construction binds pool trim dispatch but runs no native CRT
// initializer, type initializer, Text constructor, or renderer operation.
class GameNativeGuiTextProcess final {
public:
    GameNativeGuiTextProcess(const GameNativeGuiTextProcess&) = delete;
    GameNativeGuiTextProcess& operator=(const GameNativeGuiTextProcess&) = delete;

    // CD71F0: AB77C0 on the SAME pool, then register CE0A80 with std::atexit.
    // Explicit one-attempt Source policy. Cache the real registration result;
    // nonzero leaves native initialization intact without a cleanup callback.
    // An entered exception is not replayable and does not roll storage back.
    // Serialize all E188B4 list mutations across the other process owners.
    int initialize_pool_once_00cd71f0();

    // CD7210 and its AA8200 prerequisite. Borrow the canonical VFS type-counter
    // lifetime and its SAME root guard/descriptor view. A temporary bootstrap
    // over those cells guarantees that both initializers use this counter.
    // No fixed IDs, zero lineage substitute, or second root owner is supplied.
    // The counter/publications/root storage must survive this call and all
    // callbacks. The returned lineage cells remain process-owned afterward.
    // External serialization is required; entered reentry/failure is rejected
    // by Source metadata, while native guard/name/partial stores stay sticky.
    // Pool startup must already have returned, matching CE3470 then CE3474.
    void initialize_types_once_00cd7210(TypeIdCounterLifetime&,
        LightTypeBootstrapStorage canonical_common_types);

    // Explicit Source CD73F0 role, admitted only after completed pool/Text
    // startup in the SAME counter/root/root-guard domain. No normal host call,
    // second counter/root, absolute CRT timing or fixed numeric ID is supplied.
    // Keep that entire borrowed domain alive through callbacks, consumers and
    // shared shutdown; storing its identity here does not retain it. Externally
    // serialize initialization, reentry, lineage use and domain retirement.
    // Entered failure cannot replay or expose partial cells. A native guard
    // alone never establishes Source completion; unexplained nonzero guard
    // state outside the private loader-zero invariant is sticky and rejected.
    void initialize_layer_type_once_00cd73f0(TypeIdCounterLifetime&,
        LightTypeBootstrapStorage canonical_common_types);

    // Only after the corresponding complete explicit startup. Borrowers must
    // finish all Text payloads/slots and list/trim use before CE0A80 runs.
    // Do not initialize/destroy this borrowed pool through its low-level API.
    NativeGuiTextPool& pool_00f8bdf0();
    const volatile std::uint32_t (&element_lineage_00f8bc88() const)[2];
    const volatile std::uint32_t (&text_lineage_00f8be28() const)[3];
    // SAME Layer member, only after its complete explicit startup; no ID copy
    // or descriptor/default construction. Usable by the packet108 live query.
    const volatile std::uint32_t (&layer_lineage_00f8bf88() const)[3];

    // Metadata only: inspect BEFORE unwinding host/manager/CRT ownership.
    // True for an entered pool/type failure or interrupted pool retirement.
    // No lock, callback, native-storage read, recovery or retry is performed.
    // Like startup/teardown, callers must externally serialize this predicate.
    bool native_operation_interrupted() const noexcept;

private:
    friend GameNativeGuiTextProcess& game_native_gui_text_process();
    GameNativeGuiTextProcess();
    ~GameNativeGuiTextProcess() = default; // Registered CE0A80 owns pool teardown.
    static void destroy_pool_00ce0a80();
    enum class State { unattempted, returned, threw, retiring, retired };

    NativeGuiTextPoolStorage pool_storage_00f8bdf0_{};
    NativeGuiTextPool pool_;
    std::mutex pool_startup_mutex_;
    State pool_state_{State::unattempted};
    int pool_registration_status_{};

    // Original loader-zero guard/storage preimages. No public leaf exposes
    // these uninitialized words as a successful runtime type descriptor.
    std::uint8_t element_guard_00f8bc80_{};
    gui_widgets_cc13::GuiTypeDescriptor<2> element_00f8bc88_{};
    std::uint8_t text_guard_00f8bdec_{};
    gui_widgets_cc13::GuiTypeDescriptor<3> text_00f8be28_{};
    State type_state_{State::unattempted};
    TypeIdCounterLifetime* counter_{};
    const volatile RootTypeDescriptor* root_{};
    const volatile std::uint8_t* root_guard_{};

    // The sole Source Layer role, appended without moving the existing cells.
    // Zero is its loader preimage, never a successful fallback descriptor.
    std::uint8_t layer_guard_00f8bf48_{};
    gui_widgets_cc13::GuiTypeDescriptor<3> layer_00f8bf88_{};
    State layer_type_state_{State::unattempted};
};

// Construct the common E188B4 owner and this process BEFORE CD71F0 registers
// CE0A80. Reverse CRT order keeps pool storage/trim dispatch/list alive through
// that callback. All Text factories, queued uses and failed operations must be
// settled before normal CRT teardown; pool destruction destroys no Text payload.
GameNativeGuiTextProcess& game_native_gui_text_process();

} // namespace bsp::game
