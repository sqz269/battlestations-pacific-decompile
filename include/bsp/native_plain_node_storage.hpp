#pragma once
#include "bsp/native_node_base_destruction.hpp"
#include "bsp/native_node_construction.hpp"
#include <cstdint>
#include <exception>

namespace bsp {
class NativePlainNodeStorageContext final {
public:
    NativePlainNodeStorageContext(NativeNodeBaseDestructionContext&,
        void* actual_initialized_pool_0108ff58, const NativeNodeRawConstants&);
    NativePlainNodeStorageContext(const NativePlainNodeStorageContext&) = delete;
    NativePlainNodeStorageContext& operator=(const NativePlainNodeStorageContext&) = delete;
    NativeNodeBaseDestructionContext& node() const noexcept { return node_; }
    void* pool_0108ff58() const noexcept { return pool_; }
    const NativeNodeRawConstants& constants() const noexcept { return constants_; }
private:
    NativeNodeBaseDestructionContext& node_;
    void* const pool_;
    const NativeNodeRawConstants& constants_;
};
struct NativePlainNodeStorageAccess;
class NativePlainNodeStorageFrame final {
public:
    enum class Phase { fresh, creating, live, destroying, payload_dead, returned, null_result, failed };
    enum class Creator { none, owed, retired_by_direct_scalar, retained_failure };
    struct Progress {
        Phase phase;
        Creator creator;
        void* actual; // historical identity only after return; never late payload authority
        bool poisoned, node_type_live;
        bool allocation_entered, allocation_returned, name_sampled;
        bool constructor_entered, constructor_returned;
        bool factory_return_entered, factory_return_returned;
        bool destructor_entered, destructor_returned, lower_started, lower_complete;
        bool flags_read, type_ended, pool_return_entered, pool_returned, scalar_returned;
        std::uint8_t flags_low_byte;
    };
    NativePlainNodeStorageFrame() noexcept = default;
    ~NativePlainNodeStorageFrame() = default; // metadata only; no Native/type/pool cleanup
    NativePlainNodeStorageFrame(const NativePlainNodeStorageFrame&) = delete;
    NativePlainNodeStorageFrame& operator=(const NativePlainNodeStorageFrame&) = delete;
    NativePlainNodeStorageFrame(NativePlainNodeStorageFrame&&) = delete;
    NativePlainNodeStorageFrame& operator=(NativePlainNodeStorageFrame&&) = delete;
    Progress progress() const noexcept;
private:
    friend struct NativePlainNodeStorageAccess;
    const NativePlainNodeStorageContext* context_{};
    Phase phase_{Phase::fresh};
    Creator creator_{Creator::none};
    void* actual_{};
    NativeNodeStorage* node_{};
    NativeNodeBaseDestructionFrame destruction_frame_{};
    NativeNodeBaseDestructionAcquired destruction_{};
    std::exception_ptr primary_, secondary_;
    bool poisoned_{}, node_live_{}, factory_edge_{};
    bool allocation_entered_{}, allocation_returned_{}, name_sampled_{};
    bool constructor_entered_{}, constructor_returned_{};
    bool factory_return_entered_{}, factory_return_returned_{};
    bool destructor_entered_{}, destructor_returned_{}, flags_read_{}, type_ended_{};
    bool pool_return_entered_{}, pool_returned_{}, scalar_returned_{};
    std::uint8_t flags_low_byte_{};
};

// Whole normal B866C0, composed through the existing qualified raw B6F5A0.
// HOME is an ADDRESS view of its actual4B representation, not a pointer value.
// It can borrow a real Word or matching typed-pointer object/byte backing;
// no incompatible reference cast or early converted temporary is required.
// Only nonnull actual allocation reads that HOME once, then borrows its raw8
// header synchronously. NULL reads neither the HOME VALUE nor any header.
NativeNodeStorage* create_native_plain_node_storage_00b866c0(
    const volatile void* actual_name_pointer_home4,
    const NativePlainNodeStorageContext&, NativePlainNodeStorageFrame&);
// B6F8D0: actual raw B6F440 first, then ONE CURRENT LOW BYTE at this ADDRESS.
// HOME may be a real flags object or a still-live payload subobject; it must
// not overlap private metadata/active Source arguments/providers. No Word&
// reinterpretation is used. End actual trivial Node only after that borrow;
// trailing index174 remains alive for the matched178h pool return.
void* delete_native_plain_node_storage_00b6f8d0(
    const volatile void* actual_flags_byte_home,
    const NativePlainNodeStorageContext&, NativePlainNodeStorageFrame&);
// Source-only disposition after a successful direct flags0 scalar call.
// No destructor replay, synthesized flags1, retain, refund or count repair.
void return_native_plain_node_storage_slot(
    const NativePlainNodeStorageContext&, NativePlainNodeStorageFrame&);

// Caller retains genuine initialized38h pool/list/slabs, SAME raw Node owners/
// world/imports/strings/constants and all Name/buffer/frame/partial debt.
// Contexts/frame/active arguments/providers are disjoint from Native writes.
// Inherit raw B6F5A0 transient C++ birth/oldDD40/memmove/copy0-omission and raw
// base helper pointer-cell qualifications; no all-feature Native parity.
// No canonical/18/54/live-borrow issuer, NoThrow subset, Owner cast, Origin,
// Original private ABI/FH3/secondary-counter/fault/runtime claim is supplied.
} // namespace bsp
