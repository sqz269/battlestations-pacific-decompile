#pragma once
#include "bsp/native_gui_frame_box_allocation_storage.hpp"
#include "bsp/native_gui_frame_box_identity.hpp"
#include <memory>

namespace bsp { class NativeGuiWidgetLifetimeBindings; }
namespace bsp::game {
class GameNativeReadOnlyData;

// One retained, nonmoving DEFAULT attempt. Borrows the actual Page context,
// dispatcher, canonical domains and prepared cells; owns Source metadata only.
// Caller adopts this complete backing BEFORE Native entry/callbacks and keeps
// it and the entire actual graph through normal retirement or entered failure.
class GameNativeGuiPageFrameBoxDefault final {
public:
    GameNativeGuiPageFrameBoxDefault(NativeGuiWidgetLifetimeContext& actual_page_lifetime,
        NativeGuiWidgetLifetimeBindings& actual_page_dispatcher,
        NativeRenderActualOwnerRegistry& same_registry,
        NativeStringRawPoolContext& same_raw_strings, GameNativeReadOnlyData& same_ro,
        NativeGuiFrameBoxFactoryPoolProcess& same_ready_F8C188_process,
        NativeGuiFrameBoxDefaultBindings& actual_scalar_cells);
    ~GameNativeGuiPageFrameBoxDefault() noexcept;
    GameNativeGuiPageFrameBoxDefault(const GameNativeGuiPageFrameBoxDefault&) = delete;
    GameNativeGuiPageFrameBoxDefault& operator=(const GameNativeGuiPageFrameBoxDefault&) = delete;
    GameNativeGuiPageFrameBoxDefault(GameNativeGuiPageFrameBoxDefault&&) = delete;
    GameNativeGuiPageFrameBoxDefault& operator=(GameNativeGuiPageFrameBoxDefault&&) = delete;

    // Once only: unchanged whole AA1A40(nullptr) -> AD2600. Null is normal;
    // nonnull is the actual constructor result with its same initial credit.
    void* construct_default_00aa1a40();
    // Separate one-shot Source admission after complete nonnull construction.
    // Prepare actual+F0 retirement, then bind the SAME existing reference/+04.
    // Publication escape retains the constructed Box; never ctor cleanup/free.
    NativeGuiFrameBoxReference& bind_completed_default();
    // DEFAULT domain admission only, before the caller's scalar scheduling.
    // Real base predicate; no flag choice/mask, scalar call or last-use proof.
    void require_default_before_scalar();
    // Negative metadata only. False grants no teardown/slot-return permission:
    // healthy actual credits and flags0/queued custody remain caller-held.
    bool native_operation_interrupted() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// No implicit pool/type startup, Page hook/context getter, state/property/74/78
// provider, FrameBox lineage projection, semantic widget overlay or copy path.
// No extra count/retain/native owner/bank, Native cleanup/retry, new termination
// policy, Original ABI/FH3/fault/Origin/FP theorem or production activation.
// Existing factory/reference/119 metadata destructor contracts still apply;
// the owner checks negative history BEFORE any fallback or graph teardown.
} // namespace bsp::game
