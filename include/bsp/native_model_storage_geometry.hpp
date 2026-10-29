#pragma once
#include "bsp/native_model_storage_lifetime.hpp"

namespace bsp {
struct NativeModelStorageGeometryContext {
    NativeModelStorageContext& storage;
    const volatile std::uint32_t& unchanged_00d7a260;
};
enum class NativeModelStorageGeometryPhase : std::uint8_t { fresh, entered, complete, failed };
struct NativeModelStorageGeometryScalars {
    std::uint32_t first_argument_bits{}, sentinel_bits{}, second_argument_bits{};
    std::uint8_t first_read{}, sentinel_read{}, first_stored{}, second_read{}, second_stored{};
};
class NativeModelStorageGeometryFrame;
struct NativeModelStorageGeometryAcquired {
    NativeModelStorageGeometryPhase phase{NativeModelStorageGeometryPhase::fresh};
    std::uint32_t active_call_site{};
    void* actual_model{};
    NativeModelStorageGeometryContext* context{};
    const NativeModelStorageGeometryFrame* frame{};
    void* captured_requested{};
    void* captured_previous{};
    bool requested_captured{}, previous_captured{}, same_geometry{};
    bool geometry_published{}, increment_completed{}, decrement_completed{}, terminal_completed{};
    NativeTextureSurfaceReferenceIncrement captured_increment{}, captured_decrement{};
    NativeModelStorageGeometryScalars scalars;
};

// Immutable Source views of FOUR initialized caller DWORDs. Original entry
// ESP+4 is unused, +8 is geometry, +C/+10 are the two scalar arguments.
// Taking these addresses reads none of their values. Backing stays live and
// mutable through callbacks; no by-value geometry/scalar snapshot is staged.
// Explicit operand aliases, including accessible Model bytes, are allowed.
// Frame/context/diagnostics themselves must not alias operands or native data.
class NativeModelStorageGeometryFrame final {
public:
    NativeModelStorageGeometryFrame(const volatile std::uint32_t& unused_argument,
        const volatile std::uint32_t& geometry_argument,
        const volatile std::uint32_t& scalar_178_argument,
        const volatile std::uint32_t& scalar_17c_argument) noexcept;
    ~NativeModelStorageGeometryFrame() noexcept;
    NativeModelStorageGeometryFrame(const NativeModelStorageGeometryFrame&) = delete;
    NativeModelStorageGeometryFrame& operator=(const NativeModelStorageGeometryFrame&) = delete;
    NativeModelStorageGeometryPhase phase() const noexcept;
    bool requires_process_retention() const noexcept;
private:
    const volatile std::uint32_t* const unused_argument_;
    const volatile std::uint32_t* const geometry_argument_;
    const volatile std::uint32_t* const scalar_178_argument_;
    const volatile std::uint32_t* const scalar_17c_argument_;
    NativeModelStorageGeometryPhase phase_{NativeModelStorageGeometryPhase::fresh};
    friend void set_native_model_geometry_storage_00b75170(void*, NativeModelStorageGeometryFrame&,
        NativeModelStorageGeometryContext&, NativeModelStorageGeometryAcquired&);
};

// Whole B75170[123B]: native ECX Model; unused/geometry/float178/float17C stack
// DWORDs, RET10h, no result. The Source API has explicit retained operand views,
// context and diagnostics; it is NOT that Original binary/private-stack ABI.
// Capture requested geometry FIRST, then old180. Identity skips only reference
// operations. Otherwise publish requested, use CURRENT CE221C for it, then a
// separately CURRENT CE2220 for captured old; current0 only at observed zero.
// No final180 clear/reload: callback replacement survives; no rollback/EH body.
// After calls, MOVSS first argument, MOVSS CURRENT D7A260 ONCE, compare/store178;
// then MOVSS second argument and compare/store17C against the SAME sentinel.
// Original UCOMISS/LAHF/TEST AH,44/JNP and conditional MOVSS are retained, so
// equality skips scalar stores and unordered comparisons store the input bits.
void set_native_model_geometry_storage_00b75170(void* actual_model,
    NativeModelStorageGeometryFrame&, NativeModelStorageGeometryContext&,
    NativeModelStorageGeometryAcquired&);

// Qualified MSVC Win32/SSE Source domain. Caller supplies a completed bound
// 481 Raw Model from THIS storage context, keeps its native credit/storage live
// through every remaining read/store, and forbids Model destruction/reuse while
// this entry is active. Its public canonical count/pool/profile admission is
// checked; this does not inspect the reference's private phase or authorize
// entry during a direct positive-count destruction. Member+4/current0 providers
// are the genuine existing actual-owner contract, never node/scene stand-ins.
// Inputs, context, frame and acquisition remain address-stable and disjoint
// except stated operand aliases. Calls may nest with distinct retained frames.
// A frame is one-shot. After entered failure retain this record, operands and
// affected graph; prohibit replay/further graph entry and use the host's process
// retention/_Exit policy. Destruction of an interrupted frame terminates rather
// than fabricating cleanup. Unmasked FP traps, SEH/nonlocal escape recovery,
// Native register-return ABI, Text transport and GUI/game completion are excluded.
} // namespace bsp
