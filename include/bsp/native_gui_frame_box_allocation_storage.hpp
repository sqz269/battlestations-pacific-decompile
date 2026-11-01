#pragma once
#include "bsp/native_gui_frame_box_copy_storage.hpp"
#include "bsp/native_gui_frame_box_factory.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
namespace game { template<class Traits> class GameNativeGuiWidgetPoolProcess; }
using NativeGuiFrameBoxFactoryPoolProcess = game::GameNativeGuiWidgetPoolProcess<GuiFrameBoxPoolTraits>;

// Required independently retained Source backing for ONE late138 lifetime.
// It owns no raw graph. Construct before allocator callbacks, outside the
// parent's failing scope; no138 reference is formed before placement returns.
// Its destructor invokes only138 metadata destruction, which terminates for
// entered/failed138. Thus failed138/111/creator graphs and this backing must
// outlive factory metadata when their genuine contracts require retention.
// No reset, retry, implicit tidy/free or child-failure abandonment is supplied.
class NativeGuiFrameBoxFactoryCopyStorage final {
public:
    NativeGuiFrameBoxFactoryCopyStorage() noexcept = default;
    ~NativeGuiFrameBoxFactoryCopyStorage() noexcept;
    NativeGuiFrameBoxFactoryCopyStorage(const NativeGuiFrameBoxFactoryCopyStorage&) = delete;
    NativeGuiFrameBoxFactoryCopyStorage& operator=(const NativeGuiFrameBoxFactoryCopyStorage&) = delete;
    NativeGuiFrameBoxFactoryCopyStorage(NativeGuiFrameBoxFactoryCopyStorage&&) = delete;
    NativeGuiFrameBoxFactoryCopyStorage& operator=(NativeGuiFrameBoxFactoryCopyStorage&&) = delete;
    bool attempted() const noexcept { return attempted_; }
    const NativeGuiFrameBoxVectorCopyOperation* vector_operation() const noexcept;
private:
    friend class NativeGuiFrameBoxFactoryOperation;
    bool attempted_{}, vector_constructed_{};
    std::uint8_t padding_[2]{};
    alignas(NativeGuiFrameBoxVectorCopyOperation)
        std::byte vector_storage_[sizeof(NativeGuiFrameBoxVectorCopyOperation)];
};

enum class NativeGuiFrameBoxFactoryPhase : std::uint8_t {
    fresh, allocating, allocated, preparing_copy, constructing_copy,
    constructing_default, returned, returned_null, failed_before_slot,
    failed_slot_returned, failed_retained
};

