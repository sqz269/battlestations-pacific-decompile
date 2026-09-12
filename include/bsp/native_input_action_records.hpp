#pragma once
#include "bsp/native_input_binding_storage.hpp"
#include "bsp/native_input_action_owner.hpp"

namespace bsp {
struct NativeInputActionRecordCalls {
    virtual ~NativeInputActionRecordCalls() = default;
    // A93A01: captured actual listener, current slot-zero profile, no stack
    // arguments. The provider must dispatch that profile to its real body.
    // There is NO scalar-delete flags argument at this call site.
    virtual void call_listener_slot0(void* actual_listener, std::uint32_t current_profile) = 0;
};
struct NativeInputActionRecordsContext {
    NativeInputBindingStorageContext bindings;
    NativeInputActionRecordCalls& listeners;
};

// Actual30h record. +4 is a DWORD-vector header, +10 a34h-binding-vector header,
// +2C a retained actual listener pointer. Never substitute projected owning
// vectors or a projected listener whose +4 is not its canonical reference count.
// Constructor: ECX, EAX original pointer, RET; untouched padding is preserved.
void* construct_native_input_action_record_00a93940(void*) noexcept;
// Destructor: ECX, RET; release captured listener first, then binding vector,
// then DWORD vector. Listener callback has no flags; clear slot only on return.
void destroy_native_input_action_record_00a939c0(void*, NativeInputActionRecordsContext&);
// Scalar wrapper: ECX, flags stack, EAX captured address, RET4; lowbit free.
void* delete_native_input_action_record_00a93a60(void*, std::uint32_t,
    NativeInputActionRecordsContext&);
// Copy: ECX destination, stack source, EAX destination, RET4; actual x87 copies,
// independent nested arrays and retained listener. No extra incoming reference.
void* copy_native_input_action_record_00a93a80(void*, const void*, NativeInputActionRecordsContext&);
// Header ECX, signed stack capacity/count, RET4. Resize decrements count before
// each reverse destruction; reserve deep-copies then destroys old rows forward.
void reserve_native_input_actions_00a93b30(void*, std::int32_t, NativeInputActionRecordsContext&);
void resize_native_input_actions_00a93c10(void*, std::int32_t, NativeInputActionRecordsContext&);

// Concrete owner-to-storage calls. The context and actual listener provider
// remain borrowed through owner destruction; no private arrays or lifetime.
class NativeInputActionStorageCalls final : public NativeInputActionOwnerCalls {
public:
    explicit NativeInputActionStorageCalls(NativeInputActionRecordsContext& context) noexcept
        : context_(context) {}
    void call_0086a430(void* header, std::int32_t count) override;
    void call_00a93c10(void* header, std::int32_t count) override;
private:
    NativeInputActionRecordsContext& context_;
};

// Complete normal schedules and supported C++ unwinds over valid native ranges.
// FH3/SEH, hardware-fault behavior, original ABI and arbitrary listener profiles
// are not supplied. Failed placement construction has the native00401130 no-op
// cleanup: reserve does not invent rollback/freeing of unpublished replacement.
} // namespace bsp
