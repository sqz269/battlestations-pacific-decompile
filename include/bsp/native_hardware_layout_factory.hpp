#pragma once

#include "bsp/native_hardware_layout_construct.hpp"
#include <exception>

namespace bsp {

// Complete B2F710..B2F7F7. Native renderer ECX is unused; stack holds the
// actual stream-key/list address; EAX returns the selected owner; RET4.
// Borrow the constructor context's actual canonical tree, pool, owner/string
// domains and invalid-parameter service. Tree keys are raw stream identities.
//
// A hit validates the checked iterator, increments the actual owner's
// intrusive count, and returns it. A miss allocates a raw canonical slot and
// constructs it when nonnull. Only incomplete construction owns a raw-slot
// cleanup action. Disarm before pair construction/copy/insertion; a later
// failure does not destroy the completed owner or return its slot. Null raw
// allocation still inserts a null value. Ignore insertion's duplicate/result
// fields and return the newly constructed pointer without a cache AddRef.
void* get_or_create_native_hardware_layout_00b2f710(
    const void* actual_stream_key, NativeHardwareLayoutConstructContext&,
    void** acquired_before_insertion = nullptr);

// Optional host bookkeeping receives the acquired hit, or the completed new
// creator before pair construction/tree insertion. It starts empty. No native
// retain, cleanup or rollback is added; an insertion failure keeps that owner.

// New MSVC Win32 C++ interface; original calling convention is not exposed.
// The constructor's reached-input domain and existing tree/pool/invalid-handler
// contracts apply. The actual tree and pool identities remain fixed throughout
// this call. Every reached key/node/owner field must have valid backing storage;
// no key count clamp, null-hit repair, synthetic provider, ABI or game proof.

// Actual Source factory events, never Native Origin or a cleanup capability.
// Fresh retained record, distinct from key/output/native/provider write extents.
// Only the genuine observed entry/shared body writes these observations. A hit
// is not a new constructor receipt; null allocation is not a constructed owner.
class NativeHardwareLayoutFactoryObservation final {
public:
    NativeHardwareLayoutFactoryObservation() = default;
    NativeHardwareLayoutFactoryObservation(const NativeHardwareLayoutFactoryObservation&) = delete;
    NativeHardwareLayoutFactoryObservation& operator=(const NativeHardwareLayoutFactoryObservation&) = delete;
    NativeHardwareLayoutFactoryObservation(NativeHardwareLayoutFactoryObservation&&) = delete;
    NativeHardwareLayoutFactoryObservation& operator=(NativeHardwareLayoutFactoryObservation&&) = delete;
    bool source_complete() const { return returned_ && !failed_ && !invalidated_; }
    bool source_failed() const { return failed_ || invalidated_; }
    bool source_cache_hit() const { return hit_; }
    bool source_constructor_returned() const { return constructor_returned_; }
    bool source_pre_insertion() const { return pre_insertion_; }
    const void* actual_key_home() const { return key_; }
    const NativeHardwareLayoutConstructContext* actual_context() const { return context_; }
    void* allocated_slot() const { return raw_slot_; }
    void* source_result() const { return result_; }
    void* actual_tree() const { return tree_; }
    void* actual_pool() const { return pool_; }
private:
    friend void* get_or_create_native_hardware_layout_00b2f710(
        const void*, NativeHardwareLayoutConstructContext&, void**);
    friend void* get_or_create_native_hardware_layout_00b2f710(
        const void*, NativeHardwareLayoutConstructContext&, void**,
        NativeHardwareLayoutFactoryObservation&);
    static void* invoke(const void*, NativeHardwareLayoutConstructContext&,
        void**, NativeHardwareLayoutFactoryObservation*);
    bool entered_{}, returned_{}, failed_{}, invalidated_{};
    bool hit_{}, constructor_returned_{}, pre_insertion_{};
    const void* key_{};
    NativeHardwareLayoutConstructContext* context_{};
    void* tree_{};
    void* pool_{};
    void* raw_slot_{};
    void* result_{};
    std::exception_ptr failure_;
};
// Same actual raw mutable key and output home; no typed-key copy. Arm before
// Source admission, retain genuine completed creator on later insertion failure.
// Private observations authenticate this actual Source invocation/birth only;
// retain the record/context/actual creator through failure, no reset or cleanup.
void* get_or_create_native_hardware_layout_00b2f710(const void* actual_key_home,
    NativeHardwareLayoutConstructContext&, void** actual_acquired_before_insertion,
    NativeHardwareLayoutFactoryObservation&);

} // namespace bsp