// Whole AA1A40[AA1A40,AA1AC4),132B/40starts: Native ECX optional source,
// EAX constructor result or0, RET. This ordinary explicit Source API is NOT
// Native register/stack/FH3/SEH/placement or heap identity.
//
// The process must be the genuine completed SAME F8C188 authority over E188B4;
// no startup is performed here. Prehold the process/slabs/table/slot authority,
// actual scalar cells or whole copy graph, all providers/frames/arrays and
// independent aligned backing BEFORE allocator callbacks. Known metadata
// regions validate before member stores; full transitive backing/scratch,
// actual new-slot disjointness and valid C++ lifetimes remain caller proofs.
// Access is externally serialized; callbacks cannot alter operation/backing/
// prepared111 metadata, reset/reenter it, retire an active Box or reuse a slot.
//
// Default preparation requires only real96 scalar bindings. Copy preparation
// requires exact nonnull actual source in184's existing supported domain,
// real184 services/frames, and DISTINCT stable fresh111 rollback/destruction
// arrays. No raw source header/payload snapshot or source retain is added.
// Late138 is external; late184 is held by THIS persistent operation. Normal
// copy success must keep it alive through SAME163 companion/125 retirement.
// Returned default does not manufacture a Model, hook, state dispatch or163;
// any explicit post-return public163 admission is a separate Source boundary.
//
// Native states0(copy)/1(default) return only the captured slot after applicable
// genuine callee unwind. We consume the action BEFORE SAME pool.free once,
// then mark failed_slot_returned using metadata only. No count/material gate,
// retain, repeated destructor, extra default unwind or graph recovery exists.
// Added preparation errors before constructor entry are Source boundaries;
// invalid allocated-slot region checks retain rather than claim Native cleanup.
// Ordinary entered-copy escape is admitted for slot return ONLY after184's
// failed_unwind_returned; other interrupted state stays failed_retained.
//
// Native slot return can orphan names/creators/material Box credits. External
// failed138/111/166/acquisitions/providers/backing OUTLIVE this metadata as
// required. After return, old Box count/125/zero/material-release/cleanup or
// slot reuse authority is not recovered from diagnostic addresses. No private
// EH/SEH/double-exception equivalence or leak-free guarantee is claimed.
// Allocator escape has no acquired slot and does not repair its pool graph.
// Phase/action/entry flags describe SOURCE scheduling, not a private Native
// exception origin; an entered Source184 interface may reject while still fresh.
//
// Destruction is metadata only. Active/failed_retained/free-interrupted phases
// terminate; late184 delegates to its unchanged metadata-only destructor,
// including its live-companion guard. External late138 is never destroyed by
// this operation. No reset/retry or general abandonment operation is exposed.
class NativeGuiFrameBoxFactoryOperation final {
public:
    NativeGuiFrameBoxFactoryOperation(NativeGuiFrameBoxFactoryPoolProcess&,
        NativeGuiFrameBoxDefaultBindings&);
    NativeGuiFrameBoxFactoryOperation(NativeGuiFrameBoxFactoryPoolProcess&,
        const void* actual_source, NativeGuiFrameBoxCopyStorageServices&,
        NativeGuiFrameBoxCopyStorageFrames&, NativeGuiFrameBoxFactoryCopyStorage&,
        NativeGuiFrameBoxRecordReleaseOperation* fresh_rollback_records, std::size_t rollback_capacity,
        NativeGuiFrameBoxRecordReleaseOperation* fresh_destruction_records, std::size_t destruction_capacity);
    ~NativeGuiFrameBoxFactoryOperation() noexcept;
    NativeGuiFrameBoxFactoryOperation(const NativeGuiFrameBoxFactoryOperation&) = delete;
    NativeGuiFrameBoxFactoryOperation& operator=(const NativeGuiFrameBoxFactoryOperation&) = delete;
    NativeGuiFrameBoxFactoryOperation(NativeGuiFrameBoxFactoryOperation&&) = delete;
    NativeGuiFrameBoxFactoryOperation& operator=(NativeGuiFrameBoxFactoryOperation&&) = delete;

    NativeGuiFrameBoxFactoryPhase phase() const noexcept { return phase_; }
    const void* source_identity() const noexcept { return source_; }
    void* slot_identity() const noexcept { return slot_; }
    void* returned_identity() const noexcept { return result_; }
    std::int32_t scheduled_slot_action() const noexcept { return slot_action_; }
    bool constructor_entered() const noexcept { return constructor_entered_; }
    bool pool_return_entered() const noexcept { return pool_return_entered_; }
    bool pool_return_returned() const noexcept { return pool_return_returned_; }
    NativeGuiFrameBoxCopyStorageOperation* copy_operation() noexcept;
    const NativeGuiFrameBoxCopyStorageOperation* copy_operation() const noexcept;
private:
    friend void* allocate_native_gui_frame_box_00aa1a40(const void*, NativeGuiFrameBoxFactoryOperation&);
    void begin_allocation() noexcept;
    void validate_slot();
    void prepare_copy();
    bool can_return_failed_slot() const noexcept;
    void return_failed_slot() noexcept;
    NativeGuiWidgetPool<GuiFrameBoxPoolTraits>& pool_;
    const void* const source_;
    NativeGuiFrameBoxDefaultBindings* const default_;
    NativeGuiFrameBoxCopyStorageServices* const services_;
    NativeGuiFrameBoxCopyStorageFrames* const frames_;
    NativeGuiFrameBoxFactoryCopyStorage* const backing_;
    NativeGuiFrameBoxRecordReleaseOperation* const rollback_;
    const std::size_t rollback_capacity_;
    NativeGuiFrameBoxRecordReleaseOperation* const destruction_;
    const std::size_t destruction_capacity_;
    void* slot_{};
    void* result_{};
    NativeGuiFrameBoxFactoryPhase phase_{NativeGuiFrameBoxFactoryPhase::fresh};
    bool slot_checked_{}, copy_constructed_{}, constructor_entered_{};
    std::int32_t slot_action_{-1};
    bool pool_return_entered_{}, pool_return_returned_{};
    std::uint8_t padding_[2]{};
    alignas(NativeGuiFrameBoxCopyStorageOperation)
        std::byte copy_storage_[sizeof(NativeGuiFrameBoxCopyStorageOperation)];
};

void* allocate_native_gui_frame_box_00aa1a40(
    const void* optional_source, NativeGuiFrameBoxFactoryOperation&);
} // namespace bsp
